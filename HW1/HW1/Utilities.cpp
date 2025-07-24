#include "Utilities.h"
#include "ProcessManager.h"
#include "FCFS.h"
#include "ConfigManager.h"
#include "BaseScheduler.h"
#include "sharedState.h"

#include <iostream>
#include <cstdlib>
#include <vector>
#include <iomanip>
#include <fstream>
#include "MemoryManager.h"

extern ProcessManager processManager;
extern ConsoleManager consoleManager;
extern ConfigManager configManager;
extern bool isInitialized;

extern std::unique_ptr<MemoryManager> memoryManager;


/**************************************
	FUNCTIONS FOR MAIN CONSOLE
***************************************/

void print_header() {
	// Prints the OS Main Menu
	std::cout << " ::::::::   ::::::::   ::::::::  :::::::::  :::::::::: ::::::::  :::   :::\n";
	std::cout << ":+:    :+: :+:    :+: :+:    :+: :+:    :+: :+:       :+:    :+: :+:   :+:\n";
	std::cout << "+:+        +:+        +:+    +:+ +:+    +:+ +:+       +:+         +:+ +:+ \n";
	std::cout << "+#+        +#++:++#++ +#+    +:+ +#++:++#+  +#++:++#  +#++:++#++   +#++:  \n";
	std::cout << "+#+               +#+ +#+    +#+ +#+        +#+              +#+    +#+   \n";
	std::cout << "#+#    #+# #+#    #+# #+#    #+# #+#        #+#       #+#    #+#    #+#   \n";
	std::cout << " ########   ########   ########  ###        ########## ########     ###   \n";

	std::cout << "\033[32mHello, welcome to CSOPESY Command Line!\n\033[0m";
	std::cout << "\033[1;33mType \'exit\' to quit, \'clear\' to clear the screen\n\033[0m";
}

void clear() {
	// Clear the screen here
	std::system("cls");
	print_header();
}

void initialize() { // intializer logic
	if (isInitialized) {
		std::cout << "\033[33mSystem already initialized.\033[0m\n";
		return;
	}
	//configManager will read from config
	if (configManager.loadFromFile("config.txt")) {

			memoryManager = std::make_unique<MemoryManager>(
			configManager.getMaxOverallMem(),
			configManager.getMemPerFrame()
		);

		isInitialized = true;
		std::cout << "\033[32mInitialization complete.\033[0m\n";

		//DEBUG: Print configuration settings
		//std::cout << "\033[1;34mConfiguration Settings:\033[0m\n";
		//configManager.printConfig();

	}
	else {
		std::cout << "\033[31mInitialization failed.\033[0m\n";
	}
}

//report-util saves screen -ls in a log file
void report_util() { // report-util logic
	if (!isInitialized) {
		std::cout << "\033[31mError: Please run 'initialize' first.\033[0m\n";
		return;
	}

	std::ofstream outfile("csopesy-log.txt");
	if (!outfile.is_open()) {
		std::cerr << "\033[31mFailed to write to csopesy-log.txt\033[0m\n";
		return;
	}

	auto allProcesses = processManager.getAllProcesses();
	if (allProcesses.empty()) {
		outfile << "No processes available.\n";
	}
	else {
		std::vector<Process*> running, finished;

		for (auto proc : allProcesses) {
			if (proc->isFinished()) finished.push_back(proc);
			else running.push_back(proc);
		}

		auto [used, available, util] = scheduler->getCPUUtilization();
		outfile << "CPU utilization: " << util << "%\n";
		outfile << "Cores used: " << used << "\n";
		outfile << "Cores available: " << available << "\n";

		outfile << "RUNNING PROCESSES:\n";
		if (running.empty()) {
			outfile << "No running processes.\n";
		}
		else {
			outfile << std::left << std::setw(20) << "Name"
				<< std::setw(10) << "Core"
				<< std::setw(15) << "Progress"
				<< "Creation Time\n";
			outfile << std::string(60, '-') << "\n";
			for (auto proc : running) {
				outfile << std::left << std::setw(20) << proc->getName()
					<< std::setw(10) << std::to_string(proc->getCurrentCore())
					<< std::setw(15) << (std::to_string(proc->getCurrentLine()) + " / " + std::to_string(proc->getTotalLines()))
					<< proc->getCreationTimestamp() << "\n";
			}
		}

		outfile << "\nFINISHED PROCESSES:\n";
		if (finished.empty()) {
			outfile << "No finished processes.\n";
		}
		else {
			outfile << std::left << std::setw(20) << "Name"
				<< std::setw(25) << "Creation Time"
				<< "Completion Time\n";
			outfile << std::string(60, '-') << "\n";
			for (auto proc : finished) {
				outfile << std::left << std::setw(20) << proc->getName()
					<< std::setw(25) << proc->getCreationTimestamp()
					<< proc->getCompletionTimestamp() << "\n";
			}
		}
	}

	outfile.close();
	std::cout << "\033[32mReport saved to csopesy-log.txt\033[0m\n";
}

void process_smi() {
	auto allProcesses = processManager.getAllProcesses();
	size_t totalUsedMemory = 0;

	const size_t columnWidthName = 20;
	const size_t columnWidthMem = 15;
	const size_t columnWidthStat = 15;

	std::cout << "\033[32m"
		<< std::left
		<< std::setw(columnWidthName) << "Process"
		<< std::setw(columnWidthMem) << "Memory (KB)"
		<< std::setw(columnWidthStat) << "Status"
		<< "\033[0m\n"
		<< std::string(50, '-') << "\n";

	for (auto* process : allProcesses) {
		size_t memUsage = process->getMemoryUsage();
		totalUsedMemory += memUsage;

		std::cout << std::left
			<< std::setw(columnWidthName) << process->getName()
			<< std::setw(columnWidthMem) << memUsage;

		if (process->isFinished()) {
			std::cout << std::setw(columnWidthStat) << "\033[32mFinished\033[0m";
		}
		else {
			std::cout << std::setw(columnWidthStat) << "\033[33mRunning\033[0m";
		}
		std::cout << "\n";
	}

	std::cout << "\n\033[32m"
		<< "Total Processes: " << allProcesses.size() << "\n"
		<< "Total Used Memory: " << totalUsedMemory << " / "
		<< configManager.getMaxOverallMem() << " KB"
		<< "\033[0m\n\n";
}


void vmstat() {
	auto allProcesses = processManager.getAllProcesses();
	int running = 0, sleeping = 0, finished = 0;

	for (auto* p : allProcesses) {
		if (p->isFinished()) ++finished;
		else if (p->isSleeping()) ++sleeping;
		else ++running;
	}

	size_t used = processManager.getUsedMemory();
	size_t total = configManager.getMaxOverallMem();

	std::cout << "\n\033[32mProcesses:\033[0m\n"
		<< "  Running : " << running << "\n"
		<< "  Sleeping: " << sleeping << "\n"
		<< "  Finished: " << finished << "\n"
		<< "  Total   : " << (running + sleeping + finished) << "\n";

	std::cout << "\n\033[32mMemory (KB):\033[0m\n"
		<< "  Used: " << used << "\n"
		<< "  Free: " << (total - used) << "\n";

	std::cout << "\n\033[32mPages:\033\[0m\n"
		<< "  Page-ins : N/A\n"
		<< "  Page-outs: N/A\n\n";
}
/*****************************
	HELPER FUNCTIONS
******************************/

std::vector<int> getPowerOfTwoSizesInRange() {
	std::vector<int> result;

	int minVal = configManager.getMinMemPerProc();
	int maxVal = configManager.getMaxMemPerProc();

	for (int i = 0; i <= 31; ++i) {
		int power = 1 << i;
		if (power >= minVal && power <= maxVal)
			result.push_back(power);
	}
	return result;
}
