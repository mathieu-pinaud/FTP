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
    bytes.push_back(packetType);

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
    std::cout << "Packet::fromBytes" << std::endl;
    // Vérifier la taille minimale
    if (bytes.size() < 9) {
        std::cerr << "Byte vector is too small" << std::endl;
        return; // ou lancer une exception
    }

    // Extraire le type de paquet (premier byte)
    packetType = bytes[0];

    // Extraire la taille des données (8 octets)
    uint64_t dataSizecount = 0;
    for (int i = 0; i < 8; ++i) {
        dataSizecount |= (static_cast<uint64_t>(bytes[i + 1]) << (8 * (7 - i)));
    }
    dataSize = dataSizecount;

    // Vérifier que la taille des données est cohérente
    if (dataSize > bytes.size() - 9) {
        std::cerr << "Declared data size exceeds available data size" << std::endl;
        return; // ou ajustez la taille des données selon les besoins
    }
    // Extraire les données
    bytes.erase(bytes.begin(), bytes.begin() + 9);
    data = bytes;
    std::cout << "Data Size: " << dataSize << std::endl;
}


void Packet::setDataFromStr(const char* str) {
    data.clear();
    for(int i = 0; str[i] != '\0'; ++i) {
        data.push_back(str[i]);
    }
    dataSize = data.size();
}


void Packet::copyFile(std::string filename) {
    std::cout << "début d'écriture "<< std::endl;
    size_t lastDotPos = filename.find_last_of('.');
    if (lastDotPos == std::string::npos) {
        throw std::invalid_argument("Le nom de fichier ne contient pas de point ou est mal formé.");
    }

    std::string baseName = filename.substr(0, lastDotPos);
    std::string extension = filename.substr(lastDotPos);
    std::string newFilename = baseName + "_(Copy)" + extension;

    std::ofstream newFile(newFilename, std::ios_base::binary | std::ios_base::trunc);
    if (!newFile) {
        throw std::ios_base::failure("Erreur lors de l'ouverture du fichier pour écriture.");
    }

    if (dataSize != data.size()) {
        throw std::runtime_error("dataSize ne correspond pas à la taille réelle des données.");
    }
    newFile.write(reinterpret_cast<const char*>(data.data()), dataSize);
    if (newFile.fail()) {
        throw std::ios_base::failure("Erreur lors de l'écriture dans le fichier.");
    }
    newFile.flush();
    if (newFile.fail()) {
        throw std::ios_base::failure("Erreur lors de la vidange des tampons de sortie.");
    }
    newFile.close();
    if (newFile.fail()) {
        throw std::ios_base::failure("Erreur lors de la fermeture du fichier.");
    }
}

void Packet::printData() {
    std::cout << "Packet type: " << (int)this->packetType << std::endl;
    std::cout << "Data size: " << this->dataSize << std::endl;
    int type = (int)this->packetType;
    if(type == 1) {
        std::string dataStr(this->data.begin(), this->data.end());
        std::cout << "Data: " << dataStr.c_str() << std::endl;
    }else if(type == 2) {
        this->copyFile("test.zip");
    }
    return;
}