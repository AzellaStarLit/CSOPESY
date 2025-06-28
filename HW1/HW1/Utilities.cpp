#include "Utilities.h"
#include "ProcessManager.h"
#include "FCFS.h"
#include "ConfigManager.h"
#include "BaseScheduler.h"
#include "sharedState.h"

#include <iostream>
#include <cstdlib>
#include <vector>
#include <iomanip>
#include <fstream>

extern ProcessManager processManager;
extern ConsoleManager consoleManager;
extern ConfigManager configManager;
extern bool isInitialized;

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

/*
void initialize() {
	// Initialize any necessary variables or settings here
	std::cout << "initialize command recognized. Doing something.\n";
}

void report_util() {
	// Report the utilization of the system here
	std::cout << "report_util command recognized. Doing something.\n";
}*/

void clear() {
	// Clear the screen here
	std::system("cls");
	print_header();
}

void initialize() { // intializer logic
	if (isInitialized) {
		std::cout << "\033[33mSystem already initialized.\033[0m\n";
		return;
	}
	//configManager will read from config
	if (configManager.loadFromFile("config.txt")) {
		isInitialized = true;
		std::cout << "\033[32mInitialization complete.\033[0m\n";
	}
	else {
		std::cout << "\033[31mInitialization failed.\033[0m\n";
	}
}

//report-util saves screen -ls in a log file
void report_util() { // report-util logic
	if (!isInitialized) {
		std::cout << "\033[31mError: Please run 'initialize' first.\033[0m\n";
		return;
	}

	std::ofstream outfile("csopesy-log.txt");
	if (!outfile.is_open()) {
		std::cerr << "\033[31mFailed to write to csopesy-log.txt\033[0m\n";
		return;
	}

	auto allProcesses = processManager.getAllProcesses();
	if (allProcesses.empty()) {
		outfile << "No processes available.\n";
	}
	else {
		std::vector<Process*> running, finished;

		for (auto proc : allProcesses) {
			if (proc->isFinished()) finished.push_back(proc);
			else running.push_back(proc);
		}

		auto [used, available, util] = scheduler->getCPUUtilization();
		outfile << "CPU utilization: " << util << "%\n";
		outfile << "Cores used: " << used << "\n";
		outfile << "Cores available: " << available << "\n";

		outfile << "RUNNING PROCESSES:\n";
		if (running.empty()) {
			outfile << "No running processes.\n";
		}
		else {
			outfile << std::left << std::setw(20) << "Name"
				<< std::setw(10) << "Core"
				<< std::setw(15) << "Progress"
				<< "Creation Time\n";
			outfile << std::string(60, '-') << "\n";
			for (auto proc : running) {
				outfile << std::left << std::setw(20) << proc->getName()
					<< std::setw(10) << std::to_string(proc->getCurrentCore())
					<< std::setw(15) << (std::to_string(proc->getCurrentLine()) + " / " + std::to_string(proc->getTotalLines()))
					<< proc->getCreationTimestamp() << "\n";
			}
		}

		outfile << "\nFINISHED PROCESSES:\n";
		if (finished.empty()) {
			outfile << "No finished processes.\n";
		}
		else {
			outfile << std::left << std::setw(20) << "Name"
				<< std::setw(25) << "Creation Time"
				<< "Completion Time\n";
			outfile << std::string(60, '-') << "\n";
			for (auto proc : finished) {
				outfile << std::left << std::setw(20) << proc->getName()
					<< std::setw(25) << proc->getCreationTimestamp()
					<< proc->getCompletionTimestamp() << "\n";
			}
		}
	}

	outfile.close();
	std::cout << "\033[32mReport saved to csopesy-log.txt\033[0m\n";
}
