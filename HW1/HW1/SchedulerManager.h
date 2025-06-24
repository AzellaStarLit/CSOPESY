#pragma once

#include <thread>
#include <atomic>
#include <mutex>

class ProcessManager;
class ConsoleManager;

class SchedulerManager {
private:
	unsigned int minInstructions;
	unsigned int maxInstructions;
	int generationIntervalMs;

	std::atomic<bool> schedulerRunning;
	std::thread schedulerThread;
	int processCounter;

public:
	SchedulerManager(unsigned int minInst, unsigned int maxInst, int interval);
	void start_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager);
	void stop_scheduler();
	void run_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager);
	std::string generate_rand_instruction();
};
