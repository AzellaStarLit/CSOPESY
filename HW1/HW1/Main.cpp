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
#include <ctime>
#include <iomanip>
#include <sstream>

#include "Utilities.h"
#include "Console.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "FCFS.h"
#include "SchedulerController.h"


ProcessManager processManager;
ConsoleManager consoleManager;

// Global functions for initialization
void print_header();
void initialize();
void scheduler_start();
//void scheduler_stop();
void report_util();
void clear();
void exit_program();

//this is for the screen -s/-ls/-r commands
bool screen_command(const std::string& command) {

	//get the command
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
		if (processManager.exists(name)) {
			std::cout << "Process '" << name << "' already exists. Use 'screen -r " << name << "' to resume.\n";
			return false;
		}

		consoleManager.create_screen_with_process(name); 
		return true;
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

	int main() {

		int cores = 4; //TODO: CORES WILL BE READ FROM COFIG FILE

		fcfs = std::make_unique<FCFSScheduler>(cores);

		//get the processes in process manager then load it to the ready queue
		auto all = processManager.getAllProcesses();
		for (auto* p : all) {
			fcfs->add_process(p);
		}

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


		//prints the ASCII header
		print_header();

		// TEST: Manually create 10 processes with 100 instructions
		for (int i = 1; i <= 10; ++i) {
			std::string name = "process_" + std::to_string(i);
			
			processManager.create_dummy(name, 100);            // register the process in manager

			Process* p = processManager.get_process(name);

			fcfs->add_process(p);                           // add to ready queue
			consoleManager.attach_screen(name, p);          // create screen for the process
		}

		fcfs->start();

		while (true) {

			//TODO: CHECK IF THE OS IS INITIALIZED BEFORE RECOGNIZING ANY COMMAND
			
			//get the user input
			std::cout << "> ";
			std::getline(std::cin, command);

			bool shouldClear = false;

			//when the process screen is found, clear the menu screen then display the process screen
			auto typedCommand = commandMap.find(command);
			if (command.rfind("screen ", 0) == 0) {
				shouldClear = screen_command(command);
			} 
			else if (typedCommand != commandMap.end()) {
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