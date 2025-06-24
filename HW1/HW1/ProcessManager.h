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

    //for scheduler start
    void generate_processes(int numProcesses, int instructionsPerProcess, ConsoleManager& consoleManager);

    //for testing the scheduler 
    void create_dummy(const std::string& name, int instructionCount) {
        if (!exists(name)) {
            Process p(name, instructionCount);
            for (int i = 0; i < instructionCount; i++) {
                p.add_instruction("PRINT(Line " + std::to_string(i + 1) + ")");
            }

            processes[name] = p;
        }
    }
};
