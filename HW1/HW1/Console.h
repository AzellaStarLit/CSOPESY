#pragma once
#define CONSOLE_H

#include <string>

class Console {
private:
    std::string name;
    int currentLine;
    int totalLines;
    std::string creationTimestamp;

    void setTimestamp();

public:
    Console();
    Console(const std::string& name);
    void draw();
};

