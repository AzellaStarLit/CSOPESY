#pragma once

#define CONSOLE_MANAGER_H

#include <unordered_map>
#include <string>
#include "Console.h"
#include "Process.h"

class ConsoleManager {
private:
    std::unordered_map<std::string, Console> screens;
    std::unordered_map<std::string, Process> processes;

public:
    void create_screen_with_process(const std::string& name);
    void resume_screen(const std::string& name);
    void attach_screen(const std::string& name, Process* process);
};