#include "Process.h"
#include <ctime>
#include <iomanip>
#include <sstream>

void Process::setTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%m/%d/%Y, %I:%M:%S %p");
    creationTimestamp = oss.str();
}

Process::Process()
    : name("default"), currentLine(0), totalLines(0), instructionPointer(0) {
    setTimestamp();
}

Process::Process(const std::string& name)
    : name(name), currentLine(0), totalLines(0), instructionPointer(0) {
    setTimestamp();
}

Process::Process(const std::string& name, int instructionCount)
    : name(name), currentLine(0), totalLines(instructionCount), instructionPointer(0) {
    setTimestamp();
}

void Process::run_print() {
    if (instructionPointer < instructions.size()) {
        std::string output = instructions[instructionPointer];
        std::cout << output << std::endl;
        log.push_back(output);
        instructionPointer++;
        currentLine++;
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
    totalLines = instructions.size();
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
    return currentLine;
}

int Process::getTotalLines() const {
    return totalLines;
}

void Process::generate_instructions() {
    int count = 10;

    for (int i = 0; i < count; ++i) {
        add_instruction("print: Hello from " + name + " [Line " + std::to_string(i + 1) + "]");
		totalLines = instructions.size();
    }
}