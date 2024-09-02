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
        bytes.push_back((filenameSize >> (8 * i)) & 0xFF);
    }
    for (int i = 7; i >= 0; --i) { // 64 bits = 8 octets
        bytes.push_back((dataSize >> (8 * i)) & 0xFF);
    }
    bytes.insert(bytes.end(), filename.begin(), filename.end());
    bytes.insert(bytes.end(), data.begin(), data.end());
    //print bytes in hexa
    std::cout << "Packet bytes: ";
    for (int i = 0; i < bytes.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i] << " ";
    }
    return bytes;
}

void Packet::fromBytes(std::vector<uint8_t> bytes) {
    size_t offset = 0;
    // 1. Lire le PacketType
    packetType = static_cast<PacketType>(bytes[offset]);
    offset += 1;

    // 2. Lire FilenameSize (8 octets, big-endian)
    uint64_t filenameSizeCount = 0;
    for (int i = 0; i < sizeof(filenameSize); ++i) {
        filenameSizeCount |= static_cast<uint64_t>(bytes[offset + i]) << (8 * (sizeof(filenameSize) - 1 - i));
    }
    filenameSize = filenameSizeCount;
    offset += sizeof(filenameSize);

    // 3. Lire DataSize (8 octets, big-endian)
    uint64_t dataSizeCount = 0;
    for (int i = 0; i < sizeof(dataSize); ++i) {
        dataSizeCount |= static_cast<uint64_t>(bytes[offset + i]) << (8 * (sizeof(dataSize) - 1 - i));
    }
    dataSize = dataSizeCount;
    offset += sizeof(dataSize);

    std::cout << "Filename size: " << filenameSize << std::endl;
    std::cout << "Data size: " << dataSize << std::endl;
    // Vérification de la taille totale pour éviter des débordements
    if (bytes.size() < offset + filenameSize + dataSize) {
        throw std::runtime_error("Taille de bytes insuffisante pour contenir le nom de fichier et les données.");
    }

    // 4. Lire Filename (FilenameSize octets)
    filename.assign(bytes.begin() + offset, bytes.begin() + offset + filenameSize);
    offset += filenameSize;

    // 5. Lire Data (DataSize octets)
    data.assign(bytes.begin() + offset, bytes.begin() + offset + dataSize);
}

void Packet::setDataFromStr(const char* str) {
    data.clear();
    for(int i = 0; str[i] != '\0'; ++i) {
        data.push_back(str[i]);
    }
    filenameSize = 0;
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