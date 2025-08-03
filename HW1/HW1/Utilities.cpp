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
#include <unordered_set>

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

		for (auto* p : allProcesses) {
			if (!p) continue;

			auto status = p->getStatus();
			if (status == ProcessStatus::Running)
				running.push_back(p);
			else if (status == ProcessStatus::Finished)
				finished.push_back(p);
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

	//which processes are actually running now
	std::unordered_set<int> runningPids;
	if (scheduler) {
		auto pidsPerCore = scheduler->getCurrentPidsPerCore(); // [-1, pid, ...]
		for (int pid : pidsPerCore) if (pid != -1) runningPids.insert(pid);
	}

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
		if (!p) continue;

		std::cout << std::left
			<< std::setw(W_NAME) << p->getName()
			<< std::setw(W_MEM) << p->getMemoryUsage()
			<< std::setw(W_PGIN) << p->getPageIns()
			<< std::setw(W_PGOUT) << p->getPageOuts()
			<< std::setw(W_STAT) << p->getStatusString() << "\n";
		/*
		std::string state = p->isFinished() ? "Finished"
			: (p->isSleeping() ? "Sleeping" : "Running");
		std::cout << std::setw(W_STAT) << state << "\n";
		*/
	}

	std::cout << "\n\033[32m"
		<< "Total Processes: " << allProcesses.size() << "\n"
		<< "Total Used Memory: " << totalUsedMemory << "K / "
		<< configManager.getMaxOverallMem() << "K"
		<< "\033[0m\n\n";
}

/*
void process_smi() {
	auto allProcesses = processManager.getAllProcesses();

	// Build set of PIDs that are actually on CPU right now.
	std::unordered_set<int> runningPids;
	if (scheduler) {
		auto pidsPerCore = scheduler->getCurrentPidsPerCore(); // [-1, pid, ...]
		for (int pid : pidsPerCore) {
			if (pid != -1) runningPids.insert(pid);
		}
	}

	// Memory usage (frames * frameSize) -> bytes, then KB
	size_t usedKb = 0;
	if (memoryManager) {
		size_t usedBytes = memoryManager->getUsedFrames() * memoryManager->getFrameSize();
		usedKb = usedBytes / 1024;
	}

	// Columns
	const size_t W_NAME = 20;
	const size_t W_MEM = 12;
	const size_t W_PGIN = 10;
	const size_t W_PGOUT = 10;
	const size_t W_STAT = 12;

	std::cout << "\033[32m" << std::left
		<< std::setw(W_NAME) << "Process"
		<< std::setw(W_MEM) << "Mem(KB)"
		<< std::setw(W_PGIN) << "PgIn"
		<< std::setw(W_PGOUT) << "PgOut"
		<< std::setw(W_STAT) << "Status"
		<< "\033[0m\n"
		<< std::string(W_NAME + W_MEM + W_PGIN + W_PGOUT + W_STAT, '-') << "\n";

	for (auto* p : allProcesses) {
		size_t memKB = p->getMemoryUsage();

		std::string status;
		if (p->isFinished()) {
			status = "Finished";
		}
		else if (runningPids.count(p->getPID())) {
			status = "Running";     // authoritative: actually on CPU
		}
		else {
			status = "Sleeping";    // not on CPU right now (includes not admitted)
		}

		std::cout << std::left
			<< std::setw(W_NAME) << p->getName()
			<< std::setw(W_MEM) << memKB
			<< std::setw(W_PGIN) << p->getPageIns()
			<< std::setw(W_PGOUT) << p->getPageOuts()
			<< std::setw(W_STAT) << status << "\n";
	}

	std::cout << "\n\033[32m"
		<< "Total Processes: " << allProcesses.size() << "\n"
		<< "Total Used Memory: " << usedKb << " / "
		<< configManager.getMaxOverallMem() << " KB"
		<< "\033[0m\n\n";
}
*/

/*
void process_smi() {
	auto all = processManager.getAllProcesses();

	// who is currently on cores
	std::unordered_set<int> runningPids;
	if (scheduler) {
		auto pidsPerCore = scheduler->getCurrentPidsPerCore();
		for (int pid : pidsPerCore) if (pid != -1) runningPids.insert(pid);
	}

	// physical memory used (in KB)
	size_t usedKB = memoryManager ? memoryManager->getUsedFrames() * memoryManager->getFrameSize() : 0;
	size_t totalKB = configManager.getMaxOverallMem();

	const size_t W_NAME = 20, W_MEM = 12, W_PGIN = 10, W_PGOUT = 10, W_STAT = 12;

	std::cout << "\033[32m" << std::left
		<< std::setw(W_NAME) << "Process"
		<< std::setw(W_MEM) << "Mem(KB)"
		<< std::setw(W_PGIN) << "PgIn"
		<< std::setw(W_PGOUT) << "PgOut"
		<< std::setw(W_STAT) << "Status"
		<< "\033[0m\n"
		<< std::string(W_NAME + W_MEM + W_PGIN + W_PGOUT + W_STAT, '-') << "\n";

	for (auto* p : all) {
		if (!p) continue;
		std::string state =
			p->isFinished() ? "Finished" :
			(runningPids.count(p->getPID()) > 0) ? "Running" :
			"Sleeping"; // collapse remainder

		std::cout << std::left
			<< std::setw(W_NAME) << p->getName()
			<< std::setw(W_MEM) << p->getMemoryUsage()  // KB in your setup
			<< std::setw(W_PGIN) << p->getPageIns()
			<< std::setw(W_PGOUT) << p->getPageOuts()
			<< std::setw(W_STAT) << state << "\n";
	}

	std::cout << "\n\033[32m"
		<< "Total Processes: " << all.size() << "\n"
		<< "Total Used Memory: " << usedKB << " / " << totalKB << " KB"
		<< "\033[0m\n\n";
}

*/
/*
void vmstat() {
	auto all = processManager.getAllProcesses();
	int running = 0, sleeping = 0, finished = 0;
	for (auto* p : all) {
		if (!p) continue;
		if (p->isFinished())      ++finished;
		else if (p->isSleeping()) ++sleeping;
		else                      ++sleeping;
	}

	size_t totalKB = configManager.getMaxOverallMem();  // config value is in KB
	size_t frameSizeKB = 0;
	size_t usedFrames = 0;
	size_t freeFrames = 0;
	size_t pgIn = 0;
	size_t pgOut = 0;
	size_t usedKB = 0;

	if (memoryManager) {
		frameSizeKB = memoryManager->getFrameSize();     // keep units consistent with config (KB)
		usedFrames = memoryManager->getUsedFrames();
		freeFrames = memoryManager->getFreeFrames();
		pgIn = memoryManager->getTotalPageIns();
		pgOut = memoryManager->getTotalPageOuts();
		usedKB = memoryManager->getUsedFrames() * memoryManager->getFrameSize();
	}

	if (usedKB > totalKB) usedKB = totalKB;             // clamp to avoid underflow
	size_t freeKB = (usedKB <= totalKB) ? (totalKB - usedKB) : 0;

	std::cout << "\n\033[32mMemory (KB):\033[0m\n"
		<< "  Used: " << usedKB << "\n"
		<< "  Free: " << freeKB << "\n";

	std::cout << "\n\033[32mFrames:\033[0m\n"
		<< "  Used: " << usedFrames << "\n"
		<< "  Free: " << freeFrames << "\n";

	std::cout << "\n\033[32mPages:\033[0m\n"
		<< "  Page-ins : " << pgIn << '\n'
		<< "  Page-outs: " << pgOut << "\n";

	std::cout << "\n\033[32mProcesses:\033[0m\n"
		<< "  Running : " << running << '\n'
		<< "  Sleeping: " << sleeping << '\n'
		<< "  Finished: " << finished << "\n\n";
}
*/

void vmstat() {
	auto all = processManager.getAllProcesses();

	// Memory (KB)
	size_t usedKB = memoryManager ? memoryManager->getUsedFrames() * memoryManager->getFrameSize() : 0;
	size_t totalKB = configManager.getMaxOverallMem();
	std::cout << "\n\033[32mMemory (KB):\033[0m\n"
		<< "  Used: " << usedKB << "\n"
		<< "  Free: " << (totalKB > usedKB ? (totalKB - usedKB) : 0) << "\n";

	// Frames (optional but handy)
	std::cout << "\n\033[32mFrames:\033[0m\n"
		<< "  Used: " << (memoryManager ? memoryManager->getUsedFrames() : 0) << "\n"
		<< "  Free: " << (memoryManager ? memoryManager->getFreeFrames() : 0) << "\n";

	// Page-ins/outs
	std::cout << "\n\033[32mPages:\033[0m\n"
		<< "  Page-ins : " << (memoryManager ? memoryManager->getTotalPageIns() : 0) << "\n"
		<< "  Page-outs: " << (memoryManager ? memoryManager->getTotalPageOuts() : 0) << "\n";

	// Get who is actually running right now
	std::unordered_set<int> runningSet;
	if (scheduler) {
		auto pidsPerCore = scheduler->getCurrentPidsPerCore(); // [-1, pid, ...]
		for (int pid : pidsPerCore) if (pid != -1) runningSet.insert(pid);
	}

	size_t running = 0, sleeping = 0, finished = 0;
	running = runningSet.size();

	for (auto* p : all) {
		if (!p) continue;
		if (p->isFinished()) ++finished;
		else if (runningSet.count(p->getPID())) { /* counted above */ }
		else ++sleeping; // collapse: not running and not finished => "Sleeping"
	}

	std::cout << "\n\033[32mProcesses:\033[0m\n"
		<< "  Running : " << running << "\n"
		<< "  Sleeping: " << sleeping << "\n"
		<< "  Finished: " << finished << "\n\n";

	std::cout << "\n\033[32mCPU Ticks:\033[0m\n"
		<< "  Idle   : " << (scheduler ? scheduler->getIdleCpuTicks() : 0) << "\n"
		<< "  Active : " << (scheduler ? scheduler->getActiveCpuTicks() : 0) << "\n"
		<< "  Total  : " << (scheduler ? scheduler->getTotalCpuTicks() : 0) << "\n";

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
		/*
		"ADD(var_x, var_x, var_y)",
		"SUBTRACT(var_x, var_y, var_x)",
		"SLEEP(300)",
		"SLEEP(2000)",
		"FOR([PRINT(\"Looping inside process\")], 2)",
		"FOR([ADD(var_x, var_x, 1)], 10)",
		"FOR([SUBTRACT(var_x, var_x, 1)], 10)",
		"PRINT(\"Hello world from process\")",
		"PRINT(\"We love CSOPESY <3\")",
		"PRINT(\"Value from: \" +var_x)",
		*/
		"WRITE(0x50, 200)",
		"READ(var_y, 0x50)",
		"DECLARE(var_big, 70000)",
		"READ(var_big, 0x100)",
		"WRITE(0xFFFFF, 123)",      // Out of range memory address
		"WRITE(0x10, -5)",          // Negative value invalid
		"WRITE(0x10, 70000)",       // Value too large
		"WRITE(hello, 100)",        // Invalid address format
		"READ(var_bad, 0xFFFFF)",   // Out of range address
		"READ(var_bad2, hello)",    // Invalid address format
		"READ(var_uninit, 0x2000)",  // Read uninitialized memory (should return 0)

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

