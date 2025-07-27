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
extern std::unique_ptr<MemoryManager> memoryManager;


//std::unique_ptr<FCFSScheduler> fcfs = nullptr;

std::atomic<bool> generating = false;
std::thread generatorThread; //generates the processes

extern std::unique_ptr<Scheduler> scheduler;

void scheduler_start() {
	if (generating) return;

	generating = true;

	// CONFIGURE FROM FILE
	const int interval = configManager.getBatchProcessFreq();
	const size_t maxMemPerProcess = configManager.getMaxMemPerProc();
	const size_t minMemPerProcess = configManager.getMinMemPerProc();
	const size_t memPerFrame = configManager.getMemPerFrame();


	std::random_device rd;
	std::mt19937 gen(rd());
	
	std::vector<int> validMemSizes = getPowerOfTwoSizesInRange();

	if (validMemSizes.empty()) {
		std::cerr << "\033[31mNo valid power-of-two memory sizes found in config range!\033[0m\n";
		return;
	}

	std::uniform_int_distribution<> memDist(0, validMemSizes.size() - 1);

	int counter = 1;

	generatorThread = std::thread([&]() mutable {
		{
			while (generating) {
				std::string name = "process_" + std::to_string(counter++);

				{
					std::scoped_lock lock(processManager.getMutex(), consoleManager.getMutex());

					int memSize = validMemSizes[memDist(gen)];

					processManager.create_process(name, memSize, memPerFrame, memoryManager.get());
					Process* p = processManager.get_process(name);

					if (p) {

						std::vector<std::string> instructions = generate_instructions();

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