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
#include <filesystem>


//this is a list of instructions recognized by a process
std::unordered_map<std::string, std::function<void(const std::string&, int)>> instructionList;
int Process::global_pid_counter = 1; // Start PID from 1

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

//TODO: DETERMINE WHICH CONSTRUCTORS ARE USED AND REMOVE UNUSED ONES
Process::Process()
    : name("default"), instructionPointer(0), totalLines(0), memorySize(0),
    creationTimestamp(get_current_timestamp()), processId(global_pid_counter++) {
}

Process::Process(const std::string& name)
    : name(name), instructionPointer(0), totalLines(0), memorySize(0),
    creationTimestamp(get_current_timestamp()), processId(global_pid_counter++) {
}

Process::Process(const std::string& name, size_t memorySize, size_t frameSize, MemoryManager* memoryManager)
    : name(name), instructionPointer(0), totalLines(0), memorySize(memorySize),
    creationTimestamp(get_current_timestamp()), processId(global_pid_counter++),
    frameSize(frameSize), memoryManager(memoryManager), status(ProcessStatus::New) {

    //std::cout << "Process created with name: " << name << " and memory size: " << memorySize << std::endl;
    //initializeBackingStore();
}


//------------------INSTRUCTIONS------------------//

void Process::add_instruction(const std::string& instr) {
    instructions.push_back(instr);
    totalLines = instructions.size();
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
    size_t parenEnd = instruction.rfind(')');

    if (parenStart == std::string::npos || parenEnd == std::string::npos) {
        std::cout << "Invalid instruction format: " << instruction << std::endl;
        return;
    }

    std::string command = instruction.substr(0, parenStart);
    std::string argument = instruction.substr(parenStart + 1, parenEnd - parenStart - 1);

  
    if (command == "PRINT") { 
        execute_print(argument, coreId);
    }
    else if (command == "SLEEP") { 
        execute_sleep(argument);
    }
    else if (command == "DECLARE") { 
        execute_declare(argument);
    }
    else if (command == "ADD") { 
        execute_add(argument);  
    }
    else if (command == "SUBTRACT") { 
        execute_subtract(argument);
    }
    else if (command == "SLEEP") { 
        execute_sleep(argument); 
    }
    else if (command == "FOR") { 
        execute_for(argument, coreId, 1);
    }
    else if (command == "READ") {
        execute_read(argument);
    }
    else if (command == "WRITE") {
        execute_write(argument);
    }
    else {
        std::cout << "Unknown command: " << command << std::endl;
    }

	instructionPointer++; // Increment instruction pointer after execution
    if (instructionPointer >= totalLines) {
        markFinished(); // Mark process as finished if all instructions are executed
    }
}

// this is the execution logic for the READ command
/*
void Process::execute_read(const std::string& args) {

	memoryManager->resetPageFaultFlag(); // Reset page fault flag at the start of READ
    std::string timestamp = get_current_timestamp();

    size_t comma = args.find(',');
    if (comma == std::string::npos) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " READ: Invalid argument format: " + args);
        return;
    }

    std::string var = args.substr(0, comma);
    std::string addrStr = args.substr(comma + 1);

    // Trim whitespace
    var.erase(0, var.find_first_not_of(" \t"));
    var.erase(var.find_last_not_of(" \t") + 1);
    addrStr.erase(0, addrStr.find_first_not_of(" \t"));
    addrStr.erase(addrStr.find_last_not_of(" \t") + 1);

    size_t address = 0;
    try {
        address = std::stoul(addrStr, nullptr, 16); // Parse hex
    }
    catch (...) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " READ: Invalid hex address format: " + addrStr);
        return;
    }

    uint16_t value = 0;
    bool success = memoryManager->readUInt16(processId, static_cast<uint32_t>(address), value);

    if (memoryManager->wasPageFault()) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " PAGE FAULT: Suspended READ from " + addrStr + " will retry next cycle.");
        status = ProcessStatus::Waiting;  // New custom state
		//instructionPointer--; // Decrement instruction pointer to retry
        return; // Exit early so we can retry later
    }

    if (success) {
        symbolTable[var] = value;
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " READ: " + var + " = " + std::to_string(value) + " from address " + addrStr);
    }
    else {
		// Not a recoverable error, trigger termination for memory access violation
        symbolTable[var] = 0;
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " READ FAILED: Could not read from memory[" + addrStr + "]");
        markFinished();
        setCompletionTimestamp();
        status = ProcessStatus::Terminated;
    }
}
*/


// this is the execution logic for the WRITE command
/*
void Process::execute_write(const std::string& args) {
    memoryManager->resetPageFaultFlag(); // Reset page fault flag at the start of READ
    std::string timestamp = get_current_timestamp();

    size_t comma = args.find(',');
    if (comma == std::string::npos) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " WRITE: Invalid argument format. Expected: <hex_address>, <value>. Got: " + args);
        return;
    }

    std::string addrStr = args.substr(0, comma);
    std::string valueStr = args.substr(comma + 1);

    // Trim whitespace
    addrStr.erase(0, addrStr.find_first_not_of(" \t"));
    addrStr.erase(addrStr.find_last_not_of(" \t") + 1);
    valueStr.erase(0, valueStr.find_first_not_of(" \t"));
    valueStr.erase(valueStr.find_last_not_of(" \t") + 1);

    size_t address = 0;
    uint16_t value = 0;

    try {
        address = std::stoul(addrStr, nullptr, 16); // hex address
    }
    catch (...) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " WRITE: Invalid address format. Must be hexadecimal.");
        return;
    }

    try {
        value = static_cast<uint16_t>(std::stoi(valueStr));
    }
    catch (...) {
        if (symbolTable.find(valueStr) != symbolTable.end()) {
            value = static_cast<uint16_t>(symbolTable[valueStr]);
        }
        else {
            log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
                " WRITE: Invalid value. Not a valid integer or known variable: " + valueStr);
            return;
        }
    }

    if (value > 65535) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " WRITE: Value out of range (0-65535): " + std::to_string(value));
        return;
    }

    if (memoryManager->wasPageFault()) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " PAGE FAULT: Suspended WRITE to " + addrStr + " will retry next cycle.");
        status = ProcessStatus::Waiting; 
        //instructionPointer--;
        return;
    }

    bool success = memoryManager->writeUInt16(processId, static_cast<uint32_t>(address), value);

    if (success) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " WRITE: memory[" + addrStr + "] = " + std::to_string(value));
    }
    else {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " WRITE FAILED: Could not write to memory[" + addrStr + "]");
        markFinished();
        setCompletionTimestamp();
        status = ProcessStatus::Terminated;
    }
}
*/

void Process::execute_write(const std::string& args) {
    std::string timestamp = get_current_timestamp();
    std::stringstream ss(args);
    std::string addr_str, val_str;

    std::getline(ss, addr_str, ',');
    std::getline(ss, val_str);

    try {
        // Parse address as hex (base 16)
        size_t addr = std::stoul(addr_str, nullptr, 16);

        // Parse value as decimal (you can change base if you want)
        uint16_t value = static_cast<uint16_t>(std::stoul(val_str, nullptr, 10));

        if (!memoryManager) {
            log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
                " WRITE: No memory manager assigned.");
            return;
        }

        // Trigger memory manager to ensure page is present
        size_t pageNum = addr / frameSize;
        PageTableEntry& entry = getPageEntry(pageNum);

        if (!entry.valid) {
            memoryManager->handlePageFault(this, pageNum);
			status = ProcessStatus::Waiting;
        }

        entry.dirty = true;

        // Write to memory (simplified simulation)
        bool writeSuccess = memoryManager->writeUInt16(processId, static_cast<uint32_t>(addr), value);

        if (!writeSuccess) {
            log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
                " WRITE FAILED: Could not write value to address 0x" + addr_str);
            markFinished();
            setCompletionTimestamp();
            status = ProcessStatus::Terminated;
            return;
        }

        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " WRITE: Value " + std::to_string(value) + " written to address " + addr_str);

    }
    catch (const std::exception& e) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " WRITE: Invalid arguments: " + args);
    }
}

void Process::execute_read(const std::string& args) {
    std::string timestamp = get_current_timestamp();
    std::stringstream ss(args);
    std::string var, addr_str;

    std::getline(ss, var, ',');
    std::getline(ss, addr_str);

    // Trim whitespace helper lambda
    auto trim = [](std::string& s) {
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        };
    trim(var);
    trim(addr_str);

    try {
        size_t addr = std::stoul(addr_str, nullptr, 16); // parse hex address

        if (!memoryManager) {
            log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
                " READ: No memory manager assigned.");
            return;
        }

        size_t pageNum = addr / frameSize;
        PageTableEntry& entry = getPageEntry(pageNum);

        if (!entry.valid) {
            memoryManager->handlePageFault(this, pageNum);
            // suspend process to retry next cycle
            log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
                " PAGE FAULT: Suspended READ from " + addr_str + " will retry next cycle.");
            status = ProcessStatus::Waiting;
            return;
        }

        uint16_t value = 0;
        bool success = memoryManager->readUInt16(processId, static_cast<uint32_t>(addr), value);

        if (success) {
            symbolTable[var] = value;
            log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
                " READ: " + var + " = " + std::to_string(value) + " from address " + addr_str);
        }
        else {
            // unrecoverable read error
            symbolTable[var] = 0;
            log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
                " READ FAILED: Could not read from memory[" + addr_str + "]");
            markFinished();
            setCompletionTimestamp();
            status = ProcessStatus::Terminated;
        }
    }
    catch (...) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " READ: Invalid arguments: " + args);
    }
}



//this is the execution logic of the PRINT command
void Process::execute_print(const std::string& msg, int coreId) {
    
    //start timestamp for each instruction
    std::string timestamp = get_current_timestamp();

    std::string printMessage;

    if (msg.empty()) {
        printMessage = "\"Hello world from " + name + "!\"";
    }
    else {
        std::string processedMsg = msg;

        // Look for a "+" indicating string + variable (basic pattern only)
        size_t plusPos = msg.find('+');
        if (plusPos != std::string::npos) {
            std::string strPart = msg.substr(0, plusPos);
            std::string varPart = msg.substr(plusPos + 1);

            // Trim whitespace from varPart
            varPart.erase(0, varPart.find_first_not_of(" \t"));
            varPart.erase(varPart.find_last_not_of(" \t") + 1);

            // Remove quotes from strPart if they exist
            if (!strPart.empty() && strPart.front() == '\"' && strPart.back() == '\"') {
                strPart = strPart.substr(1, strPart.size() - 2);
            }

            // Look up variable in symbolTable
            std::string varValue = "undefined";
            auto it = symbolTable.find(varPart);
            if (it != symbolTable.end()) {
                varValue = std::to_string(it->second);
            }

            printMessage = varValue + " from " + name;
        }
        else {
            printMessage = msg + " from " + name;
        }
    }

    std::string logEntry = "[" + timestamp + "] Core " + std::to_string(getCurrentCore()) + " PRINT: " + printMessage;
    log.push_back(logEntry);
}

void Process::execute_declare(const std::string& args) {
    size_t comma = args.find(',');
    std::string timestamp = get_current_timestamp();

    if (comma == std::string::npos) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " DECLARE: Invalid format: " + args);
        return;
    }

    std::string var = args.substr(0, comma);
    std::string valStr = args.substr(comma + 1);

    // Trim whitespace
    auto trim = [](std::string& s) {
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        };

    trim(var);
    trim(valStr);

    if (symbolTable.size() >= 32) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " DECLARE: Symbol table full, cannot declare '" + var + "'");
        return;
    }

    try {
        uint32_t value = std::stoul(valStr);
        if (value > 65535) value = 65535;
        uint16_t val16 = static_cast<uint16_t>(value);

        // Store in symbol table first
        symbolTable[var] = val16;

        // Ensure page is resident and mark dirty (handle page faults here)
        touchForWriteVar(var);

        // Get virtual address for the variable
        size_t virtualAddr = getVirtualAddressForVar(var);

        // Write value into memory at virtual address
        bool writeSuccess = memoryManager->writeUInt16(processId, static_cast<uint32_t>(virtualAddr), val16);

        if (!writeSuccess) {
            log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
                " DECLARE: Failed to write variable '" + var + "' to memory at virtual address " + std::to_string(virtualAddr));
            return;
        }

        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " DECLARE: " + var + " = " + std::to_string(val16));
    }
    catch (...) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) +
            " DECLARE: Invalid value in '" + args + "'");
    }
}



void Process::execute_add(const std::string& args) {
    std::string timestamp = get_current_timestamp();
    std::istringstream ss(args);
    std::string var1, var2, var3;

    auto trim = [](std::string s) -> std::string {
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        return s;
    };

    getline(ss, var1, ','); var1 = trim(var1);
    getline(ss, var2, ','); var2 = trim(var2);
    getline(ss, var3, ','); var3 = trim(var3);

    size_t virtualAddr = getVirtualAddressForVar(args);
    size_t pageNum = virtualAddr / frameSize;

    if (!pageTable[pageNum].valid) {
        memoryManager->handlePageFault(this, pageNum);
    }

    auto getValue = [this](const std::string& token) -> uint16_t {
        if (symbolTable.find(token) != symbolTable.end()) return symbolTable[token];
        try {
            return static_cast<uint16_t>(std::stoul(token));
        } catch (...) {
            symbolTable[token] = 0; 
            return 0;
        }
    };

    ensureResidentVar(var2);
    ensureResidentVar(var3);

    uint32_t result = static_cast<uint32_t>(getValue(var2)) + static_cast<uint32_t>(getValue(var3));
    if (result > 65535) result = 65535;

    touchForWriteVar(var1);
    symbolTable[var1] = static_cast<uint16_t>(result);
    log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) + " ADD: " + var1 + " = " + std::to_string(symbolTable[var1]));
}

void Process::execute_subtract(const std::string& args) {
    std::string timestamp = get_current_timestamp();
    std::istringstream ss(args);
    std::string var1, var2, var3;

    auto trim = [](std::string s) -> std::string {
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        return s;
    };

    getline(ss, var1, ','); var1 = trim(var1);
    getline(ss, var2, ','); var2 = trim(var2);
    getline(ss, var3, ','); var3 = trim(var3);

    size_t virtualAddr = getVirtualAddressForVar(args);
    size_t pageNum = virtualAddr / frameSize;

    if (!pageTable[pageNum].valid) {
        memoryManager->handlePageFault(this, pageNum);
    }

    auto getValue = [this](const std::string& token) -> uint16_t {
        if (symbolTable.find(token) != symbolTable.end()) return symbolTable[token];
        try {
            return static_cast<uint16_t>(std::stoul(token));
        } catch (...) {
            symbolTable[token] = 0; 
            return 0;
        }
    };

    ensureResidentVar(var2);
    ensureResidentVar(var3);

    int result = static_cast<int>(getValue(var2)) - static_cast<int>(getValue(var3));
    if (result < 0) result = 0;

    touchForWriteVar(var1);
    symbolTable[var1] = static_cast<uint16_t>(result);
    log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) + " SUBTRACT: " + var1 + " = " + std::to_string(symbolTable[var1]));
}

void Process::execute_sleep(const std::string& msString) {
    std::string timestamp = get_current_timestamp();
    try {
        int ms = std::stoi(msString);
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) + " SLEEP: Sleeping for " + std::to_string(ms) + " ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    catch (const std::exception& e) {
        //std::string err = "Invalid sleep duration: " + msString;
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) + " SLEEP: Invalid duration : " + msString);
    }
}

void Process::execute_for(const std::string& args, int coreId, int nestingLevel) {
    std::string timestamp = get_current_timestamp();

    if (nestingLevel > 3) {
        //std::cerr << "Exceeded maximum FOR loop nesting level.\n";
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) + "FOR: Nesting limit exceeded.");
        return;
    }

    std::string trimmedArgs = args;
    trimmedArgs.erase(0, trimmedArgs.find_first_not_of(" \t\n"));
    trimmedArgs.erase(trimmedArgs.find_last_not_of(" \t\n") + 1);

    if (trimmedArgs.empty()) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) + " FOR: Empty FOR arguments.");
        return;
    }

    // Find [ ... ] and repeat count
    size_t openBracket = trimmedArgs.find('[');
    size_t closeBracket = trimmedArgs.find(']', openBracket);

    static bool alreadyErrored = false;

    if (openBracket == std::string::npos || closeBracket == std::string::npos || closeBracket <= openBracket) {
        //std::cerr << "Invalid FOR loop format: missing brackets.\n";
        if (!alreadyErrored) {
            log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) + " FOR: Invalid format - missing brackets.");
            alreadyErrored = true;
        }
        return;
    }

    std::string instructionBlock = trimmedArgs.substr(openBracket + 1, closeBracket - openBracket - 1);
    size_t commaAfterBracket = trimmedArgs.find(',', closeBracket + 1);
    if (commaAfterBracket == std::string::npos) {
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) + " FOR: Invalid format - missing repeat count.");
        return;
    }
    std::string repeatStr = trimmedArgs.substr(commaAfterBracket + 1);
    repeatStr.erase(0, repeatStr.find_first_not_of(" \t"));

    int repeatCount = 0;
    try {
        repeatCount = std::stoi(repeatStr);
    } catch (...) {
        //std::cerr << "Invalid repeat count in FOR: " << repeatStr << "\n";
        log.push_back("[" + timestamp + "] Core " + std::to_string(getCurrentCore()) + " FOR: Invalid repeat count '" + repeatStr + "'");
        return;
    }

    // Split instructions by `;` or `,` or detect nested FOR
    std::vector<std::string> innerInstructions;
    std::string current;
    int bracketDepth = 0;

    for (char c : instructionBlock) {
        if (c == '[') bracketDepth++;
        if (c == ']') bracketDepth--;

        if ((c == ';') && bracketDepth == 0) {
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

//------------------BACKING STORE------------------//
/*
void Process::initializeBackingStore() {
    std::filesystem::create_directory("csopesy-backing-store");

    backingStorePath = "csopesy-backing-store/p" + std::to_string(processId) + ".txt";

    std::ofstream out(backingStorePath, std::ios::binary | std::ios::trunc);
    size_t totalPages = memorySize / frameSize; //pass as argument

    for (size_t i = 0; i < totalPages; ++i) {
        std::string blankPage(1024, '\0'); // 1 page = 1024 bytes default
        out.write(blankPage.c_str(), 1024);
    }
    out.close();
}*/
/*
const std::string& Process::getBackingStorePath() const {
    return backingStorePath;
}
*/

bool Process::hasResidentPage() const {
    for (const auto& kv : pageTable) {
        if (kv.second.valid) return true;
    }
    return false;
}


//------------------PAGE TABLE------------------//

Process::PageTableEntry& Process::getPageEntry(size_t pageNum) {
    return pageTable[pageNum];
}

void Process::ensureResidentVar(const std::string& varName) {
    if (!memoryManager || frameSize == 0 || memorySize == 0) return;
    size_t vaddr = getVirtualAddressForVar(varName);
    size_t pageNum = vaddr / frameSize;

    auto& e = getPageEntry(pageNum);
    if (!e.valid) {
        memoryManager->handlePageFault(this, pageNum); // will bump page-ins
    }
}

void Process::touchForWriteVar(const std::string& varName) {
    if (!memoryManager || frameSize == 0 || memorySize == 0) return;
    size_t vaddr = getVirtualAddressForVar(varName);
    size_t pageNum = vaddr / frameSize;

    auto& e = getPageEntry(pageNum);
    if (!e.valid) {
        memoryManager->handlePageFault(this, pageNum); // may bump page-ins
    }
    e.dirty = true; 
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
		status = ProcessStatus::Finished;
    }
}

bool Process::isFinished() {
    return finished;
}

const std::vector<std::string>& Process::get_log() const {
    return log;
}

std::string Process::get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%m/%d/%Y, %I:%M:%S %p");
    return oss.str();
}

//FOR SIMULATION OF VIRTUAL MEMORY ACCESS IN INSTRUCTION EXECUTION
// Get virtual address for a variable within the symbol table segment (64 bytes, 32 vars max)
size_t Process::getVirtualAddressForVar(const std::string& varName) const {
    static constexpr size_t symbolTableBase = 0;  // Adjust if symbol table starts elsewhere
    static constexpr size_t maxVars = 32;
    static constexpr size_t varSize = 2;  // 2 bytes per variable (uint16)

    std::hash<std::string> hasher;
    size_t varIndex = hasher(varName) % maxVars;
    size_t virtualAddress = symbolTableBase + (varIndex * varSize);
    return virtualAddress;
}


// Process.cpp

ProcessStatus Process::getStatus() const {
    return status;
}

void Process::setStatus(ProcessStatus newStatus) {
    status = newStatus;
}

std::string Process::getStatusString() const {
    switch (status) {
        case ProcessStatus::New:        return "New";
        case ProcessStatus::Running:    return "Running";
        case ProcessStatus::Sleeping:   return "Sleeping";
        case ProcessStatus::Waiting:    return "Waiting";
        case ProcessStatus::Ready:      return "Ready";
        case ProcessStatus::Finished:   return "Finished";
        case ProcessStatus::Terminated: return "Terminated";
        default:                         return "Unknown";
    }
}