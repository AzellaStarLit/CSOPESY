#include <fstream>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <random>

#include "SchedulerController.h"


#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "ConfigManager.h"

extern ProcessManager processManager;
extern ConsoleManager consoleManager;
extern ConfigManager configManager;

//std::unique_ptr<FCFSScheduler> fcfs = nullptr;

std::atomic<bool> generating = false;
std::thread generatorThread; //generates the processes

extern std::unique_ptr<Scheduler> scheduler;

//TODO: GET TICK INTERVAL FROM CONFIG FILE
//int get_tick_interval() {
	
//}

void scheduler_start() {
	if (generating) return;

	generating = true;

	// Optional: configure from file later
	const uint32_t minInstructions = configManager.getMinInstructions();
	const uint32_t maxInstructions = configManager.getMaxInstructions();
	const int interval = configManager.getBatchProcessFreq();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> instructionDist(minInstructions, maxInstructions);
	std::uniform_int_distribution<> messageDist(0, 1); // change based on number of messages

	static const std::string templates[] = {
		"PRINT(\"Hello world from ",
		"PRINT(\"We love CSOPESY <3 - ",
		"DECLARE(var_x, 0)",
		"DECLARE(var_y, 5)",
		"ADD(var_z, var_x, var_y)",
		"SUBTRACT(var_a, var_y, var_x)",
		"SLEEP(3)",
		"SLEEP(2)",
		"FOR([PRINT(\"Looping inside process\")], 2)",
		"FOR([ADD(var_i, var_x, 1)], 2)",
		"FOR([SUBTRACT(var_j, var_y, 1)], 2)",
	};

	int counter = 1;

	generatorThread = std::thread([=]() mutable {
		while (generating) {
			std::string name = "process_" + std::to_string(counter++);

			{
				std::scoped_lock lock(processManager.getMutex(), consoleManager.getMutex());

				processManager.create_process(name);
				Process* p = processManager.get_process(name);

				if (p) {
					int numInstructions = instructionDist(gen);
					std::vector<std::string> instructions;
					for (int i = 0; i < numInstructions; ++i) {
						std::string base = templates[messageDist(gen)];
						instructions.push_back(base + name + "!\")");
					}

					p->load_instructions(instructions);
					consoleManager.attach_screen(name, p);

					// Add process to scheduler!
					if (scheduler) {
						scheduler->add_process(p);
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
		}
		});

	std::cout << "\033[32mScheduler started. Generating processes...\033[0m\n";
}

void scheduler_stop() {
	if (!generating) return;

	generating = false;
	if (generatorThread.joinable()) generatorThread.join();

	// if (scheduler) scheduler->stop();

	std::cout << "\033[32mScheduler stopped.\033[0m\n";
}


//generates processes every time interval
/*
void scheduler_start() {
	if (generating) return;

	generating = true;
	//fcfs->start(); //start fcfs threads

	int interval = 5000; //I put this here for testing only
	int counter = 1;

	generatorThread = std::thread([=]() mutable {
		while (generating) {

			std::string name = "process_" + std::to_string(counter++);
			Process* p = new Process(name, 10); //for testing; this should come from config too

			for (int i = 0; i < 10; i++) {
				p->add_instruction("print"); //should be randomized from list of all recognized instructions
			}

			//fcfs->add_process(p); //add the processes to the scheduler
			processManager.create_process(p->getName());
			consoleManager.attach_screen(name, p); //auto attach a screen to the process

			std::this_thread::sleep_for(std::chrono::milliseconds(interval)); //wait

		}
	});

	std::cout << "Scheduler started. Generating processes..."; 

}

void scheduler_stop() {
	if (!generating) return;

	generating = false;
	if (generatorThread.joinable()) generatorThread.join();

	fcfs->stop();
	std::cout << "Scheduler stopped.\n";
}
*/

