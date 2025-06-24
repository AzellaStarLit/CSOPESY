#include <fstream>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

#include "SchedulerController.h"
//INCLUDE RR TOO

#include "ProcessManager.h"
#include "ConsoleManager.h"

extern ProcessManager processManager;
extern ConsoleManager consoleManager;

std::unique_ptr<FCFSScheduler> fcfs = nullptr;

std::atomic<bool> generating = false;
std::thread generatorThread; //generates the processes

//TODO: GET TICK INTERVAL FROM CONFIG FILE
//int get_tick_interval() {
	
//}

//generates processes every time interval
void scheduler_start() {
	if (generating) return;

	generating = true;
	fcfs->start(); //start fcfs threads

	int interval = 5000; //I put this here for testing only
	int counter = 1;

	generatorThread = std::thread([=]() mutable {
		while (generating) {

			std::string name = "process_" + std::to_string(counter++);
			Process* p = new Process(name, 10); //for testing; this should come from config too

			for (int i = 0; i < 10; i++) {
				p->add_instruction("print"); //should be randomized from list of all recognized instructions
			}

			fcfs->add_process(p); //add the processes to the scheduler
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