#pragma once
#define CONSOLE_H

#include <string>
#include "Process.h"

class Console {
private:
    std::string name;
    int currentLine;
    int totalLines;
    std::string creationTimestamp;
    Process* screenProcess;

    void setTimestamp();

public:
    Console();
    Console(const std::string& name, Process* screenProcess);
    void draw();
};

