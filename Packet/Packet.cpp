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

    for (int i = 3; i >= 0; --i) { // 32 bits = 4 octets
        bytes.push_back((userNameSize >> (4 * i)) & 0xFF);
    }
    for (int i = 7; i >= 0; --i) { // 64 bits = 8 octets
        bytes.push_back((filenameSize >> (4 * i)) & 0xFF);
    }
    for (int i = 7; i >= 0; --i) { // 64 bits = 8 octets
        bytes.push_back((pathSize >> (4 * i)) & 0xFF);
    }
    for (int i = 7; i >= 0; --i) { // 64 bits = 8 octets
        bytes.push_back((dataSize >> (8 * i)) & 0xFF);
    }
    bytes.insert(bytes.end(), userName.begin(), userName.end());
    bytes.insert(bytes.end(), pathName.begin(), pathName.end());
    bytes.insert(bytes.end(), filename.begin(), filename.end());
    bytes.insert(bytes.end(), data.begin(), data.end());
    return bytes;
}

void Packet::fromBytes(std::vector<uint8_t> bytes) {
    size_t offset = 0;
    // 1. Lire le PacketType
    packetType = static_cast<PacketType>(bytes[offset]);
    offset += 1;

    // 2. Lire userNameSize (4 octets, big-endian)
    uint32_t userNameSizeCount = 0;
    for (int i = 0; i < sizeof(userNameSize); ++i) {
        userNameSizeCount |= static_cast<uint32_t>(bytes[offset + i]) << (4 * (sizeof(userNameSize) - 1 - i));
    }
    userNameSize = userNameSizeCount;
    offset += sizeof(userNameSize);

    // 2. Lire userNameSize (4 octets, big-endian)
    uint32_t pathNameSizeCount = 0;
    for (int i = 0; i < sizeof(pathSize); ++i) {
        pathNameSizeCount |= static_cast<uint32_t>(bytes[offset + i]) << (4 * (sizeof(pathNameSizeCount) - 1 - i));
    }
    pathSize = pathNameSizeCount;
    offset += sizeof(pathSize);

    // 2. Lire FilenameSize (8 octets, big-endian)
    uint32_t filenameSizeCount = 0;
    for (int i = 0; i < sizeof(filenameSize); ++i) {
        filenameSizeCount |= static_cast<uint32_t>(bytes[offset + i]) << (4 * (sizeof(filenameSize) - 1 - i));
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
    // Vérification de la taille totale pour éviter des débordements
    if (bytes.size() < offset + filenameSize + dataSize + userNameSize + pathSize) {
        throw std::runtime_error("Taille de bytes insuffisante pour contenir le nom de fichier et les données.");
    }

    // 4. Lire Username (UsernameSize octets)
    userName.assign(bytes.begin() + offset, bytes.begin() + offset + userNameSize);
    offset += userNameSize;

    // 4. Lire Filename (FilenameSize octets)
    filename.assign(bytes.begin() + offset, bytes.begin() + offset + filenameSize);
    offset += filenameSize;

    // 4. Lire pathName (pathName octets)
    pathName.assign(bytes.begin() + offset, bytes.begin() + offset + pathSize);
    offset += pathSize;

    // 5. Lire Data (DataSize octets)
    data.assign(bytes.begin() + offset, bytes.begin() + offset + dataSize);
}

void Packet::setDataFromStr(const char* str, const char* user, const char* path) {
    data.clear();
    filenameSize = 0;
    std::string userStr(user);
    for(int i = 0; user[i] != '\0'; ++i) {
        userName.push_back(user[i]);
    }
    userNameSize = userStr.size();
    if(path == nullptr) {
        pathName.clear();
        pathSize = 0;
    }else {
        std::string pathStr(path);
        for(int i = 0; path[i] != '\0'; ++i) {
            pathName.push_back(path[i]);
        }
        pathSize = pathStr.size();
        std::cout << "Path size: " << pathSize << std::endl;
    }
    for(int i = 0; str[i] != '\0'; ++i) {
        data.push_back(str[i]);
    }
    std::string data(str);
    dataSize = data.size();
}

void Packet::printData() {
    std::string dataStr(this->data.begin(), this->data.end());
    std::cout << dataStr.c_str() << std::endl;
}

std::string Packet::getDataStr() {
    std::string dataStr(this->data.begin(), this->data.end());
    return dataStr;
}   