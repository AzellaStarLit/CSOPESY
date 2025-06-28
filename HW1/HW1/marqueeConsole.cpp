#include "marqueeConsole.h"
#include <iostream>

MarqueeConsole::MarqueeConsole(const std::string& name) :
	Console(name, nullptr), marqueeThread("Hello World in marquee!!"), running(true) {}

//run marquee thread
void MarqueeConsole::runMarquee() {
	marqueeThread.start(cursorMutex);
}


void MarqueeConsole::cmd() {
	kpThread = std::thread(startKpThread, std::ref(running));
}

void MarqueeConsole::draw() {

	runMarquee();

	cmd();

	while (marqueeThread.isRunning()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	//stop keyboard polling when user exits
	running = false;

	if (kpThread.joinable())
		kpThread.join();
}