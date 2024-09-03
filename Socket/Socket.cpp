#include "Socket.hpp"
#include <iomanip>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

bool Socket::createSocket()
{
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    return true;
}

bool Socket::bindSocket(const char* ip, int port)
{
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);

    if (bind(socketFd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "Failed to bind socket" << std::endl;
        return false;
    }
    return true;
}

bool Socket::listenSocket()
{
    if (listen(socketFd, 10) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return false;
    }
    return true;
}

int Socket::acceptSocket()
{
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int clientFd = accept(socketFd, (struct sockaddr*)&clientAddress, &clientAddressSize);
    if (clientFd == -1) {
        std::cerr << "Failed to accept connection" << std::endl;
        return -1;
    }
    return clientFd;
}

bool Socket::closeSocket()
{
    if (close(socketFd) == -1) {
        std::cerr << "Failed to close socket" << std::endl;
        return false;
    }
    return true;
}

bool Socket::sendPacket(int clientFd, Packet message)
{   
    std::vector<uint8_t> packet = message.toBytes();
    int packetSize = packet.size() + sizeof(PacketHeader); // Size includes header size
    ssize_t totalSent = 0;
    while (totalSent < packetSize) {
        ssize_t bytesSent = send(clientFd, packet.data() + totalSent, packetSize - totalSent, 0);
        if (bytesSent == -1) {
            std::cerr << "Failed to send packet" << std::endl;
            return false;
        }
        totalSent += bytesSent;
    }
    return true;
}


// Fonction utilitaire pour lire les tailles en Big Endian
uint64_t fromBigEndian(const std::vector<uint8_t> bytes, size_t offset, size_t length) {
    uint64_t value = 0;
    for (size_t i = 0; i < length; ++i) {
        value = (value << 8) | bytes[offset + i];
    }
    return value;
}

Packet Socket::managePacket(char *dataBuffer, uint64_t dataSize, std::string userName, std::string filename, PacketType type) {

    switch(type) {

        case PacketType::MESSAGE: {
            Packet p = Packet(PacketType::MESSAGE, std::string(dataBuffer,dataSize).c_str(), userName.c_str());
            p.printData();
            p.setData({0});
            return p;
            break;
        }
        
        case PacketType::PASSWORD: {
            std::cout << "password" << std::endl;
            break;
        }
        
        case PacketType::UPLOAD: {
            std::cout << "upload" << std::endl;
            return this->upload(dataBuffer, dataSize, userName, filename);
            break;
        }
        
        case PacketType::DOWNLOAD: {
            std::cout << "dowload" << std::endl;
            return this->download(std::string(dataBuffer, dataSize), userName);
            break;
        }

        case PacketType::DELETE: {
            return this->deleteFile(userName, std::string(dataBuffer, dataSize));
            break;
        }

        default:
            std::cout << "default" << std::endl;
            break;
    }

}

Packet Socket::download(std::string filenameString,std::string userString) {

    if (this->isServer) {
        std::cout << "Received download packet" << std::endl;
        std::cout << filenameString << std::endl;
        filenameString = "Storage/"+ userString + "/" + filenameString;

        return Packet(readFileToUint8Vector(filenameString.c_str(), PacketType::UPLOAD, userString));
    }
}

Packet Socket::upload(char *dataBuffer, uint64_t dataSize, std::string userName, std::string filename) {
    createFileFromPacket(dataBuffer, filename, dataSize, userName);
    return Packet(PacketType::MESSAGE, "File uploaded successfully", userName.c_str());
}

Packet Socket::deleteFile(std::string userName, std::string filename) {
    std::string storagePath = "Storage/";
    std::string filePath = userName;
    filePath = filePath.append("/").append(filename);
    if(remove(storagePath.append(filePath).c_str())) {
        filePath = filePath.append(" deleted successfully");
        return Packet(PacketType::MESSAGE, filePath.c_str(), userName.c_str());
    }
    else {
        return Packet(PacketType::MESSAGE, "Error while deleting file", userName.c_str());
    }
}

// Fonction pour recevoir et traiter un paquet
Packet Socket::receivePacket(int clientFd) {   
    PacketHeader packetHeader;

    // Lire l'en-tête
    ssize_t totalReceived = 0;
    while (totalReceived < sizeof(PacketHeader)) {
        ssize_t bytesReceived = recv(clientFd, reinterpret_cast<char*>(&packetHeader) + totalReceived, sizeof(PacketHeader) - totalReceived, 0);
        if (bytesReceived < 0) {
            perror("recv failed while receiving packet header");
            return Packet(PacketType::MESSAGE, "","");
        }
        if (bytesReceived == 0) {
            std::cerr << "Connection closed while receiving packet header" << std::endl;
            return Packet(PacketType::MESSAGE, "","");
        }
        totalReceived += bytesReceived;
    }

    // Convertir les tailles depuis Big Endian
    std::vector<uint8_t> headerBytes(reinterpret_cast<uint8_t*>(&packetHeader), reinterpret_cast<uint8_t*>(&packetHeader) + sizeof(PacketHeader));
    uint32_t userNameSize = fromBigEndian(headerBytes, 1, 4); // offset 1 pour ignorer le type du paquet
    uint64_t filenameSize = fromBigEndian(headerBytes, 5, 8); // offset 9 pour ignorer le type du paquet
    uint64_t dataSize = fromBigEndian(headerBytes, 13, 8); // offset 17 pour ignorer le type du paquet

    std::cout << "Received packet header" << std::endl;
    std::cout << "Packet Type: " << static_cast<int>(packetHeader.type) << std::endl;
    std::cout << "userName size: " << userNameSize << std::endl;
    std::cout << "Filename size: " << filenameSize << std::endl;
    std::cout << "Data size: " << dataSize << std::endl;

    // Allocation des buffers

    char *userNameBuffer = (char*)malloc(userNameSize);
    if (userNameBuffer == nullptr) {
        std::cerr << "Failed to allocate memory for filename buffer" << std::endl;
        free(userNameBuffer);
        return Packet(PacketType::MESSAGE, "","");
    }

    char *dataBuffer = (char*)malloc(dataSize);
    if (dataBuffer == nullptr) {
        std::cerr << "Failed to allocate memory for data buffer" << std::endl;
        return Packet(PacketType::MESSAGE, "","");
    }

    char *filenameBuffer = (char*)malloc(filenameSize);
    if (filenameBuffer == nullptr) {
        std::cerr << "Failed to allocate memory for filename buffer" << std::endl;
        free(dataBuffer);
        return Packet(PacketType::MESSAGE, "","");
    }

    // Lire le nom de l'utilisateur
    totalReceived = 0;
    while (totalReceived < userNameSize) {
        ssize_t bytesReceived = recv(clientFd, userNameBuffer + totalReceived, userNameSize - totalReceived, 0);
        if (bytesReceived < 0) {
            perror("recv failed while receiving username");
            free(userNameBuffer);
            free(filenameBuffer);
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "","");
        }
        if (bytesReceived == 0) {
            std::cerr << "Connection closed while receiving username" << std::endl;
            free(userNameBuffer);
            free(filenameBuffer);
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "","");
        }
        totalReceived += bytesReceived;
        std::cout << "Received " << totalReceived << " bytes for username" << std::endl;
    }

    // Lire le nom du fichier
    totalReceived = 0;
    while (totalReceived < filenameSize) {
        ssize_t bytesReceived = recv(clientFd, filenameBuffer + totalReceived, filenameSize - totalReceived, 0);
        if (bytesReceived < 0) {
            perror("recv failed while receiving filename");
            free(userNameBuffer);
            free(filenameBuffer);
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "","");
        }
        if (bytesReceived == 0) {
            std::cerr << "Connection closed while receiving filename" << std::endl;
            free(userNameBuffer);
            free(filenameBuffer);
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "","");
        }
        totalReceived += bytesReceived;
        std::cout << "Received " << totalReceived << " bytes for filename" << std::endl;
    }

    // Lire les données
    totalReceived = 0;
    while (totalReceived < dataSize) {
        ssize_t bytesReceived = recv(clientFd, dataBuffer + totalReceived, dataSize - totalReceived, 0);
        if (bytesReceived < 0) {
            std::cout << "recv failed while receiving data"<< std::endl;
            free(userNameBuffer);
            free(filenameBuffer);
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "","");
        }
        if (bytesReceived == 0) {
            std::cout << "Connection closed while receiving data" << std::endl;
            free(userNameBuffer);
            free(filenameBuffer);
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "","");
        }
        totalReceived += bytesReceived;
        std::cout << "Received " << totalReceived << " bytes for data" << std::endl;
    }

    std::string filenameString(filenameBuffer, filenameSize);
    std::string userString(userNameBuffer, userNameSize);
    Packet p = managePacket(dataBuffer, dataSize, userString, filenameString, packetHeader.type);
    free(userNameBuffer);
    free(filenameBuffer);
    free(dataBuffer);
    return p; // Retourner un objet Packet avec les données
}


bool Socket::connectSocket(const char* ip, int port)
{
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);

    if (connect(socketFd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "Failed to connect to server" << std::endl;
        return false;
    }
    return true;
}

void Socket::createFileFromPacket(char *data, std::string filename, ssize_t dataSize, std::string userName) {
    std::string storagePath = "";
    if(this->isServer) {
        storagePath.append("Storage/").append(userName).append("/").append(filename);
    }
    else {
        if (!fs::exists("Downloads/")) {
            if(fs::create_directory("Downloads/")) {
                std::cout << "Directory created: " << "Downloads/" << std::endl;
            }
        }
        filename=filename.substr(filename.find_last_of("/") + 1);
        storagePath.append("Downloads/").append(filename);
    }
    std::ofstream newFile;
    newFile.open(storagePath.c_str(), std::ios_base::binary);
    
    newFile.write(data, dataSize);
    
    std::cout << "File successfully copied in " << storagePath << std::endl;
    newFile.close();
}