#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <functional>

bool ConfigManager::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "\033[31mFailed to open config file: " << filename << "\033[0m\n";
        return false;
    }

    std::unordered_map<std::string, std::function<void(const std::string&)>> setters = {
        {"num-cpu", [this](const std::string& val) { numCPU = std::stoi(val); }},
        {"scheduler", [this](const std::string& val) { scheduler = val; }},
        {"quantum-cycles", [this](const std::string& val) { quantumCycles = std::stoul(val); }},
        {"batch-process-freq", [this](const std::string& val) { batchProcessFreq = std::stoul(val); }},
        {"min-ins", [this](const std::string& val) { minInstructions = std::stoul(val); }},
        {"max-ins", [this](const std::string& val) { maxInstructions = std::stoul(val); }},
        {"delays-per-exec", [this](const std::string& val) { delaysPerExec = std::stoul(val); }},
        // New configurations
        {"max-overall-mem", [this](const std::string& val) { maxOverallMem = std::stoul(val); }},
        {"mem-per-frame", [this](const std::string& val) { memPerFrame = std::stoul(val); }},
        {"min-mem-per-proc",[this](const std::string& val) { minMemPerProc = std::stoul(val); }},
        {"max-mem-per-proc", [this](const std::string& val) { maxMemPerProc = std::stoul(val); }},
    };

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (!(iss >> key >> value)) continue;

        auto it = setters.find(key);
        if (it != setters.end()) {
            it->second(value);
        }
    }

    std::cout << "\033[32mConfiguration loaded successfully.\033[0m\n";
    return true;
}

void ConfigManager::printConfig() const {
    std::cout << "\033[34mCurrent Configuration:\033[0m\n";
    std::cout << "num-cpu: " << numCPU << "\n";
    std::cout << "scheduler: " << scheduler << "\n";
    std::cout << "quantum-cycles: " << quantumCycles << "\n";
    std::cout << "batch-process-freq: " << batchProcessFreq << "\n";
    std::cout << "min-ins: " << minInstructions << "\n";
    std::cout << "max-ins: " << maxInstructions << "\n";
    std::cout << "delays-per-exec: " << delaysPerExec << "\n";
    // New configurations
    std::cout << "max-overall-mem: " << maxOverallMem << "\n";
    std::cout << "mem-per-frame: " << memPerFrame << "\n";
    std::cout << "min-mem-per-proc: " << minMemPerProc << "\n";
    std::cout << "max-mem-per-proc: " << maxMemPerProc << "\n";
}
