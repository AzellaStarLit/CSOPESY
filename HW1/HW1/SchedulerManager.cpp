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

		/*
		else {
			for (int i = 0; i < instructionCount; ++i) {
				const Instructions& instruction = Instructions::generate_instructions();
				std::string instructionString = 
			}
		}
		*/

		consoleManager.attach_screen(processName, process);
	}

	std::cout << "\033[34mGenerating processes...\033[0m" << std::endl;
}

void SchedulerManager::generate_instructions(ProcessManager& processManager, ConsoleManager& consoleManager, int numProcesses) {
	std::random_device rd;
	std::mt19937 gen(rd());

	const Instructions& instruction = instruction[0]; // only PRINT for now

	for (int i = 0; i < numProcesses; ++i) {
		std::string processName = "process_" + std::to_string(i + 1);
		Process process(processName);

		std::cout << "Generating instructions for " << processName << "...\n";

		std::uniform_int_distribution<> dis(minInstructions, maxInstructions);
		int instructionCount = dis(gen);

		processManager.create_process(processName, instructionCount);  
		consoleManager.attach_screen(processName, &process);
	}

	std::cout << "\033[32mGenerated " << numProcesses
		<< " processes with random PRINT instructions.\033[0m\n";
}


