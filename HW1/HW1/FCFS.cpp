#include "FCFS.h"

void FCFS::addProcess(Process* process) {
	std::lock_guard<std::mutex> lock(mtx);

	jobQueue.push(process); //add process to queue
	cv.notify_one(); //notify that there is a new process in the queue
}

void FCFS::start() {
	for (int i = 0; i < 4; i++) {
		cpuThreads.emplace_back(&FCFS::cpuWorker, this, i);
	}
}

//stop the scheduler and join the threads
void FCFS::stop() {
	{
		std::lock_guard<std::mutex> lock(mtx);
		stopScheduler = true;
	}

	cv.notify_all(); 
	for (auto& thread : cpuThreads) {
		if (thread.joinable()) thread.join();
	}
}

void FCFS::cpuWorker(int coreId) {
	while (true)
	{
		Process* proc = nullptr;

		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [&] {
				return stopScheduler || !jobQueue.empty();
				});

			if (stopScheduler && jobQueue.empty()) return;

			//get the first in the queue [FCFS]
			proc = jobQueue.front();
			jobQueue.pop();
		}

		//execute the instructions for the process
		for (int i = 0; i < proc->getTotalLines(); ++i) {
			proc->execute_print("", coreId);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}