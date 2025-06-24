#pragma once

#include "BaseScheduler.h"
#include "Process.h"

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

class FCFSScheduler : public Scheduler {
public:
	//constructor
	FCFSScheduler(int cores);

	void start() override; //start the algo
	void stop() override; //stop and join threads

	Process* get_next_process() override;

private:
	//worker thread loop for each core
	void worker_loop(int coreId);
	std::vector<std::thread> workers; //list of worker threads [coreId, thread]

	//mutex and cv for snychronization
	std::mutex queueMutex;
	std::condition_variable cv;

};
