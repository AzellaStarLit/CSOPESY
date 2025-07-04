#include "Console.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib> // for system()
#include "marqueeConsole.h"

//------------------CONSTRUCTOR------------------//
Console::Console()
    : name("default"), screenProcess(nullptr){}

Console::Console(const std::string& name, Process* process)
    : name(name), screenProcess(process) {}

//------------------SCREEN MANAGEMENT------------------//

bool Console::isAttachedToProcess() const {
    return screenProcess != nullptr;
}

//choose what screen to draw
void Console::draw() {
    if (isAttachedToProcess()) {
        draw_process_screen();
    }
    else {
        draw_marquee_screen();
    }
}

//------------------SCREENS------------------//
void Console::draw_process_screen() {
    std::cout << "\033[36m--- Screen: " << name << " ---\033[0m\n";
    std::cout << "Process Name: " << screenProcess->getName() << "\n";
    std::cout << "Current Instruction: " << screenProcess->getCurrentInstruction() << "\n";
	std::cout << "Total Lines: " << screenProcess->getCurrentLine() << "/" << screenProcess->getTotalLines() << "\n";
    std::cout << "Created At: " << screenProcess->getCreationTimestamp() << "\n";
    std::cout << "Type 'exit' to return to the main menu.\n";


    std::string input;

    //there will be another loop in the process screen that waits for user input [exit, log, print, other commands within a process]
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
        else if (input == "process-smi") {  // process-smi logic
            std::cout << "\n\033[35m[Process SMI]\033[0m\n";
            std::cout << "Process Name: " << screenProcess->getName() << "\n";
            //std::cout << "ID: " << screenProcess->getId() << "\n";
            std::cout << "Logs:\n";

            for (const std::string& logEntry : screenProcess->get_log()) {
                std::cout << logEntry << "\n";
            }

            std::cout << "\nCurrent Instruction Line: " << screenProcess->getCurrentLine() << "\n";
            std::cout << "Lines of Code: " << screenProcess->getTotalLines() << "\n";

            if (screenProcess->isFinished()) {
                std::cout << "\n\033[32mFinished!\033[0m\n";
            }
        }
        else {
            std::cout << "Unknown command. Try 'print' or 'log'.\n";
        }
    }
}

void Console::draw_marquee_screen() {
    MarqueeConsole marquee(name);
    marquee.draw(); 
}