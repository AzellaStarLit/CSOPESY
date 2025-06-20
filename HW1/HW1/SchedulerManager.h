#pragma once
#include "vector"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include <random>

class SchedulerManager {
private:
	unsigned int minInstructions;
	unsigned int maxInstructions;
	bool schedulerRunning = false;

public:
	SchedulerManager(unsigned int minInst, unsigned int maxInst)
		: minInstructions(minInst), maxInstructions(maxInst) {
	}
	unsigned int getMinInstructions() const {
		return minInstructions;
	}
	unsigned int getMaxInstructions() const {
		return maxInstructions;
	}
	void setMinInstructions(unsigned int minInst) {
		minInstructions = minInst;
	}
	void setMaxInstructions(unsigned int maxInst) {
		maxInstructions = maxInst;
	}

	void start_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager);
	void stop_scheduler();

};
