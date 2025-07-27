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

    auto isPowerOfTwoInRange = [](unsigned long v) {
        return v >= 64 && v <= 65536 && (v & (v - 1)) == 0;
        };

    std::unordered_map<std::string, std::function<void(const std::string&)>> setters = {
        {"num-cpu", [this](const std::string& val) {
            int v = std::stoi(val);
            if (v < 1 || v > 128)
                throw std::runtime_error("num-cpu must be in [1, 128]");
            numCPU = v;
        }},
        {"scheduler", [this](const std::string& val) {
            if (val != "fcfs" && val != "rr")
                throw std::runtime_error("scheduler must be 'fcfs' or 'rr'");
            scheduler = val;
        }},
        {"quantum-cycles", [this](const std::string& val) {
            uint32_t v = std::stoul(val);
            if (v < 0)
                throw std::runtime_error("quantum-cycles must be >= 1");
            quantumCycles = v;
        }},
        {"batch-process-freq", [this](const std::string& val) {
            uint32_t v = std::stoul(val);
            if (v < 0)
                throw std::runtime_error("batch-process-freq must be >= 1");
            batchProcessFreq = v;
        }},
        {"min-ins", [this](const std::string& val) {
            unsigned long v = std::stoul(val);
            if (v < 1)
                throw std::runtime_error("min-ins must be >= 1");
            minInstructions = v;
        }},
        {"max-ins", [this](const std::string& val) {
            unsigned long v = std::stoul(val);
            if (v < 1)
                throw std::runtime_error("max-ins must be >= 1");
            maxInstructions = v;
        }},
        {"delays-per-exec", [this](const std::string& val) {
            unsigned long v = std::stoul(val);
            if (v > UINT32_MAX || v < 0)
                throw std::runtime_error("delays-per-exec must be in [0, 2^32]");
            delaysPerExec = v;
        }},
        {"max-overall-mem", [this, isPowerOfTwoInRange](const std::string& val) {
            unsigned long v = std::stoul(val);
            if (!isPowerOfTwoInRange(v))
                throw std::runtime_error("max-overall-mem must be power of two in [64, 65536]");
            maxOverallMem = v;
        }},
        {"mem-per-frame", [this, isPowerOfTwoInRange](const std::string& val) {
            unsigned long v = std::stoul(val);
            if (!isPowerOfTwoInRange(v))
                throw std::runtime_error("mem-per-frame must be power of two in [64, 65536]");
            memPerFrame = v;
        }},
        {"min-mem-per-proc", [this, isPowerOfTwoInRange](const std::string& val) {
            unsigned long v = std::stoul(val);
            if (!isPowerOfTwoInRange(v))
                throw std::runtime_error("min-mem-per-proc must be in [64, 65536]");
            minMemPerProc = v;
        }},
        {"max-mem-per-proc", [this, isPowerOfTwoInRange](const std::string& val) {
            unsigned long v = std::stoul(val);
            if (!isPowerOfTwoInRange(v))
                throw std::runtime_error("max-mem-per-proc must be in [64, 65536]");
            maxMemPerProc = v;
        }},
    };

    try {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key, value;
            if (!(iss >> key >> value)) continue;  // skip malformed lines

            auto it = setters.find(key);
            if (it != setters.end()) {
                it->second(value);
            }
            else {
                std::cerr << "\033[33mWarning: Unknown config key '" << key << "' ignored.\033[0m\n";
            }
        }

        // Post-validation cross-checks
        if (minInstructions > maxInstructions) {
            std::cerr << "\033[33mWarning: min-ins > max-ins, swapping values.\033[0m\n";
            std::swap(minInstructions, maxInstructions);
        }

        if (minMemPerProc > maxMemPerProc) {
            std::cerr << "\033[33mWarning: min-mem-per-proc > max-mem-per-proc, swapping values.\033[0m\n";
            std::swap(minMemPerProc, maxMemPerProc);
        }

        if (maxOverallMem % memPerFrame != 0) {
            std::cerr << "\033[33mWarning: max-overall-mem not divisible by mem-per-frame; may cause rounding errors.\033[0m\n";
        }

        if (scheduler == "fcfs" && quantumCycles > 1) {
            std::cerr << "\033[34mNote: quantum-cycles ignored when scheduler = fcfs.\033[0m\n";
        }

        std::cout << "\033[32mConfiguration loaded successfully.\033[0m\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "\033[31mConfig load error: " << e.what() << "\033[0m\n";
        return false;
    }
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
