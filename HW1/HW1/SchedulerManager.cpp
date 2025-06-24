
#include "SchedulerManager.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "Instructions.h"

#include <random>
#include <thread>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <sstream>


// Constants for validation
constexpr uint32_t MIN_INS_LOWER_BOUND = 1;
constexpr uint32_t MIN_INS_UPPER_BOUND = std::numeric_limits<uint32_t>::max();

constexpr uint32_t MAX_INS_LOWER_BOUND = 1;
constexpr uint32_t MAX_INS_UPPER_BOUND = std::numeric_limits<uint32_t>::max();

constexpr int GENERATION_INTERVAL_MIN = 0;
constexpr int GENERATION_INTERVAL_MAX = std::numeric_limits<int>::max();


//CONSTRUCTORS
SchedulerManager::SchedulerManager()
	: minInstructions(1), maxInstructions(1), generationIntervalMs(1000) {
	schedulerRunning = false;
	processCounter = 1;
}

SchedulerManager::SchedulerManager(unsigned int minInst, unsigned int maxInst, int interval)
	: minInstructions(minInst), maxInstructions(maxInst), generationIntervalMs(interval) {
	schedulerRunning = false;
	processCounter = 1;
}


//FUNCTIONS
void SchedulerManager::start_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager) {
	if (schedulerRunning) {
		std::cout << "\033[33mScheduler is already running.\033[0m" << std::endl;
		return;
	}

	schedulerRunning = true;
	schedulerThread = std::thread(&SchedulerManager::run_scheduler, this, std::ref(processManager), std::ref(consoleManager));
	std::cout << "\033[32mScheduler started.\033[0m" << std::endl;
}

void SchedulerManager::stop_scheduler() {
	if (!schedulerRunning) {
		std::cout << "\033[33mScheduler is not running.\033[0m" << std::endl;
		return;
	}

	schedulerRunning = false;

	if (schedulerThread.joinable()) {
		schedulerThread.join();
	}

	std::cout << "\033[32mScheduler stopped.\033[0m" << std::endl;
}

void SchedulerManager::run_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager) {
	std::random_device rd;
	std::mt19937 gen(rd());

	//TODO: READ minInstructions and maxInstructions from config file
	const uint32_t minInstructions = 50;     
	const uint32_t maxInstructions = 10000; 

	std::uniform_int_distribution<> dis(minInstructions, maxInstructions);

	while (schedulerRunning) {
		std::ostringstream nameStream;
		nameStream << "process_" << processCounter++;
		std::string processName = nameStream.str();

		{
			std::scoped_lock lock(processManager.getMutex(), consoleManager.getMutex());

			processManager.create_process(processName);
			Process* process = processManager.get_process(processName);

			if (process) {
				int numInstructions = dis(gen);
				std::vector<std::string> instructions;
				for (int i = 0; i < numInstructions; ++i) {
					instructions.push_back(generate_rand_instruction(processName));
				}

				process->load_instructions(instructions);
				consoleManager.attach_screen(processName, process);

				/*
				std::cout << "\033[36mGenerated process: " << processName
					<< " with " << numInstructions << " instructions.\033[0m" << std::endl;
				*/
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(generationIntervalMs));
	}
}


std::string SchedulerManager::generate_rand_instruction(const std::string& processName){

	static const std::string instructions[] = {
		"PRINT(\"Hello world from " + processName + "!\")",
		//"PRINT(\"Processing...\")",
		//"PRINT(\"We love CSOPESY <3\")",
	};

	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<> dis(0, sizeof(instructions)/sizeof(instructions[0]) - 1);

	return instructions[dis(gen)];
}


//GETTERS
uint32_t SchedulerManager::getMinInstructions() const {
	return minInstructions;
}

uint32_t SchedulerManager::getMaxInstructions() const {
	return maxInstructions;
}

int SchedulerManager::getGenerationIntervalMs() const {
	return generationIntervalMs;
}

//SETTERS
bool SchedulerManager::setMinInstructions(uint32_t minIns) {
	if (minIns < MIN_INS_LOWER_BOUND || minIns > MIN_INS_UPPER_BOUND) {
		std::cerr << "Invalid minInstructions: " << minIns << ". Must be between "
			<< MIN_INS_LOWER_BOUND << " and " << MIN_INS_UPPER_BOUND << ".\n";
		return false;
	}
	minInstructions = minIns;

	if (maxInstructions < minInstructions) {
		maxInstructions = minInstructions;
	}
	return true;
}

bool SchedulerManager::setMaxInstructions(uint32_t maxIns) {
	if (maxIns < MAX_INS_LOWER_BOUND || maxIns > MAX_INS_UPPER_BOUND) {
		std::cerr << "Invalid maxInstructions: " << maxIns << ". Must be between "
			<< MAX_INS_LOWER_BOUND << " and " << MAX_INS_UPPER_BOUND << ".\n";
		return false;
	}
	maxInstructions = maxIns;

	if (minInstructions > maxInstructions) {
		minInstructions = maxInstructions;
	}
	return true;
}

bool SchedulerManager::setGenerationIntervalMs(int intervalMs) {
	if (intervalMs < GENERATION_INTERVAL_MIN || intervalMs > GENERATION_INTERVAL_MAX) {
		std::cerr << "Invalid generationIntervalMs: " << intervalMs << ". Must be between "
			<< GENERATION_INTERVAL_MIN << " and " << GENERATION_INTERVAL_MAX << ".\n";
		return false;
	}
	generationIntervalMs = intervalMs;
	return true;
}
