#pragma once

class ClockManager {
private:
	int cpuCycles = 0;

public:
	// Increment the CPU cycle count
	void incrementCycles(int cycles) {
		cpuCycles += cycles;
	}
	// Get the current CPU cycle count
	int getCycles() const {
		return cpuCycles;
	}
	// Reset the CPU cycle count
	void resetCycles() {
		cpuCycles = 0;
	}
};