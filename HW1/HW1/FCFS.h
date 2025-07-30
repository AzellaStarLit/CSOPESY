#pragma once

#include "BaseScheduler.h"
#include "Process.h"
#include "MemoryManager.h"

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

class FCFSScheduler : public Scheduler {
public:
	//constructor
	FCFSScheduler(int cores, size_t frameSz, MemoryManager* memMgr);

	void start() override; //start the algo
	void stop() override; //stop and join threads

	Process* get_next_process() override;
	void add_process(Process* p) override;

	void setMemoryManager(MemoryManager* mgr) { memoryManager = mgr; }

	std::mutex admissionMutex;
	size_t reservedFrames = 0;

private:
	//worker thread loop for each core
	void worker_loop(int coreId);
	std::vector<std::thread> workers; //list of worker threads [coreId, thread]

	//mutex and cv for snychronization
	std::mutex queueMutex;
	std::condition_variable cv;

	MemoryManager* memoryManager = nullptr;
	size_t memPerFrame = 0; 

};
