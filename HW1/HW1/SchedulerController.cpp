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
#include "Utilities.h"

extern ProcessManager processManager;
extern ConsoleManager consoleManager;
extern ConfigManager configManager;

//std::unique_ptr<FCFSScheduler> fcfs = nullptr;

std::atomic<bool> generating = false;
std::thread generatorThread; //generates the processes

extern std::unique_ptr<Scheduler> scheduler;

void scheduler_start() {
	if (generating) return;

	generating = true;

	// CONFIGURE FROM FILE

	const uint32_t minInstructions = configManager.getMinInstructions();
	const uint32_t maxInstructions = configManager.getMaxInstructions();
	const int interval = configManager.getBatchProcessFreq();

	const size_t maxMemPerProcess = configManager.getMaxMemPerProc();
	const size_t minMemPerProcess = configManager.getMinMemPerProc();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> instructionDist(minInstructions, maxInstructions);
	std::uniform_int_distribution<> messageDist(0, 10); // change based on number of messages

	std::vector<int> validMemSizes = getPowerOfTwoSizesInRange();

	if (validMemSizes.empty()) {
		std::cerr << "\033[31mNo valid power-of-two memory sizes found in config range!\033[0m\n";
		return;
	}

	std::uniform_int_distribution<> memDist(0, validMemSizes.size() - 1);

	static const std::string templates[] = {
			"DECLARE(var_x, 0)",
			"DECLARE(var_y, 5)",
			"ADD(var_z, var_x, var_y)",
			"SUBTRACT(var_a, var_y, var_x)",
			"SLEEP(300)",
			"SLEEP(2000)",
			"FOR([PRINT(\"Looping inside process\")], 2)",
			"FOR([ADD(var_i, var_x, 1)], 2)",
			"FOR([SUBTRACT(var_j, var_y, 1)], 2)",
			"PRINT(\"Hello world from process\")",
			"PRINT(\"We love CSOPESY <3\")"
	};

	int counter = 1;
	int pidCounter = 0;

	generatorThread = std::thread([=]() mutable {
		while (generating) {
			std::string name = "process_" + std::to_string(counter++);

			{
				std::scoped_lock lock(processManager.getMutex(), consoleManager.getMutex());


				int memSize = validMemSizes[memDist(gen)];

				processManager.create_process(name, memSize);
				Process* p = processManager.get_process(name);

				if (p) {
					p->setPID(pidCounter++);
					std::cerr << p->getPID();

					int numInstructions = instructionDist(gen);

					std::vector<std::string> instructions;
					for (int i = 0; i < numInstructions; ++i) {
						std::string base = templates[messageDist(gen)];
						instructions.push_back(base);
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

	std::cout << "\033[32mScheduler stopped.\033[0m\n";
}