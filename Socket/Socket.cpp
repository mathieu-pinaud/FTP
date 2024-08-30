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

bool Socket::sendPacket(int clientFd, Packet message, int size)
{   
    std::vector<uint8_t> packet = message.toBytes();
    int packetSize = size + sizeof(PacketHeader); // Size includes header size
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

Packet Socket::receivePacket(int clientFd)
{   
    PacketHeader packetHeader;

    ssize_t totalReceived = 0;
    while (totalReceived < sizeof(PacketHeader)) {
        ssize_t bytesReceived = recv(clientFd, reinterpret_cast<char*>(&packetHeader) + totalReceived, sizeof(PacketHeader) - totalReceived, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Failed to receive complete packet header" << std::endl;
            return Packet(PacketType::MESSAGE, "");
        }
        totalReceived += bytesReceived;
    }
    uint64_t dataSize = 0;
    for (int i = 0; i < sizeof(packetHeader.size); ++i) {
        dataSize |= static_cast<uint64_t>(reinterpret_cast<uint8_t*>(&packetHeader.size)[i]) << (8 * (sizeof(packetHeader.size) - 1 - i));
    }
    char *dataBuffer = (char*)malloc(dataSize);
    if (dataBuffer == nullptr) {
        std::cerr << "Failed to allocate memory for data buffer" << std::endl;
        return Packet(PacketType::MESSAGE, "");
    }

    totalReceived = 0;
    while (totalReceived < dataSize) {
        ssize_t bytesReceived = recv(clientFd, dataBuffer + totalReceived, dataSize - totalReceived, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Failed to receive complete data" << std::endl;
            free(dataBuffer);
            return Packet(PacketType::MESSAGE, "");
        }
        totalReceived += bytesReceived;
    }
    
    // Sauvegarder les données reçues dans un fichier
    std::ofstream newFile("Storage/testMat.mkv", std::ios_base::binary);    
    newFile.write(dataBuffer, dataSize);
    newFile.close();

    free(dataBuffer);

    return Packet(PacketType::MESSAGE, ""); // Retourner un objet Packet avec les données
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

void Socket::createFileFromPacket(Packet packet, std::string filename) {
    std::string filePath = "";
    if(this->isServer) {
        filePath.append("Storage/").append(filename);
    }
    std::ofstream newFile;
    newFile.open(filePath.c_str(), std::ios_base::binary);
    for (const uint8_t byte : packet.getData()) {
        newFile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
    }
    
    std::cout << "File successfully copied" << std::endl; 
    newFile.close();
}