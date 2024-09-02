#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <stdint.h>
#include <vector>
#include <limits>

#include "../Packet/Packet.hpp"

// void test(char* filename);
std::vector<uint8_t> readFileToUint8Vector(const char* filename, PacketType type);

std::vector<std::string> split(std::string& s, const std::string& delimiter);