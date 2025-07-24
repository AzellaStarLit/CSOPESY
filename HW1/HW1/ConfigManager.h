#pragma once

#include <string>

class ConfigManager {
public:
    bool loadFromFile(const std::string& filename);

    int getNumCPU() const { return numCPU; }
    std::string getScheduler() const { return scheduler; }
    unsigned int getQuantumCycles() const { return quantumCycles; }
    unsigned int getBatchProcessFreq() const { return batchProcessFreq; }
    unsigned int getMinInstructions() const { return minInstructions; }
    unsigned int getMaxInstructions() const { return maxInstructions; }
    unsigned int getDelaysPerExec() const { return delaysPerExec; }

    unsigned int getMaxOverallMem() const { return maxOverallMem; }
    unsigned int getMemPerFrame() const { return memPerFrame; }
    unsigned int getMinMemPerProc() const { return minMemPerProc; }
	unsigned int getMaxMemPerProc() const { return maxMemPerProc; }

	void printConfig() const;

private:
    int numCPU;
    std::string scheduler;
    unsigned int quantumCycles;
    unsigned int batchProcessFreq;
    unsigned int minInstructions;
    unsigned int maxInstructions;
    unsigned int delaysPerExec;

    unsigned int maxOverallMem;
    unsigned int memPerFrame;
    unsigned int minMemPerProc;
    unsigned int maxMemPerProc;
}; 
