#pragma once

#include "FCFS.h"
#include <memory>
#include  "MemoryManager.h"

extern std::unique_ptr<FCFSScheduler> fcfs;

void scheduler_start();
void scheduler_stop();
