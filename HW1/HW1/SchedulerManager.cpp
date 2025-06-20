#include "SchedulerManager.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "random"


void SchedulerManager::start_scheduler(ProcessManager& processManager, ConsoleManager& consoleManager) {
	if(schedulerRunning) {
		std::cout << "\033[33mScheduler is already running.\033[0m" << std::endl;
		return;
	}

	schedulerRunning = true;
	std::cout << "\033[32mScheduler started.\033[0m" << std::endl;
	
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
		else {
			for (int i = 0; i < instructionCount; ++i) {
				process->add_instruction("Instruction_" + std::to_string(i + 1));
			}
		}
		
		consoleManager.attach_screen(processName, process);
		//std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate time between ticks
	}

	std::cout << "\033[34mGenerating processes...\033[0m" << std::endl;
}

void SchedulerManager::stop_scheduler() {
	if(!schedulerRunning) {
		std::cout << "\033[33mScheduler is not running.\033[0m" << std::endl;
		return;
	}

	schedulerRunning = false;
	std::cout << "\033[32mScheduler stopped.\033[0m" << std::endl;
}