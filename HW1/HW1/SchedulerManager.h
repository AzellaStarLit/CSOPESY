#pragma once

#include <thread>
#include <atomic>
#include <mutex>

class ProcessManager;
class ConsoleManager;

class SchedulerManager {
private:
	uint32_t minInstructions;
	uint32_t maxInstructions;
	int generationIntervalMs;

	std::atomic<bool> schedulerRunning;
	std::thread schedulerThread;
	int processCounter;

public:
	SchedulerManager();
	SchedulerManager(unsigned int minInst, unsigned int maxInst, int interval);
	void start_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager);
	void stop_scheduler();
	void run_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager);
	std::string generate_rand_instruction(const std::string& processName);

	//Getters
	uint32_t getMinInstructions() const;
	uint32_t getMaxInstructions() const;
	int getGenerationIntervalMs() const;

	//Setters
	bool setMinInstructions(uint32_t minIns);
	bool setMaxInstructions(uint32_t maxIns);
	bool setGenerationIntervalMs(int intervalMs);
};
