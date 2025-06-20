#pragma once

class ClockManager {
private:
	int cpuCycles = 0;

public:

	void incrementCycles(int cycles);
	int getCycles() const;
	void resetCycles();
};