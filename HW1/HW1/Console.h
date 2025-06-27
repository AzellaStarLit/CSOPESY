#pragma once

#include <string>
#include "Process.h"

class Console {
private:
    std::string name;
    Process* screenProcess = nullptr;

    void draw_process_screen(); //to attach a screen for each process
    void draw_marquee_screen(); //because we need the marquee implementation as well

public:
    //constructors
    Console(); //default
    Console(const std::string& name, Process* screenProcess); //when you have a name for a process
    bool isAttachedToProcess() const; 

    //chooses the appropriate screen [for process or marquee or smth else]
    virtual void draw(); 
};

