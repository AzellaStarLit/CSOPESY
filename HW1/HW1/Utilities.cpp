#include "Utilities.h"
#include "ProcessManager.h"
#include "FCFS.h"
#include "SchedulerManager.h"

#include <iostream>
#include <cstdlib>
#include <vector>

extern ProcessManager processManager;
extern ConsoleManager consoleManager;
extern SchedulerManager schedulerManager;

static FCFS scheduler;


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
	// TODO: Initialize any necessary variables or settings here
	std::cout << "initialize command recognized. Doing something.\n";
}

void scheduler_start() {
	// TODO: Start the appropriate scheduler 
	
	schedulerManager.start_scheduler(processManager, consoleManager); // currently generates processes
}

void scheduler_stop() {
	schedulerManager.stop_scheduler();
}

void report_util() {
	// TODO: Report the utilization of the system here
	std::cout << "report_util command recognized. Doing something.\n";
}

void clear() {
	// Clear the screen here
	std::system("cls");
	print_header();
}

void exit_program() {
	// Exit the screen here
	scheduler_stop();
	std::exit(0);
}