#pragma once

#include "BaseScheduler.h"
#include "Process.h"
#include "Memorymanager.h"
#include "ConfigManager.h"

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>

class RRScheduler : public Scheduler {
private:
	int timeQuantum; //time slice for all processes

	std::vector<std::thread> workers; //worker threads [1 thread = 1 core]
	
	//synchronization
	std::mutex queueMutex;
	std::condition_variable cv;

	MemoryManager* memoryManager = nullptr;
	size_t memPerProc = 0;
	size_t memPerFrame = 0;

	int quantumCycleCounter = 0; // tracks the number of cycles
	std::mutex fileMutex;

public:
	//TODO: Edit so that memory can handle processes with different memory requirements
	RRScheduler(int cores, int quantum, size_t memProc, size_t memFrame) : Scheduler(cores), timeQuantum(quantum), memPerProc(memProc), memPerFrame(memFrame) {}
	~RRScheduler() {
		stop();
		for (auto& t : workers) {
			if (t.joinable()) t.join();
		}
	}

	void setMemoryManager(MemoryManager* memMgr) {
		this->memoryManager = memMgr;
	}

	void start() override;
	void stop() override;
	void add_process(Process* p) override;
	bool has_ready_process() const override;
	Process* get_next_process() override;

	//1 process = 1 core [thread]
	void worker_loop(int coreId); 
};

