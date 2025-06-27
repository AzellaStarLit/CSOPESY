#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include "kpThread.h"
#include "sharedState.h"

void startKpThread(bool& running) {

	while (running) {
		if (_kbhit()) {
			char ch = _getch();

			std::lock_guard<std::mutex> lock(screenMutex);

			if (ch == '\r') {
				sharedCommandHistory.push_back(sharedInputBuffer);
				sharedInputBuffer.clear();
			}
			else if (ch == '\b')
			{
				if (!sharedInputBuffer.empty()) sharedInputBuffer.pop_back();
			}
			else {
				sharedInputBuffer += ch;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(polling_rate));
		}
	}
}