#include "RR.h"

#include <iostream>
#include <thread>
#include <chrono>

//start the worker threads
void RRScheduler::start() {
	isRunning = true;

	for (int i = 0; i < numCores; i++) {
		workers.emplace_back(&RRScheduler::worker_loop, this, i);
	}

	std::cout << "RR scheduler started\n";
}

void RRScheduler::worker_loop(int coreId) {
	while (isRunning)
	{
		Process* p = nullptr;

		{
			std::unique_lock<std::mutex> lock(queueMutex);

			//ready queue
			if (!readyQueue.empty()) {
				p = readyQueue.front();
				readyQueue.pop();
			}
		}

		if (p) {

			p->setCurrentCore(coreId); 

			int instructionsToRun = std::min(timeQuantum / 100, p->getTotalLines() - p->getCurrentLine());

			for (int i = 0; i < instructionsToRun; i++) {

				if (p->isFinished()) break;

				std::string instr = p->getCurrentInstruction(); //get current instruction
				p->execute_instruction(instr, coreId); //execute current instruction

				std::this_thread::sleep_for(std::chrono::milliseconds(100));

			}

			//if the process is not yet finished after time quantum, put it back in ready queue
			if (!p->isFinished()) {
				std::unique_lock<std::mutex> lock(queueMutex);
				readyQueue.push(p);
			}
		}
		else {
			p->markFinished();
		}
	}
}

void RRScheduler::stop() {
	isRunning = false;
	cv.notify_all();
}

Process* RRScheduler::get_next_process() {
	std::unique_lock<std::mutex> lock(queueMutex);

	if (readyQueue.empty()) return nullptr;

	Process* p = readyQueue.front();
	readyQueue.pop();
	return p;
}