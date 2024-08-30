#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <stdint.h>
#include <vector>
#include <limits>

// void test(char* filename);
std::vector<uint8_t> readFileToUint8Vector(const char* filename);

std::vector<std::string> split(std::string& s, const std::string& delimiter);