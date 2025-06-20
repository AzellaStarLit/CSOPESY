#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct Instructions {
	std::string name;
	std::string format;
	std::vector<std::string> operands;
};

extern const std::vector<Instructions> instructionTemplates;