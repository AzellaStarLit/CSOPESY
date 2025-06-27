#pragma once

#include <thread>
#include<string>
#include<mutex>

class MarqueeThread {
private:
	std::string text; //marquee text
	std::thread thread; //marquee thread
	bool running; //thread status

	void run(); //logic

	std::mutex* cursorMutex;

public:
	MarqueeThread(const std::string& text);
	~MarqueeThread();

	void start(std::mutex& sharedMutex); //start task
	void stop(); //stop task
	bool isRunning() const; //task status
};