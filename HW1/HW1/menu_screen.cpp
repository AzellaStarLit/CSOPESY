/*
CSOPESY GROUP 10
ALABAT, Jeanne Hazel 
ASTRERO, Dwight Daryl
DE CASTRO, Phoenix Claire
LABARRETE, Lance Desmond
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
#include "SchedulerManager.h"


// Global functions for initialization
void print_header();
void initialize();
void scheduler_start();
void scheduler_stop();
void report_util();
void clear();
void exit_program();

ProcessManager processManager;
ConsoleManager consoleManager;
std::unordered_map<std::string, Console> screens;
SchedulerManager schedulerManager(5, 10, 1000); // minInst, maxInst, intervalMs

bool screen_command(const std::string& command) {
	std::istringstream iss(command);
	std::string screenCmd, flag, name;
	iss >> screenCmd >> flag >> name;

	if ((flag == "-s" || flag == "-r") && !name.empty()) {
		if (flag == "-s") {
			consoleManager.create_screen_with_process(name);
			return true;
		}
		else if (flag == "-r") {
			consoleManager.resume_screen(name);
			return true;
		}
	} else if (flag == "-ls") {

		auto allProcesses = processManager.getAllProcesses();

		if (allProcesses.empty()) {
			std::cout << "No processes available.\n";
		}
		else {
			std::vector<Process*> running, finished;

			for (auto proc : allProcesses) {
				if (proc->isFinished()) finished.push_back(proc);
				else running.push_back(proc);
			}

			std::cout << "RUNNING PROCESSES: \n";
			if (running.empty()) {
				std::cout << "No running processes. \n";

			}
			else
			{
				std::cout << std::left << std::setw(20) << "Name"
					<< std::setw(10) << "Core"
					<< std::setw(15) << "Progress"
					<< "Creation Time\n";
				std::cout << std::string(60, '-') << "\n";
				for (auto proc : running) {
					std::cout << std::left << std::setw(20) << proc->getName()
						<< std::setw(10) << std::to_string(proc->getCurrentCore())
						<< std::setw(15) << (std::to_string(proc->getCurrentLine()) + " / " + std::to_string(proc->getTotalLines()))
						<< proc->getCreationTimestamp() << "\n";
				}
			}
			std::cout << "\nFINISHED PROCESSES: \n";

			if (finished.empty()) {
				std::cout << "No finished processes.\n";
			}
			else {
				std::cout << std::left << std::setw(20) << "Name"
					<< std::setw(25) << "Creation Time"
					<< "Completioin Time\n";
				std::cout << std::string(60, '-') << "\n";

				for (auto proc : finished) {
					std::cout << std::left << std::setw(20) << proc->getName()
						<< std::setw(25) << proc->getCreationTimestamp()
						<< proc->getCompletionTimestamp() << "\n";
				}
			}

			std::cout << "\n";



			/*
			if (screens.empty()) {
				std::cout << "No active screens.\n";
			} else {
				std::cout << "Active screens:\n";
				for (const auto& pair : screens) {
					std::cout << " - " << pair.first << "\n";
				}
			}*/
			return false;
		}

	} else {
		std::cout << "\033[31mInvalid screen command. Use: screen -s <name> or screen -r <name>\n\033[0m";
		return false;
	}

	return false;
}

int main() {
	
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

	print_header();

	while (true) {
		std::cout << "> ";
		std::getline(std::cin, command);

		bool shouldClear = false;

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

		if(shouldClear) {
			clear();
			shouldClear = false;
		}
	}

	return 0;
}

// Manual compilation via terminal: g++ menu_screen.cpp Utilities.cpp Console.cpp ProcessManager.cpp Process.cpp ConsoleManager.cpp FCFS.cpp -o main.exe