#pragma once
#include <string>
#include <mutex>

inline std::mutex screenMutex;
inline std::string sharedInputBuffer;
//inline std::string sharedLastCommand;
inline std::vector<std::string> sharedCommandHistory;
inline int refresh_rate = 33;
inline int polling_rate = 80;

/*
100ms [10 fps] - not smooth
50ms [20 fps] - has flicker, laggy, 8% of CPU
20ms [50fps] - smooth, 10% of CPU
33ms[30fps] - compromise, flicker, but smooth motion, 8% CPU
*/

/*
10ms - double enter
33ms - decent balance
60ms - noticeable lag
*/