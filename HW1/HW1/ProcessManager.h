#pragma once

#include <unordered_map>
#include <vector>
#include <mutex>
#include "MemoryManager.h"

#include "Process.h"

class ConsoleManager;
class MemoryManager;

class ProcessManager {
private:

    //this is a list of all the processes [name, process]
    std::unordered_map<std::string, Process> processes;
    mutable std::mutex processMutex;

    MemoryManager* memMgr = nullptr;

public:
    void create_process(const std::string& name);
    void create_process(const std::string& name, size_t memorySize, size_t frameSize, MemoryManager* memoryManager);


    Process* get_process(const std::string& name);
    bool exists(const std::string& name) const;
    size_t getUsedMemory() const;

    //GETTERS
    std::vector<Process*> getAllProcesses() const;
    std::mutex& getMutex();
    size_t get_process_count() const;

    Process* get_process_by_pid(int pid);
    void setMemoryManager(MemoryManager* m) { memMgr = m; }

};
