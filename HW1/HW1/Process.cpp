#include "Process.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>

//this is a list of instructions recognized by a process
std::unordered_map<std::string, std::function<void(const std::string&)>> instructionList;

void Process::setTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%m/%d/%Y, %I:%M:%S %p");
    creationTimestamp = oss.str();
}

void Process::setCompletionTimestamp(){
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%m/%d/%Y, %I:%M:%S %p");
    completionTimestamp = oss.str();
}

//------------------CONSTRUCTORS------------------//

Process::Process()
    : name("default"), instructionPointer(0), totalLines(0) {
    setTimestamp();

    //so far, we only have print
    instructionList["PRINT"] = [this](const std::string& msg) {
        execute_print(msg, -1); };
}

Process::Process(const std::string& name)
    : name(name), instructionPointer(0), totalLines(0) {
    setTimestamp();
	instructionList["PRINT"] = [this](const std::string& msg) { 
        execute_print(msg, -1); };
}

Process::Process(const std::string& name, int instructionCount)
    : name(name), instructionPointer(0), totalLines(instructionCount) {
    setTimestamp();
    instructionList["PRINT"] = [this](const std::string& msg) {
        execute_print(msg, -1); };
}


//------------------INSTRUCTIONS------------------//

void Process::add_instruction(const std::string& instr) {
    instructions.push_back(instr);
    totalLines = instructions.size();
}

//this will generate and add instructions to the list
void Process::generate_instructions() {
    int count = 10;

    for (int i = 0; i < count; ++i) {
        add_instruction("print: Hello World! from " + name + " [Line " + std::to_string(i + 1) + "]");
        totalLines = instructions.size();
    }
}

//------------------EXECUTION------------------//

//this looks for the generated instructions in the list and calls the corresponding function to execute it 
void Process::execute_instruction(const std::string& instruction, int coreId) {
    size_t parenStart = instruction.find('(');
    size_t parenEnd = instruction.find(')', parenStart);

    if (parenStart == std::string::npos || parenEnd == std::string::npos) {
        std::cout << "Invalid instruction format: " << instruction << std::endl;
        return;
    }

    std::string command = instruction.substr(0, parenStart);
    std::string argument = instruction.substr(parenStart + 1, parenEnd - parenStart - 1);

    if (command == "PRINT") {
        execute_print(argument, coreId);
        instructionPointer++;
        if (instructionPointer >= totalLines) markFinished();
    }
    else {
        std::cout << "Unknown command: " << command << std::endl;
    }
}

//this is the execution logic of the PRINT command
void Process::execute_print(const std::string& msg, int coreId) {
    
    //start timestamp for each instruction
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%m/%d/%Y, %I:%M:%S %p");
    std::string timestamp = oss.str();

    std::string printMessage;

    //TODO: UPDATE TO PRINT CORE
    if (msg.empty()) {
        printMessage = "\"Hello world from " + name + "!\"";
    }
    else {
        printMessage = msg + " from " + name;
    }

    //Compose output string
    std::string output = "(" + timestamp + ") Core: " + std::to_string(coreId) + " " + printMessage;

    //Append to log file
    std::ofstream logFile("logs/" + name + "_log.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << output << std::endl;
        logFile.close();
    }
    else {
        std::cerr << "Unable to open log file for writing." << std::endl;
    }

}

void Process::incrementInstructionPointer() {
    instructionPointer++;
    std::cout << name << ": instructionPointer is now " << instructionPointer << "\n";
}

//------------------DEBUG------------------//
//for logging only
void Process::show_log() const {
    std::cout << "\n--- Log for Process: " << name << " ---\n";
    for (const auto& entry : log) {
        std::cout << entry << std::endl;
    }
    std::cout << "--- End of Log ---\n";
}

//run the print command in the process
void Process::run_print() {
    if (instructionPointer < instructions.size()) {
        std::string output = instructions[instructionPointer];
        std::cout << output << std::endl;
        log.push_back(output);
        instructionPointer++;
    }
    else {
        std::cout << "Process " << name << " has finished all print commands." << std::endl;
    }
}

//------------------GETTERS/SETTERS------------------//

std::string Process::getName() const {
    return name;
}

std::string Process::getCurrentInstruction() const {
    if (instructionPointer < instructions.size()) {
        return instructions[instructionPointer];
    }
    return "No more instructions.";
}

int Process::getCurrentLine() const {
    return instructionPointer;
}

int Process::getTotalLines() const {
    return totalLines;
}

void Process::markFinished() {
    if (!finished) {
        finished = true;
        setCompletionTimestamp();
    }
}

bool Process::isFinished() {
    return finished;
}
