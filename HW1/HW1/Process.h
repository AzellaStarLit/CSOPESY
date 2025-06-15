#pragma once
#define PROCESS_H

#include <string>
#include <vector>
#include <iostream>

class Process {
private:
    std::string name;
    int totalLines;
    std::string creationTimestamp;
    std::vector<std::string> instructions;
    std::vector<std::string> log;
    int instructionPointer;

    bool finished = false;
    int currentCoreId = -1;
    std::string completionTimeStamp;

public:
	Process();
	Process(const std::string& name);
    Process(const std::string& name, int instructionCount);
    void run_print();
    void show_log() const;
    void setTimestamp();
    void add_instruction(const std::string& instr);
    void generate_instructions();
	void execute_instruction(const std::string& instruction, int coreId);
	void execute_print(const std::string& msg, int coreId);
    std::string getName() const;
    std::string getCurrentInstruction() const;
	int getCurrentLine() const;
    int getTotalLines() const;

	~Process() = default; // Default destructor

    bool isFinished() const { return finished; }
    void setFinished() {
        finished = true;
        setCompletionTime();
    }
    int getCurrentCore() const { return currentCoreId; }
    void setCurrentCore(int core) { currentCoreId = core; }
    std::string getCompletionTimestamp() const { return completionTimeStamp; }
    std::string getCreationTimestamp() const { return creationTimestamp; }
    void setCompletionTime();
};

