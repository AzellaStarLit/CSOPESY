#pragma once
#define PROCESS_H

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>


class Process {
private:
    std::string name;
    std::string creationTimestamp;
    std::string completionTimestamp;

    int instructionPointer; //stores the current line being executed
    int totalLines; 

    std::vector<std::string> instructions; //list of instructions
    std::vector<std::string> log;

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

    //TODO: GENERATE INSTRUCTIONS AND EXECUTE INSTRUCTIONS
    void generate_instructions();
    void execute_instruction(const std::string& instruction, int coreId);

    void add_instruction(const std::string& instr);
    void incrementInstructionPointer();

    void run_print(); //used this for the current version
    void show_log() const; //logs
    void show_symbol_table() const;
    
    std::string getName() const;
    std::string getCurrentInstruction() const;
	int getCurrentLine() const;
    int getTotalLines() const;
    int getCurrentCore() const { return currentCoreId; }

    std::string getCreationTimestamp() const { return creationTimestamp; }
    std::string getCompletionTimestamp() const { return completionTimestamp; }

    void setCompletionTimestamp();
    void setCurrentCore(int core) { currentCoreId = core; }
    void markFinished();
    bool isFinished();

	~Process() = default; // Default destructor
};

