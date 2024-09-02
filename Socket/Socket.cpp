#include "Socket.hpp"
#include <iomanip>

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
    std::cout << "Sent packet" << std::endl;
    while (totalSent < packetSize) {
        ssize_t bytesSent = send(clientFd, packet.data() + totalSent, packetSize - totalSent, 0);
        if (bytesSent == -1) {
            std::cerr << "Failed to send packet" << std::endl;
            return false;
        }
        totalSent += bytesSent;
    }
    std::cout << "Sent packet" << std::endl;
    return true;
}


// Fonction utilitaire pour lire les tailles en Big Endian
uint64_t fromBigEndian(const std::vector<uint8_t>& bytes, size_t offset, size_t length) {
    uint64_t value = 0;
    for (size_t i = 0; i < length; ++i) {
        value = (value << 8) | bytes[offset + i];
    }
    return value;
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
            return Packet(PacketType::MESSAGE, "");
        }
        if (bytesReceived == 0) {
            std::cerr << "Connection closed while receiving packet header" << std::endl;
            return Packet(PacketType::MESSAGE, "");
        }
        totalReceived += bytesReceived;
    }

    // Convertir les tailles depuis Big Endian
    std::vector<uint8_t> headerBytes(reinterpret_cast<uint8_t*>(&packetHeader), reinterpret_cast<uint8_t*>(&packetHeader) + sizeof(PacketHeader));
    uint64_t filenameSize = fromBigEndian(headerBytes, 1, 8); // offset 1 pour ignorer le type du paquet
    uint64_t dataSize = fromBigEndian(headerBytes, 9, 8); // offset 9 pour ignorer le type du paquet

    std::cout << "Received packet header" << std::endl;
    std::cout << "Packet Type: " << static_cast<int>(packetHeader.type) << std::endl;
    std::cout << "Filename size: " << filenameSize << std::endl;
    std::cout << "Data size: " << dataSize << std::endl;

    // Allocation des buffers
    char *dataBuffer = (char*)malloc(dataSize);
    if (dataBuffer == nullptr) {
        std::cerr << "Failed to allocate memory for data buffer" << std::endl;
        return Packet(PacketType::MESSAGE, "");
    }

    char *filenameBuffer = (char*)malloc(filenameSize);
    if (filenameBuffer == nullptr) {
        std::cerr << "Failed to allocate memory for filename buffer" << std::endl;
        free(dataBuffer);
        return Packet(PacketType::MESSAGE, "");
    }

    // Lire le nom du fichier
    totalReceived = 0;
    while (totalReceived < filenameSize) {
        ssize_t bytesReceived = recv(clientFd, filenameBuffer + totalReceived, filenameSize - totalReceived, 0);
        if (bytesReceived < 0) {
            perror("recv failed while receiving filename");
            free(filenameBuffer);
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "");
        }
        if (bytesReceived == 0) {
            std::cerr << "Connection closed while receiving filename" << std::endl;
            free(filenameBuffer);
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "");
        }
        totalReceived += bytesReceived;
        std::cout << "Received " << totalReceived << " bytes for filename" << std::endl;
    }

    std::cout << "Received filename: " << std::string(filenameBuffer, filenameSize) << std::endl;

    // Lire les données
    totalReceived = 0;
    while (totalReceived < dataSize) {
        ssize_t bytesReceived = recv(clientFd, dataBuffer + totalReceived, dataSize - totalReceived, 0);
        if (bytesReceived < 0) {
            std::cout << "recv failed while receiving data"<< std::endl;
            free(filenameBuffer);
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "");
        }
        if (bytesReceived == 0) {
            std::cout << "Connection closed while receiving data" << std::endl;
            free(filenameBuffer);
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "");
        }
        totalReceived += bytesReceived;
        std::cout << "Received " << totalReceived << " bytes for data" << std::endl;
    }

    std::string dataString(dataBuffer, dataSize);
    std::cout << filenameBuffer << std::endl;
    if (packetHeader.type == PacketType::DOWNLOAD) {
        std::cout << "Received download packet" << std::endl;
        std::cout << dataString << std::endl;
        dataString = "Storage/" + dataString;
        return Packet(readFileToUint8Vector(dataString.c_str(), PacketType::UPLOAD));
    }

    createFileFromPacket(dataBuffer, std::string(filenameBuffer, filenameSize), dataSize);
    // Libération de la mémoire
    free(filenameBuffer);
    free(dataBuffer);

    return Packet(static_cast<PacketType>(packetHeader.type), "filename"); // Retourner un objet Packet avec les données
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

void Socket::createFileFromPacket(char *data, std::string filename, ssize_t dataSize) {
    std::cout << filename << std::endl;
    std::string filePath = "";
    if(this->isServer) {
        filePath.append("Storage/").append(filename);
    }
    else {
        filePath.append("Downloads/").append(filename);
    }
    std::cout<< filePath << std::endl;
    std::ofstream newFile;
    newFile.open(filePath.c_str(), std::ios_base::binary);
    
    newFile.write(data, dataSize);
    
    std::cout << "File successfully copied" << filePath << std::endl; 
    newFile.close();
}