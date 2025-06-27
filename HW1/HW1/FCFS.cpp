#include "FCFS.h"

#include <iostream>
#include <chrono>
#include <thread>

//constructor
FCFSScheduler::FCFSScheduler(int cores) : Scheduler(cores){}

//start the algorithm
void FCFSScheduler::start() {
	isRunning = true;

	for (int i = 0; i < numCores; i++) {
		workers.emplace_back(&FCFSScheduler::worker_loop, this, i);
	}
	std::cout << "FSFC scheduler started\n";
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

		if (process) {
			process->setCurrentCore(coreId); //set the current core executing this process

			while (!process->isFinished()) {
				std::string instruction = process->getCurrentInstruction(); //get the corresponding instruction
				process->execute_instruction(instruction, coreId); //execute the instruction
				std::this_thread::sleep_for(std::chrono::milliseconds(100)); //wait

				//mark the process as finished when all of the instructions are finished executing
				if (process->getCurrentLine() >= process->getTotalLines()) {
					process->markFinished();
				}
			}
		}
	}
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