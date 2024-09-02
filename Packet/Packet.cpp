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

void Packet::fromBytes(std::vector<uint8_t> bytes) {
    size_t offset = 0;

    // 1. Lire le PacketType
    packetType = static_cast<PacketType>(bytes[offset]);
    offset += 1;

    // 2. Lire FilenameSize (8 octets, supposons little-endian)
    uint64_t filenameSize = 0;
    for (int i = 0; i < sizeof(filenameSize); ++i) {
        filenameSize |= static_cast<uint64_t>(bytes[offset + i]) << (8 * i);
    }
    offset += sizeof(filenameSize);

    // 3. Lire DataSize (8 octets, supposons little-endian)
    uint64_t dataSize = 0;
    for (int i = 0; i < sizeof(dataSize); ++i) {
        dataSize |= static_cast<uint64_t>(bytes[offset + i]) << (8 * i);
    }
    offset += sizeof(dataSize);

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



    // // Extraire le type de paquet (premier byte)

    // // Lire la taille des données à partir des 8 octets suivants
    // uint64_t dataSizecount = 0;
    // for (int i = 0; i < 8; ++i) { // 64 bits = 8 octets
    //     dataSizecount |= (static_cast<uint64_t>(bytes[i + 1]) << (8 * (7 - i)));
    // }
    // dataSize = dataSizecount;

    // // Vérifier que la taille totale est cohérente
    // if (bytes.size() < 9 + dataSize) {
    //     std::cerr << "Invalid data size: " << dataSize << std::endl;
    //     return;
    // }

    // // Extraire les données (à partir du 9ème byte jusqu'à la fin)
    // std::vector<uint8_t> dataBytes(bytes.begin() + 9, bytes.begin() + 9 + dataSize);
    // data = dataBytes;



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