#pragma once
#define PROCESS_H

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>


class Process {
private:
	//size_t pid;
    std::string name;
    std::string creationTimestamp;
    std::string completionTimestamp;
	size_t memorySize;

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

   
public:

    //constructors
	Process(); //default
	Process(const std::string& name); //when a process is given a name
    Process(const std::string& name, int instructionCount); //when a process is given a name and instruction count
    Process(const std::string& name, size_t memory);


    //TODO: GENERATE INSTRUCTIONS AND EXECUTE INSTRUCTIONS
    void generate_instructions();
	std::string generate_rand_instruction();
    void execute_instruction(const std::string& instruction, int coreId);
    void load_instructions(const std::vector<std::string>& instrs);

    void add_instruction(const std::string& instr);

    void incrementInstructionPointer();

	//TODO: Clean up unused functions
    void run_print(); //used this for the current version
    void show_log() const; //logs
    void show_symbol_table() const;
    const std::vector<std::string>& get_log() const;
    
    std::string getName() const;
    std::string getCurrentInstruction() const;
	int getCurrentLine() const;
    int getTotalLines() const;
    int getCurrentCore() const { return currentCoreId; }

    std::string getCreationTimestamp() const { return creationTimestamp; }
    std::string getCompletionTimestamp() const { return completionTimestamp; }
    std::string get_current_timestamp() const;

    void setCompletionTimestamp();
    void setCurrentCore(int core) { currentCoreId = core; }
    void markFinished();
    bool isFinished();

   // For process-smi and vmstat in the main menu
    size_t getMemoryUsage() const { return memorySize; }

    bool isSleeping() const {
        return instructionPointer < instructions.size() &&
               instructions[instructionPointer].find("SLEEP") != std::string::npos;
    }


	~Process() = default; // Default destructor
};

