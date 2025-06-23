#pragma once

#include <unordered_map>
#include <vector>

#include "Process.h"

class ConsoleManager;

class ProcessManager {
private:

    //this is a list of all the processes [name, process]
    std::unordered_map<std::string, Process> processes;

public:
    void create_process(const std::string& name);

    Process* get_process(const std::string& name);
    bool exists(const std::string& name) const;
    std::vector<Process*> getAllProcesses() const;

    void generate_processes(int numProcesses, int instructionsPerProcess, ConsoleManager& consoleManager);
};
