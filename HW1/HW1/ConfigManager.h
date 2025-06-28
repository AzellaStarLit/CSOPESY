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
    int numCPU;
    std::string scheduler;
    unsigned int quantumCycles;
    unsigned int batchProcessFreq;
    unsigned int minInstructions;
    unsigned int maxInstructions;
    unsigned int delaysPerExec;
}; 
