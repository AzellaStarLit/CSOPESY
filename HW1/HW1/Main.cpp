/*
CSOPESY GROUP 10
ALABAT, Jeanne Hazel
ASTRERO, Dwight Daryl
DE CASTRO, Phoenix Claire
LABARRETE, Lance Desmond
*/

/*
This is where the program loop will be running unless the user exits.
*/

#include <iostream>
#include <string>
#include <unordered_map>

#include <iomanip>
#include <sstream>
#include <fstream>
#include <random>
#include <ctime>

#include "Utilities.h"
#include "Console.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "FCFS.h"
#include "RR.h"
#include "SchedulerController.h"
#include "ConfigManager.h"
#include "marqueeConsole.h"
#include "sharedState.h"
#include "MemoryManager.h"


ProcessManager processManager;
ConsoleManager consoleManager;
ConfigManager configManager;
std::unordered_map<std::string, Process> processes; 

std::unique_ptr<Scheduler> scheduler; 
bool isInitialized = false;

//MemoryManager memoryManager;
std::unique_ptr<MemoryManager> memoryManager;

// Global functions for initialization
void print_header();
//void initialize();
void scheduler_start();
//void scheduler_stop();
void report_util();
void clear();
void exit_program();
void vmstat();
void process_smi();


//this is for the screen -s/-ls/-r/-c commands
bool screen_command(const std::string& command) {

	//get the command
	std::mutex processConsoleMutex;
	std::istringstream iss(command);
	std::string screenCmd, flag, name;
	iss >> screenCmd >> flag >> name;

	//screen -ls displays the running processes, the core they are running on, and the number of instructions they are currently executing
	if (flag == "-ls") {

		//get all processes
		auto allProcesses = processManager.getAllProcesses();
		std::vector<Process*> running, finished;
		
		//finished and running processes
		for (auto* p : allProcesses) {
			if (!p) continue;

			auto status = p->getStatus();
			if (status == ProcessStatus::Running)
				running.push_back(p);
			else if (status == ProcessStatus::Finished)
				finished.push_back(p);
		}

		auto [used, available, util] = scheduler->getCPUUtilization();
		std::cout << "CPU utilization: " << util << "%\n";
		std::cout << "Cores used: " << used << "\n";
		std::cout << "Cores available: " << available << "\n\n";


		std::cout << "\033[35mRUNNING PROCESSES:\033[0m\n";
		if (running.empty()) {
			std::cout << "No running processes. \n";
		} else {
			std::cout << std::left << std::setw(20) << "Name"
				<< std::left << std::setw(10) << "PID"
				<< std::setw(10) << "Core"
				<< std::setw(15) << "Progress"
				<< "Creation Time\n" << std::string(90, '-') << "\n";

			for (auto* p : running) {
				std::cout << std::left << std::setw(20) << p->getName()
					<< std::left << std::setw(10) << p->getPID()
					<< std::setw(10) << p->getCurrentCore()
					<< std::setw(15) << (std::to_string(p->getCurrentLine()) + " / " + std::to_string(p->getTotalLines()))
					<< p->getCreationTimestamp() << "\n";
			}
		}
		
		std::cout << "\033[35m\nFINISHED PROCESSES:\033[0m\n";
		if (finished.empty()) {
			std::cout << "No finished processes.\n";
		}
		else {
			std::cout << std::left << std::setw(20) << "Name"
				<< std::setw(25) << "Creation Time"
				<< "Completion Time\n" << std::string(90, '-') << "\n";

			for (auto p : finished) {
				std::cout << std::left << std::setw(20) << p->getName()
					<< std::setw(25) << p->getCreationTimestamp()
					<< p->getCompletionTimestamp() << "\n";
				}
			}

		std::cout << "\n";
		return false;
	}

	//error message for when user doesn't input the name
	if ((flag == "-s" || flag == "-r") && name.empty()) {
		std::cout << "\033[31mMissing process name. Usage: screen -s <name> or screen -r <name>\n\033[0m";
		return false;
	}

	
	//screen -s creates a screen for the process
	if (flag == "-s") {
		std::lock_guard<std::mutex> lock(processConsoleMutex);

		std::string memSizeStr;
		iss >> memSizeStr;

		// Validation for memory size input
		if (memSizeStr.empty()) {
			std::cout << "\033[31mMissing memory size. Usage: screen -s <process_name> <process_memory_size>\033[0m\n";
			return false;
		}

		size_t memorySize;
		try {
			memorySize = std::stoi(memSizeStr);
		}
		catch (...) {
			std::cout << "\033[31mInvalid memory size format.\033[0m\n";
			return false;
		}

		if (!isPowerOfTwo(memorySize)) {
			std::cout << "\033[31mInvalid memory allocation. Allowed: powers of 2 from 64 to 65536 bytes.\033[0m\n";
			return false;
		}

		if (processManager.exists(name)) {
			std::cout << "Process '" << name << "' already exists. Use 'screen -r " << name << "' to resume.\n";
			return false;
		}

		// Create instructions vector
		std::vector<std::string> instructions = generate_instructions();

		// Create the process and load instructions
		processManager.create_process(name, memorySize, configManager.getMemPerFrame(), memoryManager.get());
		;
		Process* p = processManager.get_process(name);

		if (p) {
			p->load_instructions(instructions);
			consoleManager.attach_screen(name, p);

			if (scheduler) {
				scheduler->add_process(p);
			}
			
			std::cout << "\033[32mGenerated " << instructions.size() << " instructions for process '" << name << "'.\033[0m\n";
			consoleManager.resume_screen(name);

			return true;
		}
	}
		

	//screen -r displays the screen for the process
	if (flag == "-r") {
		if (!processManager.exists(name) || !consoleManager.has_screen(name)){
			std::cout << "Process '" << name << "' does not exist. Use screen -s " << name << "' to create it.\n";
			return false;
		}

		//if the process is already finished
		if (processManager.get_process(name)->isFinished()) {
			std::cout << "Process '" << name << "' has already finished.\n";
			return false;
		}

		consoleManager.resume_screen(name);
		return true;

	}

	if (flag == "-c") {
		std::lock_guard<std::mutex> lock(processConsoleMutex);

		std::string memSizeStr;
		iss >> memSizeStr;

		if (memSizeStr.empty()) {
			std::cout << "\033[31mMissing memory size. Usage: screen -c <name> <memory_size> <instructions>\033[0m\n";
			return false;
		}

		size_t memorySize;
		try {
			memorySize = std::stoi(memSizeStr);
		}
		catch (...) {
			std::cout << "\033[31mInvalid memory size format.\033[0m\n";
			return false;
		}

		if (!isPowerOfTwo(memorySize)) {
			std::cout << "\033[31mInvalid memory allocation. Allowed: powers of 2 from 64 to 65536 bytes.\033[0m\n";
			return false;
		}

		if (processManager.exists(name)) {
			std::cout << "\033[31mProcess '" << name << "' already exists. Use 'screen -r " << name << "' to resume.\033[0m\n";
			return false;
		}

		// Grab remaining instructions from the input line
		std::string restOfLine;
		std::getline(iss, restOfLine);

		// Trim leading spaces
		restOfLine.erase(0, restOfLine.find_first_not_of(" \t"));

		if (restOfLine.empty()) {
			std::cout << "\033[31mMissing instructions. Usage: screen -c <name> <memory_size> <instructions>\033[0m\n";
			return false;
		}

		// Split by semicolon
		std::vector<std::string> instructions;
		std::stringstream ss(restOfLine);
		std::string instr;

		while (std::getline(ss, instr, ';')) {
			// Trim whitespace from each instruction
			instr.erase(0, instr.find_first_not_of(" \t"));
			instr.erase(instr.find_last_not_of(" \t") + 1);
			if (!instr.empty()) {
				instructions.push_back(instr);
			}
		}

		// Now create and register the process
		processManager.create_process(name, memorySize, configManager.getMemPerFrame(), memoryManager.get());
		Process* p = processManager.get_process(name);

		if (p) {
			p->load_instructions(instructions);
			consoleManager.attach_screen(name, p);

			if (scheduler) {
				scheduler->add_process(p);
			}

			std::cout << "\033[32mLoaded " << instructions.size() << " instructions for process '" << name << "'.\033[0m\n";
			consoleManager.resume_screen(name);
			return true;
		}

	}
	return false;
}

void run_marquee() {
	MarqueeConsole marquee("marquee");
	marquee.draw(); 
	system("cls"); 
}

void exit_program() {
	// Exit the screen here
	scheduler->stop(); 
	std::exit(0);
}

	int main() {

		//prints the ASCII header
		print_header();

		//this is a list of commands recognized by the OS emulator
		std::string command;
		std::unordered_map<std::string, void(*)()> commandMap = {
			// List of commands for the Main Menu
			{"initialize", initialize},
			{"scheduler-start", scheduler_start},
			{"scheduler-stop", scheduler_stop},
			{"report-util", report_util},
			{"clear", clear},
			{"marquee", run_marquee},
			{"process-smi", process_smi},
    		{"vmstat", vmstat},
			{"exit", exit_program}
		};

		while (true) {

			//get the user input
			std::cout << "> ";
			std::getline(std::cin, command);

			bool shouldClear = false;

			//when the process screen is found, clear the menu screen then display the process screen
			auto typedCommand = commandMap.find(command);

			if (command == "initialize") {
				initialize();

				//THESE WILL BE READ FROM COFIG FILE
				int cores = configManager.getNumCPU();
				std::string algo = configManager.getScheduler();
				int quantum = configManager.getQuantumCycles();

				size_t minMemPerProc = configManager.getMinMemPerProc();
				size_t maxMemPerProc = configManager.getMaxMemPerProc();
				size_t memPerFrame = configManager.getMemPerFrame();

				std::cout << "mem-per-frame: " << memPerFrame << "\n"; 

				//sanitize the algorithm string
				if (!algo.empty() && algo.front() == '"' && algo.back() == '"') {
					algo = algo.substr(1, algo.length() - 2);
				}

				//this checks for the algorithms to use
				if (algo == "fcfs") {
					scheduler = std::make_unique<FCFSScheduler>(cores, memPerFrame, memoryManager.get());
				}
				else if (algo == "rr") {
					//TODO: Edit to not be hardcoded
					scheduler = std::make_unique<RRScheduler>(cores, quantum, memPerFrame);
					dynamic_cast<RRScheduler*>(scheduler.get())->setMemoryManager(memoryManager.get());
					;

				}
				else {
					std::cerr << "Unknown algorithm\n";
					return 1;
				}

				//set the delays per execution for the scheduler
				scheduler->setDelayPerExec(configManager.getDelaysPerExec());

				//get the processes in process manager then load it to the ready queue
				auto all = processManager.getAllProcesses();
				for (auto* p : all) {
					scheduler->add_process(p);
				}

				//start the scheduler [whatever was chosen after initialization] 
				scheduler->start();
			}
			else if (command.rfind("screen ", 0) == 0) {
				if (!isInitialized) {
					std::cout << "\033[31mError: Please run 'initialize' first.\033[0m\n";
					continue;
				}
				shouldClear = screen_command(command);
			}
			else if (command == "marquee") {
				if (!isInitialized) {
					std::cout << "\033[31mError: Please run 'initialize' first.\033[0m\n";
					continue;
				}

				run_marquee();
				shouldClear = true; 
			}
			else if (typedCommand != commandMap.end()) {
				if (!isInitialized && command != "initialize" && command != "exit") {
					std::cout << "\033[31mError: Please run 'initialize' first.\033[0m\n";
					continue;
				}
				commandMap[command](); // Call the function associated with the command
			}
			else {
				std::cout << "\033[31mCommand not recognized. Please try again.\n\033[0m";
				std::cout << "\033[1;33mType \'exit\' to quit, \'clear\' to clear the screen\n\033[0m";
			}

			if (shouldClear) {
				clear();
				shouldClear = false;
			}
		}
		return 0;
	}

	// Manual compilation via terminal: g++ menu_screen.cpp Utilities.cpp Console.cpp ProcessManager.cpp Process.cpp ConsoleManager.cpp FCFS.cpp -o main.exe
