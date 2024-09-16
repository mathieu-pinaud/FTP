#include "Socket.hpp"
#include "../Utils/Logger.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace fs = std::filesystem;

Logger *logger = Logger::getInstance();

bool Socket::createSocket() {
  socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFd == -1) {
    logger->errLog("Failed to create socket");
    return false;
  }
  return true;
}

bool Socket::bindSocket(const char *ip, int port) {
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(ip);
  address.sin_port = htons(port);

  if (bind(socketFd, (struct sockaddr *)&address, sizeof(address)) == -1) {
    logger->errLog("Failed to bind socket");
    return false;
  }
  return true;
}

bool Socket::listenSocket() {
  if (listen(socketFd, 10) == -1) {
    logger->errLog("Failed to listen to socket");
    return false;
  }
  return true;
}

int Socket::acceptSocket() {
  struct sockaddr_in clientAddress;
  socklen_t clientAddressSize = sizeof(clientAddress);
  int clientFd =
      accept(socketFd, (struct sockaddr *)&clientAddress, &clientAddressSize);
  if (clientFd == -1) {
    logger->errLog("Failed to accept connection");
    return -1;
  }
  return clientFd;
}

bool Socket::closeSocket() {
  if (close(socketFd) == -1) {
    logger->errLog("Failed to close socket");
    return false;
  }
  return true;
}

bool Socket::sendPacket(int clientFd, Packet message) {
  std::vector<uint8_t> packet = message.toBytes();
  int packetSize = packet.size(); // Size includes header size
  ssize_t totalSent = 0;
  while (totalSent < packetSize) {
    ssize_t bytesSent =
        send(clientFd, packet.data() + totalSent, packetSize - totalSent, 0);
    if (bytesSent == -1) {
      logger->errLog("Failed to send packet");
      return false;
    }
    totalSent += bytesSent;
  }

  logger->log(this->isServer, message);

  return true;
}

// Fonction utilitaire pour lire les tailles en Big Endian
uint64_t fromBigEndian(const std::vector<uint8_t> bytes, size_t offset,
                       size_t length) {
  uint64_t value = 0;
  for (size_t i = 0; i < length; ++i) {
    value = (value << 8) | bytes[offset + i];
  }
  return value;
}

Packet Socket::managePacket(char *dataBuffer, uint64_t dataSize,
                            std::string userName, std::string filename,
                            PacketType type) {

  switch (type) {

  case PacketType::MESSAGE: {
    Packet p =
        Packet(PacketType::MESSAGE, std::string(dataBuffer, dataSize).c_str(),
               userName.c_str());
    p.printData();
    return p;
    break;
  }

  case PacketType::PASSWORD: {
    return this->password(dataBuffer, dataSize, userName);
    break;
  }

  case PacketType::UPLOAD: {
    return this->upload(dataBuffer, dataSize, userName, filename);
    break;
  }

  case PacketType::DOWNLOAD: {
    return this->download(std::string(dataBuffer, dataSize), userName);
    break;
  }

  case PacketType::DELETE: {
    return this->deleteFile(userName, std::string(dataBuffer, dataSize));
    break;
  }

  case PacketType::CONNECT: {
    return this->acceptClient(dataBuffer, dataSize, userName);
    break;
  }

  case PacketType::REMOVE: {
    return this->removeFolder(userName, std::string(dataBuffer, dataSize));
    break;
  }

  case PacketType::RENAME: {
    return this->renameFolder(userName, std::string(dataBuffer, dataSize),
                              filename);
    break;
  }

  case PacketType::CREATE: {
    std::cout << std::string(dataBuffer, dataSize) << std::endl;
    std::cout << filename << std::endl;
    return this->createFolder(userName, std::string(dataBuffer, dataSize),
                              filename);
    break;
  }

  case PacketType::LIST: {
    return this->listFolder(userName, std::string(dataBuffer, dataSize));
    break;
  }

  default:
    return Packet(PacketType::MESSAGE, "", "");
    break;
  }
  return Packet(PacketType::MESSAGE, "FAILED", userName.c_str());
}

Packet Socket::removeFolder(std::string userName, std::string path) {
  std::string folderPath = "Storage/";
  folderPath = folderPath.append(userName).append("/").append(path);
  if (fs::remove_all(folderPath.c_str())) {
    if (path.empty()) {
      fs::create_directory("Storage/" + userName);
    }
    folderPath = folderPath.append(" deleted successfully");
    return Packet(PacketType::MESSAGE, folderPath.c_str(), userName.c_str());
  } else {
    return Packet(PacketType::MESSAGE, "Error while deleting folder",
                  userName.c_str());
  }
}

Packet Socket::createFolder(std::string userName, std::string path,
                            std::string foldername) {
  std::string folderPath = "Storage/";
  folderPath.append(userName).append("/");
  if (!path.empty()) {
    folderPath.append(path).append("/").append(foldername);
  } else {
    folderPath.append(foldername);
  }
  if (!fs::exists(folderPath)) {
    if (fs::create_directory(folderPath)) {
      std::cout << "Directory created: " << foldername << std::endl;
      return Packet(PacketType::MESSAGE, "Directory created", userName.c_str());
    } else {
      return Packet(PacketType::MESSAGE, "Error while creating folder",
                    userName.c_str());
    }
  } else {
    return Packet(PacketType::MESSAGE, "Error folder already created",
                  userName.c_str());
  }
}

Packet Socket::renameFolder(std::string userName, std::string path,
                            std::string foldername) {

  if (path.empty())
    return Packet(PacketType::MESSAGE, "path vide", userName.c_str());
  std::string folderPath = "Storage/";
  folderPath.append(userName).append("/").append(path);
  std::string newfolderPath = folderPath;
  size_t pos = newfolderPath.find_last_of('/');
  if (pos != std::string::npos) {
    newfolderPath = folderPath.substr(0, pos + 1);
  }

  newfolderPath.append(foldername);
  if (fs::exists(folderPath)) {
    fs::rename(folderPath, newfolderPath);
    std::cout << "Folder renamed: " << foldername << std::endl;
    return Packet(PacketType::MESSAGE, "Folder renamed", userName.c_str());
  } else {
    return Packet(PacketType::MESSAGE, "Error folder not found",
                  userName.c_str());
  }
}

Packet Socket::listFolder(std::string userName, std::string path) {
  std::string folderPath = "Storage/";
  folderPath.append(userName).append("/").append(path);
  std::string list;
  if (!fs::exists(folderPath)) {
    return Packet(PacketType::MESSAGE, "Error folder not found",
                  userName.c_str());
  }
  for (const auto &entry : fs::directory_iterator(folderPath)) {
    list.append(entry.path().filename().string()).append("\n");
  }
  if (list.empty()) {
    list = "Folder is empty";
  }
  return Packet(PacketType::MESSAGE, list.c_str(), userName.c_str());
}

Packet Socket::acceptClient(char *dataBuffer, uint64_t dataSize,
                            std::string userName) {
  if (this->isServer) {
    std::cout << "Received connect packet" << std::endl;
    return Packet(PacketType::PASSWORD, "Veuillez entrer un mot de passe",
                  userName.c_str());
  } else {
    return Packet(PacketType::MESSAGE, "Connexion établie", userName.c_str());
  }
}

std::string Socket::getPassword() {
  std::string password;
  std::cout << "Enter password: ";
  std::cin >> password;
  return password;
}

Packet Socket::password(char *dataBuffer, uint64_t dataSize,
                        std::string userName) {
  if (this->isServer) {
    std::cout << "Received password packet" << std::endl;
    if (isPasswordValid(userName, std::string(dataBuffer, dataSize))) {
      return Packet(PacketType::MESSAGE, "Connexion accepté", userName.c_str());
    } else {
      return Packet(PacketType::MESSAGE, "Connexion refusé", userName.c_str());
    }
  } else {
    std::string password = this->getPassword();
    return Packet(PacketType::PASSWORD, password.c_str(), userName.c_str());
  }
}

Packet Socket::download(std::string filenameString, std::string userString) {

  std::cout << "Received download packet" << std::endl;
  std::cout << filenameString << std::endl;
  filenameString = "Storage/" + userString + "/" + filenameString;
  return Packet(readFileToUint8Vector(filenameString.c_str(),
                                      PacketType::UPLOAD, userString));
}

Packet Socket::upload(char *dataBuffer, uint64_t dataSize, std::string userName,
                      std::string filename) {
  createFileFromPacket(dataBuffer, filename, dataSize, userName);
  return Packet(PacketType::MESSAGE, "File uploaded successfully",
                userName.c_str());
}

Packet Socket::deleteFile(std::string userName, std::string filename) {
  std::string storagePath = "Storage/";
  std::string filePath = userName;
  filePath = filePath.append("/").append(filename);
  if (remove(storagePath.append(filePath).c_str()) == 0) {
    filePath = filePath.append(" deleted successfully");
    return Packet(PacketType::MESSAGE, filePath.c_str(), userName.c_str());
  } else {
    logger->errLog("Error while deleting file");
    return Packet(PacketType::MESSAGE, "Error while deleting file",
                  userName.c_str());
  }
}

// Fonction pour recevoir et traiter un paquet
Packet Socket::receivePacket(int clientFd) {
  PacketHeader packetHeader;

  // Lire l'en-tête
  ssize_t totalReceived = 0;
  while (totalReceived < sizeof(PacketHeader)) {
    ssize_t bytesReceived =
        recv(clientFd, reinterpret_cast<char *>(&packetHeader) + totalReceived,
             sizeof(PacketHeader) - totalReceived, 0);
    if (bytesReceived <= 0) {
      perror("recv failed while receiving packet header");
      logger->errLog("Connection closed while receiving packet header");
      return Packet(PacketType::MESSAGE, "", "");
    }
    totalReceived += bytesReceived;
  }
  // Convertir les tailles depuis Big Endian
  std::vector<uint8_t> headerBytes(reinterpret_cast<uint8_t *>(&packetHeader),
                                   reinterpret_cast<uint8_t *>(&packetHeader) +
                                       sizeof(PacketHeader));
  uint8_t type = fromBigEndian(headerBytes, 0, 1);
  uint32_t userNameSize = fromBigEndian(
      headerBytes, 1, 4); // offset 1 pour ignorer le type du paquet
  uint64_t filenameSize = fromBigEndian(
      headerBytes, 5, 8); // offset 9 pour ignorer le type du paquet
  uint64_t dataSize = fromBigEndian(
      headerBytes, 13, 8); // offset 17 pour ignorer le type du paquet

  // Allocation des buffers

  char *userNameBuffer = (char *)malloc(userNameSize);
  if (userNameBuffer == nullptr) {
    logger->errLog("Failed to allocate memory for username buffer");
    free(userNameBuffer);
    return Packet(PacketType::MESSAGE, "", "");
  }

  char *dataBuffer = (char *)malloc(dataSize);
  if (dataBuffer == nullptr) {
    logger->errLog("Failed to allocate memory for data buffer");
    return Packet(PacketType::MESSAGE, "", "");
  }

  char *filenameBuffer = (char *)malloc(filenameSize);
  if (filenameBuffer == nullptr) {
    logger->errLog("Failed to allocate memory for filename buffer");
    free(dataBuffer);
    return Packet(PacketType::MESSAGE, "", "");
  }

  // Lire le nom de l'utilisateur
  totalReceived = 0;
  while (totalReceived < userNameSize) {
    ssize_t bytesReceived = recv(clientFd, userNameBuffer + totalReceived,
                                 userNameSize - totalReceived, 0);
    if (bytesReceived < 0) {
      perror("recv failed while receiving username");
      free(userNameBuffer);
      free(filenameBuffer);
      free(dataBuffer);
      return Packet(PacketType::MESSAGE, "", "");
    }
    if (bytesReceived == 0) {
      logger->errLog("Connection closed while receiving username");
      free(userNameBuffer);
      free(filenameBuffer);
      free(dataBuffer);
      return Packet(PacketType::MESSAGE, "", "");
    }
    totalReceived += bytesReceived;
  }

  // Lire le nom du fichier
  totalReceived = 0;
  while (totalReceived < filenameSize) {
    ssize_t bytesReceived = recv(clientFd, filenameBuffer + totalReceived,
                                 filenameSize - totalReceived, 0);
    if (bytesReceived < 0) {
      perror("recv failed while receiving filename");
      free(userNameBuffer);
      free(filenameBuffer);
      free(dataBuffer);
      return Packet(PacketType::MESSAGE, "", "");
    }
    if (bytesReceived == 0) {
      logger->errLog("Connection closed while receiving filename");
      free(userNameBuffer);
      free(filenameBuffer);
      free(dataBuffer);
      return Packet(PacketType::MESSAGE, "", "");
    }
    totalReceived += bytesReceived;
  }

  // Lire les données
  totalReceived = 0;
  while (totalReceived < dataSize) {
    ssize_t bytesReceived =
        recv(clientFd, dataBuffer + totalReceived, dataSize - totalReceived, 0);
    if (bytesReceived < 0) {
      std::cout << "recv failed while receiving data" << std::endl;
      free(userNameBuffer);
      free(filenameBuffer);
      free(dataBuffer);
      return Packet(PacketType::MESSAGE, "", "");
    }
    if (bytesReceived == 0) {
      logger->errLog("Connection closed while receiving data");
      free(userNameBuffer);
      free(filenameBuffer);
      free(dataBuffer);
      return Packet(PacketType::MESSAGE, "", "");
    }
    totalReceived += bytesReceived;
  }

  std::string filenameString(filenameBuffer, filenameSize);
  std::string userString(userNameBuffer, userNameSize);
  Packet p = managePacket(dataBuffer, dataSize, userString, filenameString,
                          static_cast<PacketType>(packetHeader.type));
  free(userNameBuffer);
  free(filenameBuffer);
  free(dataBuffer);

  logger->log(this->isServer, p);

  return p; // Retourner un objet Packet avec les données
}

bool Socket::connectSocket(const char *ip, int port) {
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(ip);
  address.sin_port = htons(port);

  if (connect(socketFd, (struct sockaddr *)&address, sizeof(address)) == -1) {
    std::cerr << "Failed to connect to server" << std::endl;
    return false;
  }
  return true;
}

void Socket::createFileFromPacket(char *data, std::string filename,
                                  ssize_t dataSize, std::string userName) {
  std::string storagePath = "";
  if (this->isServer) {
    storagePath.append("Storage/")
        .append(userName)
        .append("/")
        .append(filename);
  } else {
    if (!fs::exists("Downloads/")) {
      if (fs::create_directory("Downloads/")) {
        std::cout << "Directory created: "
                  << "Downloads/" << std::endl;
      }
    }
    filename = filename.substr(filename.find_last_of("/") + 1);
    storagePath.append("Downloads/").append(filename);
  }
  std::ofstream newFile;
  newFile.open(storagePath.c_str(), std::ios_base::binary);

  newFile.write(data, dataSize);

  std::cout << "File successfully copied in " << storagePath << std::endl;
  newFile.close();
}