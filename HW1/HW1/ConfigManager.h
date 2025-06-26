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

private:
    int numCPU = 1;
    std::string scheduler = "fcfs";
    unsigned int quantumCycles = 1;
    unsigned int batchProcessFreq = 1;
    unsigned int minInstructions = 1;
    unsigned int maxInstructions = 1;
    unsigned int delaysPerExec = 0;
}; 
