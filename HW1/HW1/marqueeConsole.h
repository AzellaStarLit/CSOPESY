#pragma once

#include "console.h"
#include "marqueeThread.h"
#include "kpThread.h"
#include <mutex>

//subclass of Console
class MarqueeConsole : public Console {
private:
	bool running; //console status

	std::mutex cursorMutex;

	MarqueeThread marqueeThread; //create a thread for marquee
	std::thread kpThread;


public:
	MarqueeConsole(const std::string& name); //name of console and marquee text
	void draw() override;
	void runMarquee();
	void cmd();
};