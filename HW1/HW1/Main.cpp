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
#include <unordered_map>std::unordered_map<std::string, Process> processes;
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <random>

#include "Utilities.h"
#include "Console.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "FCFS.h"
#include "RR.h"
#include "SchedulerController.h"
#include "ConfigManager.h"



ProcessManager processManager;
ConsoleManager consoleManager;
ConfigManager configManager;


std::unique_ptr<Scheduler> scheduler; 
bool isInitialized = false;

// Global functions for initialization
void print_header();
//void initialize();
void scheduler_start();
//void scheduler_stop();
void report_util();
void clear();
void exit_program();


//this is for the screen -s/-ls/-r commands
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
			(p->isFinished() ? finished : running).push_back(p);
		}

		std::cout << "RUNNING PROCESSES: \n";
		if (running.empty()) {
			std::cout << "No running processes. \n";
		} else {
			std::cout << std::left << std::setw(20) << "Name"
				<< std::setw(10) << "Core"
				<< std::setw(15) << "Progress"
				<< "Creation Time\n" << std::string(60, '-') << "\n";

			for (auto* p : running) {
				std::cout << std::left << std::setw(20) << p->getName()
					<< std::setw(10) << p->getCurrentCore()
					<< std::setw(15) << (std::to_string(p->getCurrentLine()) + " / " + std::to_string(p->getTotalLines()))
					<< p->getCreationTimestamp() << "\n";
			}
		}
		
		std::cout << "\nFINISHED PROCESSES: \n";
		if (finished.empty()) {
			std::cout << "No finished processes.\n";
		}
		else {
			std::cout << std::left << std::setw(20) << "Name"
				<< std::setw(25) << "Creation Time"
				<< "Completion Time\n" << std::string(60, '-') << "\n";

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

		if (processManager.exists(name)) {
			std::cout << "Process '" << name << "' already exists. Use 'screen -r " << name << "' to resume.\n";
			return false;
		}

		static const std::string templates[] = {
			"DECLARE(var_x, 0)",
			"DECLARE(var_y, 5)",
			"ADD(var_z, var_x, var_y)",
			"SUBTRACT(var_a, var_y, var_x)",
			"SLEEP(300)",
			"SLEEP(2000)",
			"FOR([PRINT(\"Looping inside process\")], 2)",
			"FOR([ADD(var_i, var_x, 1)], 2)",
			"FOR([SUBTRACT(var_j, var_y, 1)], 2)",
			"PRINT(\"Hello world from process\")",
			"PRINT(\"We love CSOPESY <3\")"
		};

		// Get min and max instructions from config
		uint32_t minInstructions = configManager.getMinInstructions();
		uint32_t maxInstructions = configManager.getMaxInstructions();

		if (minInstructions > maxInstructions) std::swap(minInstructions, maxInstructions);

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> instructionCountDist(minInstructions, maxInstructions);
		std::uniform_int_distribution<> templateIndexDist(0, sizeof(templates) / sizeof(templates[0]) - 1);

		// Generate random number of instructions
		int numInstructions = instructionCountDist(gen);

		// Create instructions vector
		std::vector<std::string> instructions;
		for (int i = 0; i < numInstructions; ++i) {
			std::string instr = templates[templateIndexDist(gen)];
			instructions.push_back(instr);
		}

		// Create the process and load instructions
		processManager.create_process(name);
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

		// Add process to scheduler!

		/*
		if (processManager.exists(name)) {
			std::cout << "Process '" << name << "' already exists. Use 'screen -r " << name << "' to resume.\n";
			return false;
		}

		consoleManager.create_screen_with_process(name); 
		return true;
		*/
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
			{"exit", exit_program}
		};

		while (true) {

			//TODO: CHECK IF THE OS IS INITIALIZED BEFORE RECOGNIZING ANY COMMAND
			
			//get the user input
			std::cout << "> ";
			std::getline(std::cin, command);

			bool shouldClear = false;

			//when the process screen is found, clear the menu screen then display the process screen
			auto typedCommand = commandMap.find(command);

			if (command == "initialize") {
				initialize();

				//TODO: THESE WILL BE READ FROM COFIG FILE
				int cores = configManager.getNumCPU();
				std::string algo = configManager.getScheduler();
				int quantum = configManager.getQuantumCycles();

				//this checks for the algorithms to use
				if (algo == "fcfs") {
					scheduler = std::make_unique<FCFSScheduler>(cores);
				}
				else if (algo == "rr") {
					scheduler = std::make_unique<RRScheduler>(cores, quantum);
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

				/*
				// TEST: Manually create 10 processes with 100 instructions
				for (int i = 1; i <= 10; ++i) {
					std::string name = "process_" + std::to_string(i);

					processManager.create_dummy(name, 100);            // register the process in manager

					Process* p = processManager.get_process(name);

					scheduler->add_process(p);                           // add to ready queue
					consoleManager.attach_screen(name, p);          // create screen for the process
				}
				*/

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
