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

    Process* get_process(const std::string& name);
    bool exists(const std::string& name) const;

    //GETTERS
    std::vector<Process*> getAllProcesses() const;
    std::mutex& getMutex();
    size_t get_process_count() const;

    //for scheduler start
    //void generate_processes(int numProcesses, int instructionsPerProcess, ConsoleManager& consoleManager);
    std::string generate_rand_instruction();
    void generate_instructions(const std::string& processName, ConsoleManager& consoleManager);


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
