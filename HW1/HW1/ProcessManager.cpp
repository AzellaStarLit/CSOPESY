#include "ProcessManager.h"
#include "ConsoleManager.h"
#include <iostream>
#include <unordered_map>
#include <random>

//Create a new process by name if it does not already exist
void ProcessManager::create_process(const std::string& name) {
    if (!exists(name)) {
        processes[name] = Process(name);
        //std::cout << "Process '" << name << "' created.\n";
    }
    else {
        std::cout << "Process '" << name << "' already exists.\n";
    }
}

void ProcessManager::create_process(const std::string& name, size_t processMemory) {
    if (!exists(name)) {
        processes.try_emplace(name, name, processMemory);
        //std::cout << "Process '" << name << "' created.\n";
    }
    else {
        std::cout << "Process '" << name << "' already exists.\n";
    }
}


//retrieve a process by name
Process* ProcessManager::get_process(const std::string& name) {
    auto it = processes.find(name);
    if (it != processes.end()) {
        return &(it->second);
    }
    return nullptr;
}

//check if a process with given name exists
bool ProcessManager::exists(const std::string& name) const {
    return processes.find(name) != processes.end();
}



std::mutex& ProcessManager::getMutex() {
    return processMutex;
}

size_t ProcessManager::get_process_count() const {
    return processes.size();
}

//expose all processes for screen -ls and report-util
std::vector<Process*> ProcessManager::getAllProcesses() const{
    std::vector<Process*> result;
    for (const auto& pair : processes) {
        result.push_back(const_cast<Process*>(&pair.second));
    }
    return result;
}

//Memory for process-smi and vmstat in the main menu
size_t ProcessManager::getUsedMemory() const {
    size_t total = 0;
    for (const auto& [_, process] : processes) {
        total += process.getMemoryUsage();
    }
    return total;
}



