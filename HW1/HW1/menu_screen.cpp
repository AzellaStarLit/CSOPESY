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


// Global functions for initialization
void print_header();
void initialize();
void scheduler_test();
void scheduler_stop();
void report_util();
void clear();
void exit_program();


// Class Declaration of the console
class Console {
private:
	std::string name;
	int currentLine;
	int totalLines;
	std::string creationTimestamp;

	void setTimestamp() {
		// Declares a timestamp
		std::time_t now = std::time(nullptr);
		std::tm* localTime = std::localtime(&now);
		std::ostringstream oss;
		oss << std::put_time(localTime, "%m/%d/%Y, %I:%M:%S %p");
		creationTimestamp = oss.str();
	}

public:
	Console() : name("default"), currentLine(0), totalLines(100) {
		setTimestamp();
	}

	Console(const std::string& name) : name(name), currentLine(0), totalLines(100) {
		// Get current time and format it
		std::time_t now = std::time(nullptr);
		std::tm* localTime = std::localtime(&now);
		std::ostringstream oss;
		oss << std::put_time(localTime, "%m/%d/%Y, %I:%M:%S %p");
		creationTimestamp = oss.str();
	}

	void draw() {
		// Draws the screen
		std::cout << "\033[36m--- Screen: " << name << " ---\033[0m\n";
		std::cout << "Process Name: " << name << "\n";
		std::cout << "Instruction: Line " << currentLine << " / " << totalLines << "\n";
		std::cout << "Created At: " << creationTimestamp << "\n";
		std::cout << "Type 'exit' to return to the main menu.\n";

		std::string input;
		while (true) {
			std::cout << "[" << name << "]> ";
			std::getline(std::cin, input);
			if (input == "exit"){
				clear();
				break;
			}
		}
	}
};

std::unordered_map<std::string, Console> screens;


void screen_command(const std::string& command) {
	// Function that encompasses all screen commands
	std::istringstream iss(command);
	std::string screenCmd, flag, name;
	iss >> screenCmd >> flag >> name;

	if ((flag == "-s" || flag == "-r") && !name.empty()) {
		if (flag == "-s") {
			if (screens.find(name) == screens.end()) {
				screens[name] = Console(name);
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

int main() {
	
	std::string command;
	std::unordered_map<std::string, void(*)()> commandMap = {
		// List of commands for the Main Menu
		{"initialize", initialize},
		{"scheduler-test", scheduler_test},
		{"scheduler-stop", scheduler_stop},
		{"report-util", report_util},
		{"clear", clear},
		{"exit", exit_program}
	};

	print_header();

	while (true) {
		std::cout << "> ";
		std::getline(std::cin, command);

		auto typedCommand = commandMap.find(command);
		if (command.rfind("screen ", 0) == 0) {
			screen_command(command);
		}	
		else if (typedCommand != commandMap.end()) {
			commandMap[command](); // Call the function associated with the command
		}
		else {
			std::cout << "\033[31mCommand not recognized. Please try again.\n\033[0m";
			std::cout << "\033[1;33mType \'exit\' to quit, \'clear\' to clear the screen\n\033[0m";
		}
	}

	return 0;
}
