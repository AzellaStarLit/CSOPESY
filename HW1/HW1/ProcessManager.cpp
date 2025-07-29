#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "MemoryManager.h"  
#include <iostream>
#include <unordered_map>
#include <random>

//Create a new process by name if it does not already exist
void ProcessManager::create_process(const std::string& name) {
    if (!exists(name)) {
        //processes[name] = Process(name);
        processes.emplace(name, Process(name));
        //std::cout << "Process '" << name << "' created.\n";
    }
    else {
        std::cout << "Process '" << name << "' already exists.\n";
    }
}

void ProcessManager::create_process(const std::string& name, size_t memorySize, size_t frameSize, MemoryManager* memoryManager) {

    std::lock_guard<std::mutex> lock(processMutex);
    auto it = processes.find(name);

    if (it == processes.end()) {
        processes.emplace(std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(name, memorySize, frameSize, memoryManager));
    }
    else {
        std::cout << "Process '" << name << "' already exists.\n";
    }

    /*
    if (!exists(name)) {
        processes.try_emplace(name, name, memorySize, frameSize, memoryManager);
        //std::cout << "Process '" << name << "' created.\n";
    }
    else {
        std::cout << "Process '" << name << "' already exists.\n";
    }*/
}


//retrieve a process by name
Process* ProcessManager::get_process(const std::string& name) {
   
    std::lock_guard<std::mutex> lock(processMutex);

    auto it = processes.find(name);
    if (it != processes.end()) {
        return &(it->second);
    }
    return nullptr;
}

//check if a process with given name exists
bool ProcessManager::exists(const std::string& name) const {

    std::lock_guard<std::mutex> lock(processMutex);

    return processes.find(name) != processes.end();
}



std::mutex& ProcessManager::getMutex() {
    return processMutex;
}

size_t ProcessManager::get_process_count() const {
    std::lock_guard<std::mutex> lock(processMutex);

    return processes.size();
}

//expose all processes for screen -ls and report-util
std::vector<Process*> ProcessManager::getAllProcesses() const{
    std::lock_guard<std::mutex> lock(processMutex);

    std::vector<Process*> result;
    for (const auto& pair : processes) {
        result.push_back(const_cast<Process*>(&pair.second));
    }
    return result;
}

//Memory for process-smi and vmstat in the main menu
size_t ProcessManager::getUsedMemory() const {
   /*
    size_t total = 0;
    for (const auto& [_, process] : processes) {
        total += process.getMemoryUsage();
    }
    return total;*/

    return memMgr ? memMgr->getUsedFrames() * memMgr->getFrameSize()
        : 0;
}


Process* ProcessManager::get_process_by_pid(int pid) {
    std::lock_guard<std::mutex> lock(processMutex);

    for (auto& [name, proc] : processes) {
        if (proc.getPID() == pid)
            return &proc;
    }
    return nullptr;
}
