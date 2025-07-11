#pragma once

#include <unordered_map>
#include <string>
#include "Console.h"
#include "Process.h"
#include <mutex>

class ConsoleManager {
private:
    mutable std::mutex screenMutex;
    std::unordered_map<std::string, Console> screens;
    std::unordered_map<std::string, Process> processes;

public:
    //when you want to create a process with a screen 
    void create_screen_with_process(const std::string& name);

    bool has_screen(const std::string& name); //check if a screen is already attached
    void attach_screen(const std::string& name, Process* process); //attach console to an existing Process

    //for screen -r
    void resume_screen(const std::string& name);
    std::mutex& getMutex();
};