#include "marqueeThread.h"
#include <iostream>
#include <chrono>
#include <windows.h>
#include "sharedState.h"

MarqueeThread::MarqueeThread(const std::string& text) :
	text(text), running(false) {}

MarqueeThread::~MarqueeThread() {
	stop();

	if (thread.joinable()) {
		thread.join(); //always thread join
	}
}

//make this thread run this task
void MarqueeThread::start(std::mutex& sharedMutex) {
	this->cursorMutex = &sharedMutex;
	running = true;
	thread = std::thread(&MarqueeThread::run, this);
}

void MarqueeThread::stop() {
	running = false;
}

//thread status
bool MarqueeThread::isRunning() const {
	return running;
}

void moveCursor(int x, int y) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = {
		static_cast<SHORT>(x), static_cast<SHORT>(y)
	};
	SetConsoleCursorPosition(hConsole, pos);
}

void hideCursor() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE;  // hide cursor
	SetConsoleCursorInfo(hConsole, &cursorInfo);
}


void MarqueeThread::run() {
	const int width = 100;
	const int height = 25;
	int x = 1, dx = 1;
	int y = 5, dy = 1;

	const int top = 4;
	const int bottom = height - 4;
	const int left = 0;
	const int right = width - static_cast<int> (text.length());

	hideCursor();

	while (running) {

		{
			std::lock_guard<std::mutex> lock(screenMutex);

			//check for exit command
			if (!sharedCommandHistory.empty() && sharedCommandHistory.back() == "exit") {
				running = false;
				break;
			}

			system("cls");

			std::cout << "*******************\n";
			std::cout << "* Marquee Console *\n"; //header
			std::cout << "*******************";

			for (int row = 3; row < y; ++row) {
				std::cout << "\n";
			}

			std::string line(width, ' ');
			if (x + text.size() <= width) {
				line.replace(x, text.size(), text);
			}
			std::cout << line << "\n\n";

			int padding = bottom - (y + 1);

			for (int i = -1; i < padding; ++i) {
				std::cout << "\n";
			}

			std::cout << "Enter a command for MARQUEE CONSOLE: " << sharedInputBuffer;

			if (!sharedCommandHistory.empty()) {
				for (auto it = sharedCommandHistory.rbegin(); it != sharedCommandHistory.rend(); ++it) {
					std::cout << "\nCommand processed in MARQUEE CONSOLE: " << *it << "\n";
				}
			}

		}

		x += dx;
		y += dy;
		if (x <= left || x >= right) dx *= -1;
		if (y <= top || y >= bottom) dy *= -1;

		std::this_thread::sleep_for(std::chrono::milliseconds(refresh_rate));
	}
}