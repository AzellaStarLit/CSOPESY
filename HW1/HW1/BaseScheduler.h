#pragma once

#include <queue>
#include <vector>
#include <string>
#include <chrono>

#include "Process.h"

class Scheduler {
protected:
	int numCores;
	bool isRunning;
	std::queue<Process*> readyQueue;
	uint32_t delayPerExec = 0; //default value 

	std::vector<bool> coreActive;
	std::mutex statsMutex;

	std::vector<int> currentPidPerCore;
	std::vector<std::chrono::steady_clock::time_point> lastActive;

	size_t totalCpuTicks = 0;
	size_t activeCpuTicks = 0;
	size_t idleCpuTicks = 0;
	std::mutex tickMutex;

	void incrementTick(bool isActive) {
		std::lock_guard<std::mutex> lock(tickMutex);
		totalCpuTicks++;
		if (isActive) activeCpuTicks++;
		else idleCpuTicks++;
	}

public: 

	//constructor/deconstructor
	Scheduler(int cores)
		: numCores(cores),
		isRunning(false),
		coreActive(cores, false),
		currentPidPerCore(cores, -1),
		lastActive(cores, std::chrono::steady_clock::now()){}
	virtual ~Scheduler(){}

	virtual void start() = 0; //begin the scheduling loop
	virtual void stop() { isRunning = false; } //stop scheduling

	//add a process in the ready queue
	virtual void add_process(Process* p) {
		
		readyQueue.push(p);
	}

	//check if ready queue is not empty
	virtual bool has_ready_process() const {
		return !readyQueue.empty();
	}

	virtual Process* get_next_process() = 0; //this will be defined by each algortihm [fcfs or rr]

	void setDelayPerExec(uint32_t delay) {
		delayPerExec = delay;
	}

	uint32_t getDelayPerExec() const {
		return delayPerExec;
	}

	// Centralized markers to keep timestamps in sync
	void markCoreRunning(int coreId, int pid) {
		std::lock_guard<std::mutex> lock(statsMutex);
		currentPidPerCore[coreId] = pid;
		coreActive[coreId] = true;
		lastActive[coreId] = std::chrono::steady_clock::now();
	}
	void markCoreIdle(int coreId) {
		std::lock_guard<std::mutex> lock(statsMutex);
		currentPidPerCore[coreId] = -1;
		coreActive[coreId] = false;
		lastActive[coreId] = std::chrono::steady_clock::now();
	}


	// Returns: {used cores, available cores, utilization percentage}
	std::tuple<int, int, double> getCPUUtilization() {
		using namespace std::chrono;
		std::lock_guard<std::mutex> lock(statsMutex);

		const auto now = steady_clock::now();
		const auto grace = 5ms;

		int used = 0;
		for (int pid : currentPidPerCore) {
			if (pid != -1) used++;
		}

		int available = numCores - used;
		double utilization = (numCores > 0) ? (static_cast<double>(used) / numCores) * 100.0 : 0.0;

		return { used, available, utilization };
	}

	std::vector<int> getCurrentPidsPerCore() {
		std::lock_guard<std::mutex> lock(statsMutex);
		return currentPidPerCore; // copy
	}

	bool isProcessRunningNow(int pid) {
		std::lock_guard<std::mutex> lock(statsMutex);
		for (int p : currentPidPerCore) if (p == pid) return true;
		return false;
	}

	//getters for cpu ticks
	size_t getTotalCpuTicks() {
		std::lock_guard<std::mutex> lock(tickMutex);
		return totalCpuTicks;
	}
	size_t getActiveCpuTicks(){
		std::lock_guard<std::mutex> lock(tickMutex);
		return activeCpuTicks;
	}
	size_t getIdleCpuTicks() {
		std::lock_guard<std::mutex> lock(tickMutex);
		return idleCpuTicks;
	}

};