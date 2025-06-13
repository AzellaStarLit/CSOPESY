#include "ProcessManager.h"
#include "ConsoleManager.h"
#include <iostream>
#include <unordered_map>

void ProcessManager::create_process(const std::string& name) {
    if (!exists(name)) {
        processes[name] = Process(name);
        std::cout << "Process '" << name << "' created.\n";
    }
    else {
        std::cout << "Process '" << name << "' already exists.\n";
    }
}

Process* ProcessManager::get_process(const std::string& name) {
    auto it = processes.find(name);
    if (it != processes.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool ProcessManager::exists(const std::string& name) const {
    return processes.find(name) != processes.end();
}

void ProcessManager::generate_instructions(int numProcesses, int instructionsPerProcess, ConsoleManager& consoleManager) {
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



