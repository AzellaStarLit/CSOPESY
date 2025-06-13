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
	}
	else {
		std::cout << "\033[31mInvalid screen command. Use: screen -s <name> or screen -r <name>\n\033[0m";
		return false;
	}
}

/*
void screen_command(const std::string& command) {
	// Function that encompasses all screen commands
	std::istringstream iss(command);
	std::string screenCmd, flag, name;
	iss >> screenCmd >> flag >> name;

	if ((flag == "-s" || flag == "-r") && !name.empty()) {
		if (flag == "-s") {
			if (screens.find(name) == screens.end()) {
				Process* process = new Process(name);
				screens[name] = Console(name, process);
				std::cout << "Screen '" << name << "' created.\n";
			} else {
				std::cout << "Screen '" << name << "' already exists. Attaching...\n";
			}
			screens[name].draw();
		} else if (flag == "-r") {
			auto it = screens.find(name);
			if (it != screens.end()) {
				std::cout << "Resuming screen '" << name << "'...\n";
				it->second.draw(); //Draws the screen when screen -r <name> is initialized 
			} else {
				std::cout << "No such screen named '" << name << "'.\n";
			}
		}
	} else {
		std::cout << "\033[31mInvalid screen command. Use: screen -s <name> or screen -r <name>\n\033[0m";
	}
}
*/

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

	// Initialize 10 processes with 10 instructions each
	scheduler_start();


	// THIS PART OF THE CODE IS FOR TESTING PURPOSES ONLY
	Process p("TestProcess");

	// Sample instruction to test PRINT instruction
	std::string instruction = "PRINT(\"This is some random string\")";
	p.execute_instruction(instruction);
	std::string instruction2 = "PRINT()";
	p.execute_instruction(instruction2);
	//Sample to test logs
	p.show_log();



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
