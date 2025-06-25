#include "Process.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <fstream>

int Process::nextId = 1;  // nextId process for process-smi
std::unordered_map<std::string, std::function<void(const std::string&)>> instructionList;

void Process::setTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%m/%d/%Y, %I:%M:%S %p");
    creationTimestamp = oss.str();
}

Process::Process()
    : name("default"), instructionPointer(0), totalLines(0), id(nextId++) {  // Updated constructor for nextId
    setTimestamp();
    instructionList["PRINT"] = [this](const std::string& msg) {
        execute_print(msg, -1); };
}

Process::Process(const std::string& name)
    : name(name), instructionPointer(0), totalLines(0), id(nextId++) { // Updated constructor for nextId
    setTimestamp();
	instructionList["PRINT"] = [this](const std::string& msg) { 
        execute_print(msg, -1); };
}

Process::Process(const std::string& name, int instructionCount)
    : name(name), instructionPointer(0), totalLines(instructionCount), id(nextId++) { // Updated constructor for nextId
    setTimestamp();
    instructionList["PRINT"] = [this](const std::string& msg) {
        execute_print(msg, -1); };
}

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

void Process::show_log() const {
    std::cout << "\n--- Log for Process: " << name << " ---\n";
    for (const auto& entry : log) {
        std::cout << entry << std::endl;
    }
    std::cout << "--- End of Log ---\n";
}

void Process::add_instruction(const std::string& instr) {
    instructions.push_back(instr);
    totalLines = static_cast<uint32_t>(instructions.size());
}

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


void Process::load_instructions(const std::vector<std::string>& instrs) {
    // Clear existing instructions if needed
    instructions.clear();

    // Load new instructions
    instructions = instrs;
    totalLines = static_cast<uint32_t>(instructions.size());
}

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
    }
    else {
        std::cout << "Unknown command: " << command << std::endl;
    }
}

void Process::execute_print(const std::string& msg, int coreId) {
	
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

    if (coreId != -1) {
        instructionPointer++;
    }

    //Compose string
    std::string output = "(" + timestamp + ") Core: " + std::to_string(coreId) + " " + printMessage;
   // std::cout << output << std::endl;
	
	//Append to log file
    std::ofstream logFile("logs/" + name + "_log.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << output << std::endl;
        logFile.close();
    } else {
        std::cerr << "Unable to open log file for writing." << std::endl;
	}

}

void Process::setCompletionTime() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%m/%d/%Y, %I:%M:%S %p");
    completionTimeStamp = oss.str();
}


// Newly added functions for getId and get_log

int Process::getId() const {
    return id;
}

const std::vector<std::string>& Process::get_log() const {
    return log;
}
