#pragma once

#include <unordered_map>
#include <string>
#include "Process.h"
#include "ConsoleManager.h"
#include <mutex>

class ConsoleManager;
class ProcessManager {
private:
    std::unordered_map<std::string, Process> processes;

    std::vector<Process*> allProcesses;
    mutable std::mutex processMutex;

public:
    void create_process(const std::string& name);
    Process* get_process(const std::string& name);
    bool exists(const std::string& name) const;
    size_t get_process_count() const;
    void generate_instructions(const std::string& processName, ConsoleManager& consoleManager);
    std::mutex& getMutex();

    const std::vector<Process*>& getAllProcesses() const { return allProcesses; }
    void addToAllProcesses(Process* process) { allProcesses.push_back(process); }
};
