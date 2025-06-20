#pragma once
#define UTILITIES_H
#include "Instructions.h"
#include <string>

void print_header();
void clear();
void initialize();
void scheduler_start();
void scheduler_stop();
void report_util();
void clear();
void exit_program();

std::string generate_instruction(const Instructions& tmpl, const std::string& processName);
