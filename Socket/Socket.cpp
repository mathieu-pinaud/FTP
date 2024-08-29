#include "Socket.hpp"

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
    if (send(clientFd, packet.data(), size + 5, 0) == -1) {
        std::cerr << "Failed to send packet" << std::endl;
        return false;
    }
    return true;
}

Packet Socket::receivePacket()
{
    char buffer[1024] = {0};
    ssize_t bytesReceived = recv(this->socketFd, buffer, sizeof(buffer), 0);
    if (bytesReceived <=  0) {
        std::cerr << "Failed to receive packet" << std::endl;
        return Packet(PacketType::MESSAGE, "");
    }
    std::vector<uint8_t> bytes(buffer, buffer + bytesReceived);
    Packet response = Packet(bytes);
    return response;
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