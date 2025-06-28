#pragma once

#include <queue>
#include <vector>
#include <string>

#include "Process.h"

class Scheduler {
protected:
	int numCores;
	bool isRunning;
	std::queue<Process*> readyQueue;
	uint32_t delayPerExec = 0; //default value 

	std::vector<bool> coreActive;
	std::mutex statsMutex;

public: 

	//constructor/deconstructor
	Scheduler(int cores) : numCores(cores), isRunning(false), coreActive(cores, false) {}
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

	// Returns: {used cores, available cores, utilization percentage}
	std::tuple<int, int, double> getCPUUtilization() {
		std::lock_guard<std::mutex> lock(statsMutex);

		int used = 0;
		for (bool active : coreActive) {
			if (active) used++;
		}

		int available = numCores - used;
		double utilization = (numCores > 0) ? (static_cast<double>(used) / numCores) * 100.0 : 0.0;

		return { used, available, utilization };
	}

};