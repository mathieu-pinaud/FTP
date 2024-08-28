#include <iostream>
#include "Packet.hpp"

/**
 * Convertit un Packet en chaine d'octets
 */
std::vector<uint8_t> Packet::toBytes() {
    std::vector<uint8_t> bytes;
    bytes.push_back(packetType);

    for(int i = 3; i >= 0; --i) {
        bytes.push_back((dataSize >> (8 * i)) & 0xFF);
    }
    bytes.insert(bytes.end(), data.begin(), data.end());
    return bytes;
}

/**
 * Convertit une chaine d'octets en Packet
 */
Packet Packet::fromBytes(std::vector<uint8_t> bytes) {
    // Extraire le type de paquet (premier byte)
    this->packetType = bytes[0];

    // Extraire la taille des données (bytes 1 à 4)
    uint32_t dataSize = 0;
    for (int i = 0; i < 4; ++i) {
        this->dataSize |= (bytes[i + 1] << (8 * (3 - i)));
    }

    // Extraire les données (à partir du byte 5 jusqu'à la taille des données)
    std::vector<uint8_t> dataBytes(bytes.begin() + 5, bytes.begin()+ 5 + dataSize);
    int i = dataSize;
    this->data = dataBytes;
    return *this;
}


void Packet::setDataFromStr(const char* str) {
    data.clear();
    for(int i = 0; str[i] != '\0'; ++i) {
        data.push_back(str[i]);
    }
    dataSize = data.size();
}


void Packet::printData() {
    std::string dataStr(this->data.begin(), this->data.end());  
    std::cout << "Data: " << dataStr.c_str() << std::endl;
}