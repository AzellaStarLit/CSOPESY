
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

SchedulerManager::SchedulerManager(unsigned int minInst, unsigned int maxInst, int interval)
	: minInstructions(minInst), maxInstructions(maxInst), generationIntervalMs(interval) {
	schedulerRunning = false;
	processCounter = 1;
}

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
	std::uniform_int_distribution<> dis(minInstructions, maxInstructions);

	while (schedulerRunning) {
		std::ostringstream nameStream;
		nameStream << "process_" << std::setw(2) << std::setfill('0') << processCounter++;
		std::string processName = nameStream.str();

		{
			std::scoped_lock lock(processManager.getMutex(), consoleManager.getMutex());

			processManager.create_process(processName);
			Process* process = processManager.get_process(processName);

			if (process) {
				int numInstructions = dis(gen);
				std::vector<std::string> instructions;
				for (int i = 0; i < numInstructions; ++i) {
					instructions.push_back(generate_rand_instruction());
				}

				process->load_instructions(instructions);
				consoleManager.attach_screen(processName, process);

				std::cout << "\033[36mGenerated process: " << processName
					<< " with " << numInstructions << " instructions.\033[0m" << std::endl;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(generationIntervalMs));
	}
}


std::string SchedulerManager::generate_rand_instruction(){

	static const std::string instructions[] = {
		"PRINT(\"\")",
		"PRINT(\"Processing...\")",
		"PRINT(\"We love CSOPESY <3\")",
	};

	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<> dis(0, sizeof(instructions)/sizeof(instructions[0]) - 1);

	return instructions[dis(gen)];
}


/*
void SchedulerManager::run_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(minInstructions, maxInstructions);

	while (schedulerRunning) {
		// Generate name like p01, p02...
		std::ostringstream nameStream;
		nameStream << "process_" << std::setw(2) << std::setfill('0') << processCounter++;
		std::string processName = nameStream.str();

		// Create process
		processManager.create_process(processName);
		Process* process = processManager.get_process(processName);

		if (!process) {
			std::cout << "\033[31mFailed to create process: " << processName << "\033[0m" << std::endl;
		}
		else {
			// Attach to console
			consoleManager.attach_screen(processName, process);

			std::cout << "\033[36mGenerated process: " << processName << "\033[0m" << std::endl;
			processManager.generate_instructions(processName, consoleManager);
		}

		// Sleep between generations
		std::this_thread::sleep_for(std::chrono::milliseconds(generationIntervalMs));
	}
}
*/

/*
#include "SchedulerManager.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "Instructions.h"
#include "random"
#include <thread>
#include <atomic>


void SchedulerManager::start_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager) {
	if(schedulerRunning) {
		std::cout << "\033[33mScheduler is already running.\033[0m" << std::endl;
		return;
	}

	schedulerRunning = true;
	schedulerThread = std::thread(&SchedulerManager::run_scheduler, this, std::ref(processManager), std::ref(consoleManager));
	std::cout << "\033[32mScheduler started.\033[0m" << std::endl;
	
}

void SchedulerManager::stop_scheduler() {
	if(!schedulerRunning) {
		std::cout << "\033[33mScheduler is not running.\033[0m" << std::endl;
		return;
	}

	schedulerRunning = false;
	std::cout << "\033[32mScheduler stopped.\033[0m" << std::endl;
}

void SchedulerManager::run_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(minInstructions, maxInstructions);

	// Generate processes with random instruction counts

	while (schedulerRunning) {

		std::string processName = "Process_" + std::to_string(processManager.get_process_count() + 1);
		int instructionCount = dis(gen);

		processManager.create_process(processName);
		Process* process = processManager.get_process(processName);

		if (!process) {
			std::cout << "\033[31mFailed to create process: " << processName << "\033[0m" << std::endl;
			continue;
		}

		consoleManager.attach_screen(processName, process);
	}

	std::cout << "\033[34mGenerating processes...\033[0m" << std::endl;
}

void SchedulerManager::generate_instructions(ProcessManager& processManager, ConsoleManager& consoleManager, int numProcesses) {
	std::random_device rd;
	std::mt19937 gen(rd());

	const Instructions& instructionSet = instructionTemplates[0]; // only PRINT for now

	for (int i = 0; i < numProcesses; ++i) {
		std::string processName = "process_" + std::to_string(i + 1);
		Process process(processName);

		std::cout << "Generating instructions for " << processName << "...\n";

		std::uniform_int_distribution<> dis(minInstructions, maxInstructions);
		int instructionCount = dis(gen);

		processManager.create_process(processName);  
		consoleManager.attach_screen(processName, &process);
	}

	std::cout << "\033[32mGenerated " << numProcesses
		<< " processes with random PRINT instructions.\033[0m\n";
}

*/