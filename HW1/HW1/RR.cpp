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
    while (isRunning)
    {
        Process* p = nullptr;

        /* -- pick a PCB --------------------------------------------------- */
        {
            std::unique_lock<std::mutex> lk(queueMutex);
            cv.wait(lk, [&] { return !readyQueue.empty() || !isRunning; });
            if (!isRunning) return;                 // shutting down

            p = readyQueue.front();
            readyQueue.pop();
        }

        if (!p || p->isFinished())
            continue;                               // nothing to do

        /* -- memory admission --------------------------------------------- */
        if (!memoryManager) {                       // still not configured?
            add_process(p);                         // give it back
            std::this_thread::yield();
            continue;
        }

        std::size_t bytesNeeded = p->getMemoryUsage();
        std::size_t framesNeeded = (bytesNeeded + memPerFrame - 1) / memPerFrame;

        const int pid = p->getPID();
        bool resident = memoryManager->getProcessStartFrame(pid) != std::size_t(-1);

        if (!resident &&                                  // not in RAM _and_
            !memoryManager->allocateFrames(framesNeeded, pid, {})) {
            add_process(p);                // push to the tail
            std::this_thread::yield();     // let another core run
            continue;                      // pick a different PCB
        }

        /* -- run one quantum ---------------------------------------------- */
        {
            std::lock_guard<std::mutex> g(statsMutex);
            coreActive[coreId] = true;
        }
        p->setCurrentCore(coreId);

        for (int i = 0; i < timeQuantum && !p->isFinished(); ++i) {
            p->execute_instruction(p->getCurrentInstruction(), coreId);
            if (auto d = getDelayPerExec(); d)
                std::this_thread::sleep_for(std::chrono::milliseconds(d));
        }

        /* -- bookkeeping --------------------------------------------------- */
        {
            std::lock_guard<std::mutex> g(fileMutex);
            if (++quantumCycleCounter % timeQuantum == 0)
                memoryManager->snapshotMemoryToFile(quantumCycleCounter);
        }

        if (p->isFinished()) {
            p->markFinished();
            if (auto sf = memoryManager->getProcessStartFrame(pid); sf != std::size_t(-1))
                memoryManager->deallocateFrames(framesNeeded, sf, {});
        }
        else {
            add_process(p);                           // round?robin: back to tail
        }

        {
            std::lock_guard<std::mutex> g(statsMutex);
            coreActive[coreId] = false;
        }
    }
}
