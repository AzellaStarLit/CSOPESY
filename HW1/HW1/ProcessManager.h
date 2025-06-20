#pragma once

#include <unordered_map>
#include <string>
#include "Process.h"
#include "ConsoleManager.h"

class ConsoleManager;
class ProcessManager {
private:
    std::unordered_map<std::string, Process> processes;

    std::vector<Process*> allProcesses;

public:
    void create_process(const std::string& name);
    Process* get_process(const std::string& name);
    bool exists(const std::string& name) const;
    int get_process_count() const;
    void generate_instructions(int numProcesses, int instructionsPerProcess, ConsoleManager& consoleManager);

    const std::vector<Process*>& getAllProcesses() const { return allProcesses; }
    void addToAllProcesses(Process* process) { allProcesses.push_back(process); }
};
