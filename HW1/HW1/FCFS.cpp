#include "FCFS.h"

#include <iostream>
#include <chrono>
#include <thread>

//constructor
FCFSScheduler::FCFSScheduler(int cores,
							size_t framesSz,
							MemoryManager* memMgr) 
	: Scheduler(cores),
	  memoryManager(memMgr),
	  memPerFrame(framesSz){}

//start the algorithm
void FCFSScheduler::start() {
	isRunning = true;

	for (int i = 0; i < numCores; i++) {
		workers.emplace_back(&FCFSScheduler::worker_loop, this, i);
	}
	std::cout << "FCFS scheduler started\n";
}

//1 core = 1 process to run
void FCFSScheduler::worker_loop(int coreId) {
	while (isRunning) {
		Process* process = nullptr;

		{
			std::unique_lock<std::mutex> lock(queueMutex);
			cv.wait(lock, [&]() {return !readyQueue.empty() || !isRunning; });

			if (!isRunning && readyQueue.empty()) return;

			process = readyQueue.front();
			readyQueue.pop();
		}

		if (!process || process->isFinished()) continue;

		size_t framesNeeded = std::max<size_t>(
			1, (process->getMemoryUsage() + memPerFrame - 1) / memPerFrame);

		/*
		if (!process->hasResidentPage() && memoryManager->getFreeFrames() == 0) {
			add_process(process);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}*/

		bool reservedNow = false;

		if (!process->hasResidentPage()) {
			std::unique_lock<std::mutex> lk(admissionMutex);
			size_t totalFrames = memoryManager ? memoryManager->getFreeFrames() : 0;
			size_t freeForStart = (totalFrames >= reservedFrames)
				? (totalFrames - reservedFrames)
				: 0;

			if (freeForStart < framesNeeded) {
				lk.unlock();
				add_process(process);

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

			reservedFrames += framesNeeded;
			reservedNow = true;
		}
		/*
		{
			std::lock_guard<std::mutex> statsLock(statsMutex);
			coreActive[coreId] = true; // mark this core as active
			currentPidPerCore[coreId] = process->getPID();
		}*/
		markCoreRunning(coreId, process->getPID());
		process->setCurrentCore(coreId); //set the current core executing this process

		while (!process->isFinished()) {
			process->execute_instruction(process->getCurrentInstruction(), coreId);
			if (uint32_t d = getDelayPerExec(); d)
				std::this_thread::sleep_for(std::chrono::milliseconds(d));
		}
		/*
		{
			std::lock_guard<std::mutex> statsLock(statsMutex);
			coreActive[coreId] = false;
			currentPidPerCore[coreId] = -1;
		}*/

		markCoreIdle(coreId);
		process->markFinished();

		if (memoryManager) memoryManager->freeAllFramesForPid(process->getPID());
		memoryManager->unregisterProcessSwap(process->getPID());
	}
}

void FCFSScheduler::add_process(Process* p) {
	std::lock_guard<std::mutex> lock(queueMutex);
	readyQueue.push(p);
	cv.notify_one();
}

Process* FCFSScheduler::get_next_process() {
	if (!readyQueue.empty()) {
		Process* p = readyQueue.front();
		readyQueue.pop();
		return p;
	}

	return nullptr;
}

void FCFSScheduler::stop() {
	isRunning = false;
	cv.notify_all();

	//join all worker threads
	for (auto& worker : workers) {
		if (worker.joinable()) {
			worker.join();
		}
	}

	//clear the worker list
	workers.clear();

	std::cout << "All worker threads joined.\n";
}