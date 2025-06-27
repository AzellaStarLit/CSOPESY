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
#include <thread>
#include <algorithm>
#include <random>

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
    int count = 10; //for testing only, this should come from random number based on config range

    for (int i = 0; i < count; ++i) {
        add_instruction("print: Hello World! from " + name + " [Line " + std::to_string(i + 1) + "]");
        totalLines = instructions.size();
    }
}

//this will generate random instructions from a predefined set
std::string Process::generate_rand_instruction() {
    static const std::string instructions[] = {
        "PRINT(\"\")",
        "PRINT(\"Processing...\")",
        "PRINT(\"We love CSOPESY <3\")",
    };

    //TODO: EDIT TO READ CONFIGURATION FILE FOR RANGE
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(instructions) - 1);

    return instructions[dis(gen)];
}


void Process::load_instructions(const std::vector<std::string>& instrs) {
    // Clear existing instructions if needed
    instructions.clear();

    // Load new instructions
    instructions = instrs;
    totalLines = static_cast<uint32_t>(instructions.size());
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
    else if (command == "SLEEP") {
        execute_sleep(argument);
        instructionPointer++;
        if (instructionPointer >= totalLines) markFinished();
    }
    else if (command == "DECLARE") {
        execute_declare(argument);
        instructionPointer++;
        if (instructionPointer >= totalLines) markFinished();
    }
    else if (command == "ADD") {
        execute_add(argument);
        instructionPointer++;
        if (instructionPointer >= totalLines) markFinished();
    }
    else if (command == "SUBTRACT") {
        execute_subtract(argument);
        instructionPointer++;
        if (instructionPointer >= totalLines) markFinished();
    }
    else if (command == "SLEEP") {
        execute_sleep(argument);
        instructionPointer++;
        if (instructionPointer >= totalLines) markFinished();
    } else if (command == "FOR") {
        execute_for(argument, coreId, 1);
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

    if (msg.empty()) {
        printMessage = "\"Hello world from " + name + "!\"";
    }
    else {
        printMessage = msg + " from " + name;
    }

    //TODO: ADD TO DEAL WITH VARIABLES
}

void Process::execute_declare(const std::string& args) {
    size_t comma = args.find(',');
    if (comma == std::string::npos) {
        std::cerr << "Invalid DECLARE format: " << args << "\n";
        return;
    }

    std::string var = args.substr(0, comma);
    std::string valStr = args.substr(comma + 1);
    
    try {
        uint32_t value = std::stoul(valStr);
        if (value > 65535) value = 65535; 
        symbolTable[var] = static_cast<uint16_t>(value);
    } catch (...) {
        std::cerr << "Invalid value in DECLARE: " << args << "\n";
    }
}

void Process::execute_add(const std::string& args) {
    std::istringstream ss(args);
    std::string var1, var2, var3;
    getline(ss, var1, ',');
    getline(ss, var2, ',');
    getline(ss, var3, ',');

    auto getValue = [this](const std::string& token) -> uint16_t {
        if (symbolTable.find(token) != symbolTable.end()) return symbolTable[token];
        try {
            return static_cast<uint16_t>(std::stoul(token));
        } catch (...) {
            symbolTable[token] = 0; 
            return 0;
        }
    };

    uint32_t result = static_cast<uint32_t>(getValue(var2)) + static_cast<uint32_t>(getValue(var3));
    if (result > 65535) result = 65535;

    symbolTable[var1] = static_cast<uint16_t>(result);
}

void Process::execute_subtract(const std::string& args) {
    std::istringstream ss(args);
    std::string var1, var2, var3;
    getline(ss, var1, ',');
    getline(ss, var2, ',');
    getline(ss, var3, ',');

    auto getValue = [this](const std::string& token) -> uint16_t {
        if (symbolTable.find(token) != symbolTable.end()) return symbolTable[token];
        try {
            return static_cast<uint16_t>(std::stoul(token));
        } catch (...) {
            symbolTable[token] = 0; 
            return 0;
        }
    };

    int result = static_cast<int>(getValue(var2)) - static_cast<int>(getValue(var3));
    if (result < 0) result = 0;

    symbolTable[var1] = static_cast<uint16_t>(result);
}

void Process::execute_sleep(const std::string& msString) {
    try {
        int ms = std::stoi(msString);
        std::cout << name << ": sleeping for " << ms << " ms\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    catch (const std::exception& e) {
        std::cerr << "Invalid sleep duration: " << msString << "\n";
    }
}

void Process::execute_for(const std::string& args, int coreId, int nestingLevel) {
    if (nestingLevel > 3) {
        std::cerr << "Exceeded maximum FOR loop nesting level.\n";
        return;
    }

    // Find [ ... ] and repeat count
    size_t openBracket = args.find('[');
    size_t closeBracket = args.find(']', openBracket);

    if (openBracket == std::string::npos || closeBracket == std::string::npos) {
        std::cerr << "Invalid FOR loop format: missing brackets.\n";
        return;
    }

    std::string instructionBlock = args.substr(openBracket + 1, closeBracket - openBracket - 1);
    std::string repeatStr = args.substr(closeBracket + 2); // skip ", "

    int repeatCount = 0;
    try {
        repeatCount = std::stoi(repeatStr);
    } catch (...) {
        std::cerr << "Invalid repeat count in FOR: " << repeatStr << "\n";
        return;
    }

    // Split instructions by `;` or `,` or detect nested FOR
    std::vector<std::string> innerInstructions;
    std::string current;
    int bracketDepth = 0;

    for (char c : instructionBlock) {
        if (c == '[') bracketDepth++;
        if (c == ']') bracketDepth--;

        if ((c == ',' || c == ';') && bracketDepth == 0) {
            if (!current.empty()) {
                innerInstructions.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) innerInstructions.push_back(current);

    // Repeat and execute
    for (int i = 0; i < repeatCount; ++i) {
        for (const std::string& instr : innerInstructions) {
            std::string trimmed = instr;
            trimmed.erase(0, trimmed.find_first_not_of(" \t")); // trim leading spaces

            if (trimmed.find("FOR") == 0) {
                size_t parenStart = trimmed.find('(');
                size_t parenEnd = trimmed.rfind(')');
                if (parenStart != std::string::npos && parenEnd != std::string::npos) {
                    std::string forArgs = trimmed.substr(parenStart + 1, parenEnd - parenStart - 1);
                    execute_for(forArgs, coreId, nestingLevel + 1);
                }
            } else {
                execute_instruction(trimmed, coreId);
            }
        }
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

void Process::show_symbol_table() const {
    std::cout << "\n--- Symbol Table for Process: " << name << " ---\n";
    for (const auto& [key, value] : symbolTable) {
        std::cout << key << " = " << value << "\n";
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
