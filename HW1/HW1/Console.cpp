#include "Console.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdlib> // for system()

void Console::setTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%m/%d/%Y, %I:%M:%S %p");
    creationTimestamp = oss.str();
}

Console::Console()
    : name("default"), currentLine(0), totalLines(0), screenProcess(nullptr) {
    setTimestamp();
}

Console::Console(const std::string& name, Process* screenProcess) 
    : name(name), currentLine(0), totalLines(0), screenProcess(screenProcess) {
    setTimestamp();
}

void Console::draw() {
    std::cout << "\033[36m--- Screen: " << name << " ---\033[0m\n";
    std::cout << "Process Name: " << screenProcess->getName() << "\n";
    std::cout << "Current Instruction: " << screenProcess->getCurrentInstruction() << "\n";
	std::cout << "Total Lines: " << screenProcess->getCurrentLine() << "/" << screenProcess->getTotalLines() << "\n";
    std::cout << "Created At: " << creationTimestamp << "\n";
    std::cout << "Type 'exit' to return to the main menu.\n";

    std::string input;

    while (true) {
        std::cout << "[" << name << "]> ";
        std::getline(std::cin, input);
        if (input == "exit") {
            system("cls");
            break;
        }
        else if (input == "print") {
            if (screenProcess) screenProcess->run_print();
        }
        else if (input == "log") {
            if (screenProcess) screenProcess->show_log();
        }
        else {
            std::cout << "Unknown command. Try 'print' or 'log'.\n";
        }
    }
}