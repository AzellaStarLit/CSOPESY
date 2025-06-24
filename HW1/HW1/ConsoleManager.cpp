#include "ConsoleManager.h"
#include <iostream>
#include <mutex>

void ConsoleManager::create_screen_with_process(const std::string& name) {
    //std::lock_guard<std::mutex> lock(screenMutex);
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
    //std::lock_guard<std::mutex> lock(screenMutex);
    if (screens.find(name) == screens.end()) {
        processes[name] = *process; 
        screens[name] = Console(name, &processes[name]); 
        std::cout << "Screen + Process '" << name << "\n";
    }
    else {
        std::cout << "Screen '" << name << "' already exists. Attaching...\n";
    }
}


void ConsoleManager::resume_screen(const std::string& name) {
    //std::lock_guard<std::mutex> lock(screenMutex);
    auto it = screens.find(name);
    if (it != screens.end()) {
        std::cout << "Resuming screen '" << name << "'...\n";
        it->second.draw();
    }
    else {
        std::cout << "No such screen named '" << name << "'.\n";
    }
}

std::mutex& ConsoleManager::getMutex() { 
    return screenMutex; 
}