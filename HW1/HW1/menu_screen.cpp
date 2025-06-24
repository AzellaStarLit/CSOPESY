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
#include <random>


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
SchedulerManager schedulerManager;
std::unordered_map<std::string, Console> screens;


bool screen_command(const std::string& command, SchedulerManager& schedulerManager) {
	std::mutex processConsoleMutex;

	std::istringstream iss(command);
	std::string screenCmd, flag, name;
	iss >> screenCmd >> flag >> name;

	if ((flag == "-s" || flag == "-r") && !name.empty()) {
		if (flag == "-s") {
			std::lock_guard<std::mutex> lock(processConsoleMutex);

			Process* proc = processManager.get_process(name);
			if (!proc) {
				processManager.create_process(name);
				proc = processManager.get_process(name);
			}

			if (proc) {
				std::vector<std::string> instructions;

				// Generate instructions here...
				std::random_device rd;
				std::mt19937 gen(rd());

				//TODO: READ minInstructions and maxInstructions from config file
				const uint32_t minInstructions = 50;
				const uint32_t maxInstructions = 10000;

				std::uniform_int_distribution<> dis(minInstructions, maxInstructions);

				int numInstructions = dis(gen);
				std::vector<std::string> dummy_instructions;
				for (int i = 0; i < numInstructions; ++i) {
					dummy_instructions.push_back(schedulerManager.generate_rand_instruction(name));
				}

				proc->load_instructions(dummy_instructions);
				consoleManager.attach_screen(name, proc);

				std::cout << "\033[32mGenerated " << instructions.size() << " instructions for process '" << name << "'.\033[0m\n";
			}

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

			return false;
		}

	} else {
		std::cout << "\033[31mInvalid screen command. Use: screen -s <name> or screen -r <name>\n\033[0m";
		return false;
	}

	return false;
}

int main() {

	// THIS PART OF THE CODE IS FOR TESTING PURPOSES ONLY
	/*
	Process p("TestProcess");

	// Sample instruction to test PRINT instruction
	std::string instruction = "PRINT(\"This is some random string\")";
	p.execute_instruction(instruction);
	std::string instruction2 = "PRINT()";
	p.execute_instruction(instruction2);
	//Sample to test logs
	p.show_log();
	*/

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
			shouldClear = screen_command(command, schedulerManager);
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