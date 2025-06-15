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

public:
	void addProcess(Process* process); //add to process queue
	void start();
	void stop();
	void cpuWorker(int coreId);
};