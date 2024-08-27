#include <iostream>
#include "Packet.hpp"

/**
 * Convertit un Packet en chaine d'octets
 */
std::vector<uint8_t> Packet::toBytes() {
    std::vector<uint8_t> bytes;
    bytes.push_back(packetType);

    for(int i = 3; i >= 0; --i) {
        // En sah je comprends rien 💀
        bytes.push_back((dataSize >> (8 * i)) & 0xFF);
    }
    bytes.insert(bytes.end(), data.begin(), data.end());
    return bytes;
}

/**
 * Convertit une chaine d'octets en Packet
 */
Packet Packet::fromBytes(std::vector<uint8_t> bytes) {
    uint8_t packetType = bytes[0];
    uint32_t dataSize = 0;

    for (int i = 0; i < 4; ++i) {
        // En sah je comprends rien 💀
        dataSize |= (bytes[i + 1] << (8 * (3 - i)));
    }
    std::vector<uint8_t> data(bytes.begin() + 5, bytes.end());
    return Packet(packetType, data);
}


void Packet::setDataFromStr(const char* str) {
    data.clear();
    for(int i = 0; str[i] != '\0'; ++i) {
        data.push_back(str[i]);
    }
    dataSize = data.size();
}


void Packet::printData() {
    std::cout << "Data: ";
    for (uint8_t byte : data) {
        std::cout << (char)byte;
    }
    std::cout << std::endl;
}