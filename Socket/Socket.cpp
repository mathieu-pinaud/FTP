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
    char* buffer = (char*)malloc(sizeof(char) * 10000000);
    for (int i = 0; i < size + 15; i++) {
        buffer[i] = packet[i];
    }
    if (send(clientFd, buffer, size + 15, 0) == -1) {
        std::cerr << "Failed to send packet" << std::endl;
        return false;
    }
    return true;
}

Packet Socket::receivePacket(int clientFd)
{   
    // int headerSize = 5;
    // char* headerBuffer = (char*)malloc(sizeof(char) * headerSize);
    PacketHeader packetHeader;
    // if(headerBuffer == 0) {
    //     std::cout << "Error buffer ??" << std::endl;
    // }

    if (clientFd == -42) {
        clientFd = this->socketFd;
    }

    ssize_t headerBytesReceived = recv(clientFd, &packetHeader, sizeof(PacketHeader), 0);
    if (headerBytesReceived <=  0) {
        std::cerr << "Failed to receive packet" << std::endl;
        return Packet(PacketType::MESSAGE, "");
    }
    char *dataBuffer = (char*)malloc(sizeof(char) * packetHeader.size);

    ssize_t dataBytesReceived = recv(clientFd, &dataBuffer, packetHeader.size, 0);



    // char* buffer = (char*)malloc(sizeof(char) * 10000000);
    // //cas client
    // ssize_t bytesReceived = recv(clientFd, buffer, 10000000, 0);

    // std::vector<uint8_t> packet(headerBytesReceived, dataBuffer);
    
    // std::cout << "truc1" << bytesReceived << std::endl;
    // Packet response = Packet(packet);
    std::cout << "truc2" << std::endl;

    // return response;
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