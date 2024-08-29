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

    uint64_t dataSizecount = 0;
    for (int i = 0; i < 8; ++i) { // 64 bits = 8 octets
        dataSizecount |= (static_cast<uint64_t>(bytes[i + 1]) << (8 * (7 - i)));
    }
    dataSize=dataSizecount;
    // Extraire les données (à partir du byte 9 jusqu'à la taille des données)
    std::vector<uint8_t> dataBytes(bytes.begin() + 9, bytes.begin()+ 9 + dataSize);
    data = dataBytes;
}


void Packet::setDataFromStr(const char* str) {
    data.clear();
    for(int i = 0; str[i] != '\0'; ++i) {
        data.push_back(str[i]);
    }
    dataSize = data.size();
}


void Packet::copyFile(std::string filename) {
        std::cout << "sizeClient" << this->getDataSize() << std::endl;
    std::vector<std::string> filenameVector = split(filename, ".");
    std::string newFilename = filenameVector[0].append(" (Copy).").append(filenameVector[1]);
    std::ofstream newFile;
    newFile.open(newFilename.c_str(), std::ios_base::binary);
    for (const uint8_t byte : this->data) {
        newFile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
    }
    newFile.close();
}

void Packet::printData() {
    std::cout << "Packet type: " << (int)this->packetType << std::endl;
    std::cout << "Data size: " << this->dataSize << std::endl;
    int type = (int)this->packetType;
    if(type == 1) {
        std::string dataStr(this->data.begin(), this->data.end());
        std::cout << "Data: " << dataStr.c_str() << std::endl;
    }else if(type == 2) {
        this->copyFile("tests.png");
    }
    return;
}