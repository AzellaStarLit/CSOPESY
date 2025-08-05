#pragma once
#define PROCESS_H

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include "MemoryManager.h"  


class MemoryManager;

enum class ProcessStatus {
    New,
    Ready,
    Running,
    Sleeping,
    Waiting,
    Finished,
    Terminated
};

class Process {
private:
	//size_t pid;
    static int global_pid_counter;
    int processId = 0;
    std::string name;
    std::string creationTimestamp;
    std::string completionTimestamp;
	size_t memorySize;
	ProcessStatus status = ProcessStatus::New; //default status

    int instructionPointer; //stores the current line being executed
    int totalLines; 

    std::vector<std::string> instructions; //list of instructions
    std::vector<std::string> log; //list of logs

    bool finished = false;
    int currentCoreId = -1; //-1 for default core [N/A]

    std::unordered_map<std::string, uint16_t> symbolTable; // variable storage

    void setTimestamp(); 
    void execute_print(const std::string& msg, int coreId); //temporary since we only execute print for now
    void execute_declare(const std::string& args);
    void execute_add(const std::string& args);
    void execute_subtract(const std::string& args);
    void execute_sleep(const std::string& msString);
    void execute_for(const std::string& args, int coreId, int nestingLevel);
    void execute_read(const std::string& args);
    void execute_write(const std::string& args);

    size_t frameSize;
    MemoryManager* memoryManager;

    size_t pageIns = 0;
    size_t pageOuts = 0;

    void ensureResidentVar(const std::string& varName);
    void touchForWriteVar(const std::string& varName);
   
public:

    //constructors
	Process(); //default
	Process(const std::string& name); //when a process is given a name
    Process(const std::string& name, size_t memorySize, size_t frameSize, MemoryManager* memoryManager);


    //GENERATE INSTRUCTIONS AND EXECUTE INSTRUCTIONS
    void execute_instruction(const std::string& instruction, int coreId);
    void load_instructions(const std::vector<std::string>& instrs);
    void add_instruction(const std::string& instr);
    void incrementInstructionPointer();

	//TODO: Clean up unused functions
    void run_print(); //used this for the current version
    void show_log() const; //logs
    void show_symbol_table() const;
    const std::vector<std::string>& get_log() const;
    
    // GETTERS
    std::string getName() const;
    std::string getCurrentInstruction() const;
	int getCurrentLine() const;
    int getTotalLines() const;
    int getCurrentCore() const { return currentCoreId; }
    ProcessStatus getStatus() const;
    std::string getStatusString() const;

    std::string getCreationTimestamp() const { return creationTimestamp; }
    std::string getCompletionTimestamp() const { return completionTimestamp; }
    std::string get_current_timestamp() const;
    int getPID() const { return processId; }
    
    //SETTERS
    void setPID(int pid) { this->processId = pid; }
    void setCompletionTimestamp();
    void setCurrentCore(int core) { currentCoreId = core; }
    void setStatus(ProcessStatus newStatus);

    void markFinished();
    bool isFinished();

   // For process-smi and vmstat in the main menu
    size_t getMemoryUsage() const { return memorySize; }

    bool isSleeping() const {
        return instructionPointer < instructions.size() &&
               instructions[instructionPointer].find("SLEEP") != std::string::npos;
    }

    //page table
    struct PageTableEntry {
        bool valid = false; //valid bits [false == not in memory; true == in memory]
        size_t frameNumber = -1; //if valid, frame number
        bool dirty = false; //modified?
    };
    std::unordered_map<size_t, PageTableEntry> pageTable; //list of page tables per process
    PageTableEntry& getPageEntry(size_t pageNum); 

	//Backing store
    std::string backingStorePath; //backing store file for each process
    void initializeBackingStore();
    const std::string& getBackingStorePath() const;
    
    size_t getVirtualAddressForVar(const std::string& varName) const;

	~Process() = default; // Default destructor

    // page?fault statistics
    void incrementPageIns() { ++pageIns; }
    void incrementPageOuts() { ++pageOuts; }
    size_t getPageIns()  const { return pageIns; }
    size_t getPageOuts() const { return pageOuts; }

    bool hasResidentPage() const;

    std::string int_to_hex(size_t value) const;

};

