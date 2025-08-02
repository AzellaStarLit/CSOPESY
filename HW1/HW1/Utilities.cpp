#include "Utilities.h"
#include "ProcessManager.h"
#include "FCFS.h"
#include "ConfigManager.h"
#include "BaseScheduler.h"
#include "sharedState.h"
#include "MemoryManager.h"

#include <iostream>
#include <cstdlib>
#include <vector>
#include <iomanip>
#include <fstream>
#include <random>
#include <memory>
#include <string>

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

		processManager.setMemoryManager(memoryManager.get());


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
	size_t totalUsedMemory = memoryManager->getUsedFrames() * configManager.getMemPerFrame();

	const size_t columnWidthName = 20;
	const size_t columnWidthMem = 15;
	const size_t columnWidthStat = 15;

	const size_t W_NAME = 20;
	const size_t W_MEM = 12;
	const size_t W_PGIN = 10;
	const size_t W_PGOUT = 10;
	const size_t W_STAT = 12;

	std::cout << "\033[32m" << std::left
		<< std::setw(W_NAME) << "Process"
		<< std::setw(W_MEM) << "Mem(K)"
		<< std::setw(W_PGIN) << "PgIn"
		<< std::setw(W_PGOUT) << "PgOut"
		<< std::setw(W_STAT) << "Status"
		<< "\033[0m\n"
		<< std::string(W_NAME + W_MEM + W_PGIN + W_PGOUT + W_STAT, '-') << "\n";

	for (auto* p : allProcesses)
	{
		std::cout << std::left
			<< std::setw(W_NAME) << p->getName()
			<< std::setw(W_MEM) << p->getMemoryUsage()
			<< std::setw(W_PGIN) << p->getPageIns()
			<< std::setw(W_PGOUT) << p->getPageOuts();

		std::string state = p->isFinished() ? "Finished"
			: (p->isSleeping() ? "Sleeping" : "Running");
		std::cout << std::setw(W_STAT) << state << "\n";
	}

	std::cout << "\n\033[32m"
		<< "Total Processes: " << allProcesses.size() << "\n"
		<< "Total Used Memory: " << totalUsedMemory << "K / "
		<< configManager.getMaxOverallMem() << "K"
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

	std::cout << "\n\033[32mMemory (KB):\033[0m\n"
		<< "  Used: " << used << "\n"
		<< "  Free: " << (total - used) << "\n";

	/* page‑in/out counters */
	std::cout << "\n\033[32mPages:\033[0m\n"
		<< "  Page-ins : " << memoryManager->getTotalPageIns() << '\n'
		<< "  Page-outs: " << memoryManager->getTotalPageOuts() << "\n\n";

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

bool isPowerOfTwo(int n) {

	int min = configManager.getMinMemPerProc();
	int max = configManager.getMaxMemPerProc();

	if (n < min || n > max) return false;
	if (n <= 0) return false;

	return (n & (n - 1)) == 0;
}

//TODO: Instructions may need to take space in memory
std::vector<std::string> generate_instructions() {
	uint32_t minInstructions = configManager.getMinInstructions();
	uint32_t maxInstructions = configManager.getMaxInstructions();

	//TODO: EDIT Instructions for more variety
	static const std::string templates[] = {
			"DECLARE(var_x, 100)",
			"DECLARE(var_y, 50)",
			"ADD(var_x, var_x, var_y)",
			"SUBTRACT(var_x, var_y, var_x)",
			"SLEEP(300)",
			"SLEEP(2000)",
			//"FOR([PRINT(\"Looping inside process\")], 2)",
			//"FOR([ADD(var_x, var_x, 1)], 2)",
			//"FOR([SUBTRACT(var_x, var_y, 1)], 2)",
			"PRINT(\"Hello world from process\")",
			"PRINT(\"We love CSOPESY <3\")",
			"PRINT(\"Value from: \" +var_x)"
	};

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> instructionCountDist(minInstructions, maxInstructions);
	std::uniform_int_distribution<> templateIndexDist(0, sizeof(templates) / sizeof(templates[0]) - 1);

	// Generate random number of instructions
	int numInstructions = instructionCountDist(gen);

	std::vector<std::string> instructions;
	for (int i = 0; i < numInstructions; ++i) {
		std::string instr = templates[templateIndexDist(gen)];
		instructions.push_back(instr);
	}
	return instructions;
}

