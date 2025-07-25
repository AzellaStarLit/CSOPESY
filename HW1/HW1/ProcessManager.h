#pragma once

#include <unordered_map>
#include <vector>
#include <mutex>

#include "Process.h"

class ConsoleManager;

class ProcessManager {
private:

    //this is a list of all the processes [name, process]
    std::unordered_map<std::string, Process> processes;
    mutable std::mutex processMutex;

public:
    void create_process(const std::string& name);
    void create_process(const std::string& name, size_t processMemory);

    Process* get_process(const std::string& name);
    bool exists(const std::string& name) const;
    size_t getUsedMemory() const;

    //GETTERS
    std::vector<Process*> getAllProcesses() const;
    std::mutex& getMutex();
    size_t get_process_count() const;


};
