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
        processes[name] = Process(name, processMemory);
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

//generate multiple dummy processes with x number of instructions
/*
void ProcessManager::generate_processes(int numProcesses, int instructionsPerProcess, ConsoleManager& consoleManager) {
    for (int i = 0; i < numProcesses; ++i) {
        std::string processName = "process_" + std::to_string(i + 1);
        Process process(processName);

		std::cout << "Generating instructions for " << processName << "...\n";

        for (int j = 0; j < instructionsPerProcess; ++j) {
            process.add_instruction("print: Hello from " + processName + " [Line " + std::to_string(j + 1) + "]");
        }

        processes[processName] = process;
        consoleManager.attach_screen(processName, &process);
    }

    std::cout << "\033[32mGenerated " << numProcesses << " processes with "
        << instructionsPerProcess << " instructions each.\033[0m\n";
}
*/

void ProcessManager::generate_instructions(const std::string& processName, ConsoleManager& consoleManager) {
    //std::lock_guard<std::mutex> lock(processMutex);
    std::random_device rd;
    std::mt19937 gen(rd());

    //TODO: Edit this to read set values in Configuration file
    int min = 1;
    int max = 10000;
    std::uniform_int_distribution<> dis(min, max);

    int instructionCount = dis(gen);

    std::cout << "Generating " << instructionCount << " instructions for " << processName << "...\n";

    Process* process = get_process(processName);
    if (!process) return;

    for (int i = 0; i < instructionCount; ++i) {
        std::cout << "Adding instruction " << i + 1 << " to " << processName << "\n";
        process->add_instruction(generate_rand_instruction());
    }

    std::cout << "\033[32mGenerated " << instructionCount << " instructions for " << processName << ".\033[0m\n";
    consoleManager.attach_screen(processName, process);
}

std::string ProcessManager::generate_rand_instruction() {
    static const std::string instructions[] = {
        "PRINT(\"\")",
        "PRINT(\"Processing...\")",
        "PRINT(\"We love CSOPESY <3\")",
    };

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(instructions) - 1);

    return instructions[dis(gen)];
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



