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


void print_header() {

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

void initialize() {
	// Initialize any necessary variables or settings here
	std::cout << "initialize command recognized. Doing something.\n";
}

void screen() {
	std::cout << "screen command recognized. Doing something.\n";
}

void scheduler_test(){
	// Test the scheduler functionality here
	std::cout << "scheduler_test command recognized. Doing something.\n";
}

void scheduler_stop(){
	// Stop the scheduler here
	std::cout << "scheduler_stop command recognized. Doing something.\n";
}

void report_util(){
	// Report the utilization of the system here
	std::cout << "report_util command recognized. Doing something.\n";
}

void clear() {
	// Clear the screen here
	std::cout << "clear command recognized. Doing something.\n";
}

void exit_program() {
	std::cout << "exit_program command recognized. Doing something.\n";
}

int main() {
	
	std::string command;
	std::unordered_map<std::string, void(*)()> commandMap = {
		{"initialize", initialize},
		{"screen", screen},
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
		if (typedCommand != commandMap.end()) {
			typedCommand->second(); // Call the function associated with the command
		}
		else {
			std::cout << "\033[31mCommand not recognized. Please try again.\n\033[0m";
			std::cout << "\033[1;33mType \'exit\' to quit, \'clear\' to clear the screen\n\033[0m";
		}
	}

	return 0;
}