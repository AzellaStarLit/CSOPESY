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

void RRScheduler::worker_loop(int coreId) {
	while (isRunning) {
		Process* process = nullptr;

		{
			std::unique_lock<std::mutex> lock(queueMutex);
			cv.wait(lock, [&]() { return !readyQueue.empty() || !isRunning; });

			if (!isRunning && readyQueue.empty()) return;

			process = readyQueue.front();
			readyQueue.pop();
		}

		if (process && !process->isFinished()) {
			process->setCurrentCore(coreId);

			for (int i = 0;  i < timeQuantum; ++i) {
				if (process->isFinished()) break;

				std::string instruction = process->getCurrentInstruction();
				process->execute_instruction(instruction, coreId);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			if (!process->isFinished()) {
				add_process(process); //back to queue
			}
			else {
				process->markFinished();
			}
		}
	}
}