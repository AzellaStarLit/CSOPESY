#pragma once

#include "FCFS.h"
#include <memory>

extern std::unique_ptr<FCFSScheduler> fcfs;

void scheduler_start();
void scheduler_stop();
