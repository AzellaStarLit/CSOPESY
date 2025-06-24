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
	// Initialize any necessary variables or settings here
	std::cout << "initialize command recognized. Doing something.\n";
}

/*
void scheduler_start() {
	std::cout << "Starting scheduler and generating processes...\n";

	for(int i = 1; i <= 10; i++){
		std::string name = "process_" + std::to_string(i);
		Process* proc = new Process(name, 100);

		for (int j = 0; j < 100; ++j) {
			proc->add_instruction("");
		}

		scheduler.addProcess(proc);
		processManager.addToAllProcesses(proc);
	}

	scheduler.start();


	processManager.generate_instructions(10, 100, consoleManager);
}
*/

void scheduler_start() {
	schedulerManager.start_scheduler(processManager, consoleManager);
}

void scheduler_stop() {
	schedulerManager.stop_scheduler();
}

/*
void scheduler_stop() {
	// Stop the scheduler here
	//std::cout << "scheduler_stop command recognized. Doing something.\n";
	scheduler.stop();
}
*/

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
	scheduler_stop();
	std::exit(0);
}