#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <stdint.h>
#include <vector>

// void test(char* filename);
std::vector<uint8_t> readFileToUint8Vector(const char* filename);