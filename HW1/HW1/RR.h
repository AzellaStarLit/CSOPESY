#pragma once

#include "BaseScheduler.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

class RRScheduler : public Scheduler {
private:
	int timeQuantum; //time slice for all processes

	std::vector<std::thread> workers; //worker threads [1 thread = 1 core]
	
	//synchronization
	std::mutex queueMutex;
	std::condition_variable cv;

public:
	RRScheduler(int cores, int quantum) : Scheduler(cores), timeQuantum(quantum){}
	~RRScheduler() {
		stop();
		for (auto& t : workers) {
			if (t.joinable()) t.join();
		}
	}

	void start() override;
	void stop() override;
	Process* get_next_process() override;

	//1 process = 1 core [thread]
	void worker_loop(int coreId); 
};

