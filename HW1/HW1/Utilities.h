#pragma once
#define UTILITIES_H
#include <vector>
#include <string>

void print_header();
void clear();
void initialize();
void scheduler_stop();
void report_util();
void clear();
void exit_program();
void vmstat();
void process_smi();
std::vector<int> getPowerOfTwoSizesInRange();
bool isPowerOfTwo(int n);
std::vector<std::string> generate_instructions();

