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

		if (process && !process->isFinished()) {
			process->setCurrentCore(coreId); //set the current core executing this process

			while (!process->isFinished()) {
				std::string instruction = process->getCurrentInstruction(); //get the corresponding instruction
				process->execute_instruction(instruction, coreId); //execute the instruction
				//std::this_thread::sleep_for(std::chrono::milliseconds(100)); //wait

				//delay per execution
				uint32_t delay = getDelayPerExec();
				if (delay > 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(delay));
				}

				//mark the process as finished when all of the instructions are finished executing
				if (process->getCurrentLine() >= process->getTotalLines()) {
					process->markFinished();
				}

			}
		}
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