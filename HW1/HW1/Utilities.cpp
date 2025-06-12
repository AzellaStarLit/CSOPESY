#include "Utilities.h"
#include <iostream>
#include <cstdlib>

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

void initialize() {
	// Initialize any necessary variables or settings here
	std::cout << "initialize command recognized. Doing something.\n";
}

void scheduler_test() {
	// Test the scheduler functionality here
	std::cout << "scheduler_test command recognized. Doing something.\n";
}

void scheduler_stop() {
	// Stop the scheduler here
	std::cout << "scheduler_stop command recognized. Doing something.\n";
}

void report_util() {
	// Report the utilization of the system here
	std::cout << "report_util command recognized. Doing something.\n";
}

void clear() {
	// Clear the screen here
	std::system("cls");
	print_header();
}

void exit_program() {
	// Exit the screen here
	std::exit(0);
}