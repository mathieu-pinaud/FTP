#include <iostream>
#include <iomanip>
#include <fstream>
#include "Packet.hpp"
#include "../Utils/Utils.hpp"

/**
 * Convertit un Packet en chaine d'octets
 */
std::vector<uint8_t> Packet::toBytes() {
    std::vector<uint8_t> bytes;
    bytes.push_back(static_cast<uint8_t>(packetType));

    for (int i = 7; i >= 0; --i) { // 64 bits = 8 octets
        bytes.push_back((dataSize >> (8 * i)) & 0xFF);
    }
    bytes.insert(bytes.end(), data.begin(), data.end());
    return bytes;
}

/**
 * Convertit une chaine d'octets en Packet
 */
void Packet::fromBytes(std::vector<uint8_t> bytes) {
    // Extraire le type de paquet (premier byte)
    packetType = static_cast<PacketType>(bytes[0]);

    // Lire la taille des données à partir des 8 octets suivants
    uint64_t dataSizecount = 0;
    for (int i = 0; i < 8; ++i) { // 64 bits = 8 octets
        dataSizecount |= (static_cast<uint64_t>(bytes[i + 1]) << (8 * (7 - i)));
    }
    dataSize = dataSizecount;

    // Vérifier que la taille totale est cohérente
    if (bytes.size() < 9 + dataSize) {
        std::cerr << "Invalid data size: " << dataSize << std::endl;
        return;
    }

    // Extraire les données (à partir du 9ème byte jusqu'à la fin)
    std::vector<uint8_t> dataBytes(bytes.begin() + 9, bytes.begin() + 9 + dataSize);
    data = dataBytes;
}


void Packet::setDataFromStr(const char* str) {
    data.clear();
    for(int i = 0; str[i] != '\0'; ++i) {
        data.push_back(str[i]);
    }
    dataSize = data.size();
}

void Packet::printData() {
    std::cout << "Packet type: " << (int)this->packetType << std::endl;
    std::cout << "Data size: " << this->dataSize << std::endl;
    int type = (int)this->packetType;
    if(type == 1) {
        std::string dataStr(this->data.begin(), this->data.end());
        std::cout << "Data: " << dataStr.c_str() << std::endl;
    }
    return;
}