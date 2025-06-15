#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

#include "Process.h"

class FCFS {
private:
	std::queue<Process*> jobQueue;
	std::mutex mtx;
	std::condition_variable cv;
	bool stopScheduler = false;
	std::vector<std::thread> cpuThreads;

	std::mutex executionLock; //run one process at a time
	int nextCoreId = 0; //assign the cores in order

public:
	void addProcess(Process* process); //add to process queue
	void start();
	void stop();
	void cpuWorker(int coreId);
};