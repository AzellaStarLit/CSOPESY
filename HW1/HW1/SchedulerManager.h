#pragma once
#include "SchedulerManager.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "random"
#include <thread>
#include <atomic>

class SchedulerManager {
private:
	unsigned int minInstructions;
	unsigned int maxInstructions;
	std::atomic<bool> schedulerRunning = false;
	std::thread schedulerThread;

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
	void run_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager);
	void generate_instructions(ProcessManager& processManager, ConsoleManager& consoleManager, int numProcesses);
};
