#include "Packet.hpp"
#include "../Utils/Utils.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>


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
    bytes.push_back((filenameSize >> (8 * i)) & 0xFF);
  }
  for (int i = 7; i >= 0; --i) { // 64 bits = 8 octets
    bytes.push_back((dataSize >> (8 * i)) & 0xFF);
  }
  bytes.insert(bytes.end(), userName.begin(), userName.end());
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
    userNameSizeCount |= static_cast<uint32_t>(bytes[offset + i])
                         << (4 * (sizeof(userNameSize) - 1 - i));
  }
  userNameSize = userNameSizeCount;
  offset += sizeof(userNameSize);

  // 2. Lire FilenameSize (8 octets, big-endian)
  uint64_t filenameSizeCount = 0;
  for (int i = 0; i < sizeof(filenameSize); ++i) {
    filenameSizeCount |= static_cast<uint64_t>(bytes[offset + i])
                         << (8 * (sizeof(filenameSize) - 1 - i));
  }
  filenameSize = filenameSizeCount;
  offset += sizeof(filenameSize);

  // 3. Lire DataSize (8 octets, big-endian)
  uint64_t dataSizeCount = 0;
  for (int i = 0; i < sizeof(dataSize); ++i) {
    dataSizeCount |= static_cast<uint64_t>(bytes[offset + i])
                     << (8 * (sizeof(dataSize) - 1 - i));
  }
  dataSize = dataSizeCount;
  offset += sizeof(dataSize);
  // Vérification de la taille totale pour éviter des débordements
  if (bytes.size() < offset + filenameSize + dataSize + userNameSize) {
    throw std::runtime_error("Taille de bytes insuffisante pour contenir le "
                             "nom de fichier et les données.");
  }

  // 4. Lire Username (UsernameSize octets)
  userName.assign(bytes.begin() + offset,
                  bytes.begin() + offset + userNameSize);
  offset += userNameSize;

  // 4. Lire Filename (FilenameSize octets)
  filename.assign(bytes.begin() + offset,
                  bytes.begin() + offset + filenameSize);
  offset += filenameSize;

  // 5. Lire Data (DataSize octets)
  data.assign(bytes.begin() + offset, bytes.begin() + offset + dataSize);
}

void Packet::setDataFromStr(const char *str, const char *user,
                            const char *folder) {
  data.clear();
  userName.clear();
  filename.clear();
  for (int i = 0; user[i] != '\0'; ++i) {
    userName.push_back(user[i]);
  }
  for (int i = 0; folder[i] != '\0'; ++i) {
    filename.push_back(folder[i]);
  }
  for (int i = 0; str[i] != '\0'; ++i) {
    data.push_back(str[i]);
  }
  std::string folderStr(folder);
  filenameSize = folderStr.size();
  std::string userStr(user);
  userNameSize = userStr.size();
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

std::string Packet::getUserNameStr() {
  std::string usernameStr(this->userName.begin(), this->userName.end());
  return usernameStr;
}

std::string Packet::packetTypeToString(PacketType pt) {
  switch (pt) {
  case PacketType::CONNECT:
    return "CONNECT";
  case PacketType::DELETE:
    return "DELETE";
  case PacketType::DOWNLOAD:
    return "DOWNLOAD";
  case PacketType::MESSAGE:
    return "MESSAGE";
  case PacketType::PASSWORD:
    return "PASSWORD";
  case PacketType::UPLOAD:
    return "UPLOAD";
  case PacketType::REMOVE:
    return "REMOVE";
  case PacketType::CREATE:
    return "CREATE";
  case PacketType::RENAME:
    return "RENAME";
  case PacketType::LIST:
    return "LIST";
  default:
    break;
  }

  return "";
}