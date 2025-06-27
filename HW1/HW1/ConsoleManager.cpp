#include "ConsoleManager.h"
#include "ProcessManager.h"
#include <iostream>
#include <mutex>

extern ProcessManager processManager;

//TODO: I think this should take the number of instructions 
/*
void ConsoleManager::create_screen_with_process(const std::string& name) {
    if (screens.find(name) != screens.end()) {
        std::cout << "Screen '" << name << "' already exists. Use 'screen -r' to resume.\n";
        return;
   }

    //add the process to the list
    processManager.create_process(name);
    Process* process = processManager.get_process(name); //retrieve the new process

    screens[name] = Console(name, process); //create a console for the process
    screens[name].draw(); //enter screen after creation

}
*/

void ConsoleManager::create_screen_with_process(const std::string& name) {
    if (screens.find(name) == screens.end()) {
        processes[name] = Process(name);
        screens[name] = Console(name, &processes[name]);
        std::cout << "Screen + Process '" << name << "' created.\n";
        screens[name].draw();
    }
    else {
        std::cout << "Screen '" << name << "' already exists. Attaching...\n";
    }
}

void ConsoleManager::attach_screen(const std::string& name, Process* process) {
    if (screens.find(name) != screens.end()) {
        std::cout << "Screen '" << name << "' already exists. Use 'screen -r' to resume.\n";
        return;
    }

    screens[name] = Console(name, process); //create and attach console to process
    std::cout << "Attached screen to process '" << name << "'.\n";
}


void ConsoleManager::resume_screen(const std::string& name) {
    auto it = screens.find(name);
    if (it != screens.end()) {
        std::cout << "Resuming screen '" << name << "'...\n";
        it->second.draw();
    }
    else {
        std::cout << "No such screen named '" << name << "'.\n";
    }
}

bool ConsoleManager::has_screen(const std::string& name) {
    return screens.find(name) != screens.end();
}

std::mutex& ConsoleManager::getMutex() {
    return screenMutex;
}