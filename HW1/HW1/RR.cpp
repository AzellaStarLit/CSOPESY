#include "RR.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <condition_variable>

//start the worker threads
void RRScheduler::start() {
	isRunning = true;

	for (int i = 0; i < numCores; i++) {
		workers.emplace_back(&RRScheduler::worker_loop, this, i);
	}

	std::cout << "RR scheduler started\n";
	std::cout << "Memory per Frame: " << memPerFrame << "\n";
}

void RRScheduler::stop() {
	isRunning = false;
	cv.notify_all();

	for (auto& worker : workers) {
		if (worker.joinable()) {
			worker.join();
		}
	}
	workers.clear();
}


void RRScheduler::add_process(Process* p) {
	std::lock_guard<std::mutex> lock(queueMutex);
    p->setStatus(ProcessStatus::Ready);
	readyQueue.push(p);
	cv.notify_one();
}

bool RRScheduler::has_ready_process() const {
	return !readyQueue.empty();
}

Process* RRScheduler::get_next_process() {
	std::lock_guard<std::mutex> lock(queueMutex);
	if (!readyQueue.empty()) {
		Process* p = readyQueue.front();
		readyQueue.pop();
		return p;
	}

	return nullptr;
} 


void RRScheduler::worker_loop(int coreId)
{
    while (isRunning) {
        Process* process = nullptr;

        /* --------- pick a PCB --------- */
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [&] { return !readyQueue.empty() || !isRunning; });
            if (!isRunning && readyQueue.empty()) return;

            process = readyQueue.front();
            readyQueue.pop();
        }

        if (!process || process->isFinished())   // double check
            continue;
        
        if (!process->hasResidentPage() && memoryManager->getFreeFrames() == 0) {
            process->setStatus(ProcessStatus::Waiting);
            process->setCurrentCore(-1);
            add_process(process);                   // push back to the queue
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;                               // try another PCB
        }



        /* --------- run one quantum --------- */
       /*
        {
            std::lock_guard<std::mutex> g(statsMutex);
            coreActive[coreId] = true;
            currentPidPerCore[coreId] = process->getPID();
        }*/

        markCoreRunning(coreId, process->getPID());
        process->setCurrentCore(coreId);
		process->setStatus(ProcessStatus::Running);

        for (int i = 0; i < timeQuantum && !process->isFinished(); ++i) {
            process->execute_instruction(process->getCurrentInstruction(), coreId);
            if (uint32_t d = getDelayPerExec(); d) std::this_thread::sleep_for(std::chrono::milliseconds(d));
        }

        /* --------- bookkeeping --------- */
        // THIS IS FOR MEMORY STAMP
        /*
        {
            std::lock_guard<std::mutex> g(fileMutex);
            if (++quantumCycleCounter % timeQuantum == 0)
                memoryManager->snapshotMemoryToFile(quantumCycleCounter);
        }
        */

        if (process->isFinished()) {
            process->markFinished();

            if (memoryManager) {
                memoryManager->freeAllFramesForPid(process->getPID());
                memoryManager->unregisterProcessSwap(process->getPID());
            }
            
            /* free its last resident frame(s) --------------------------- 
            size_t bytes = process->getMemoryUsage();
            size_t frames = std::max<size_t>(1, (bytes + memPerFrame - 1) / memPerFrame);
            size_t start = memoryManager->getProcessStartFrame(process->getPID());
            if (start != size_t(-1))
                memoryManager->deallocateFrames(frames, start, {});*/
        }
        else {
            process->setCurrentCore(-1);
            process->setStatus(ProcessStatus::Ready);
            add_process(process);       // round?robin back to tail
        }
        /*
        {
            std::lock_guard<std::mutex> g(statsMutex);
            coreActive[coreId] = false;
            currentPidPerCore[coreId] = -1;
        }*/

        markCoreIdle(coreId);
    }
}

