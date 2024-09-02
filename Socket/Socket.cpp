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

bool Socket::sendPacket(int clientFd, Packet message, int size) {
    std::vector<uint8_t> packet = message.toBytes();

    size_t totalBytesToSend = packet.size();
    size_t bytesSent = 0;

    while (bytesSent <= totalBytesToSend) {
        ssize_t result = send(clientFd, packet.data() + bytesSent, totalBytesToSend - bytesSent, 0);
        if (result == -1) {
            std::cerr << "Failed to send packet: " << strerror(errno) << std::endl; // Utilisez strerror pour obtenir la description de l'erreur
            return false;
        }

        if (result == 0) {
            std::cerr << "Connection closed by peer" << std::endl;
            return false;
        }

        bytesSent += result;

        // Debugging statements
        std::cout << "Bytes Sent: " << bytesSent << " / " << totalBytesToSend << std::endl;
    }

    return true;
}

void Socket::receivePacket(int fd) {
    const size_t bufferSize = 999999;
    char buffer[bufferSize] = {0};
    std::vector<uint8_t> packetData;

    ssize_t totalBytesReceived = 0;
    ssize_t bytesReceived = 0;
    uint64_t totalDataSize;
    if(fd == -42){
        fd = this->socketFd;
        totalDataSize = 20;
    }else{
        totalDataSize = 475781945;
    }

    while (true) {
        bytesReceived = recv(fd, buffer, sizeof(buffer), 0);

        if (bytesReceived == 0) {
            std::cerr << "Connection closed by peer" << std::endl;
            close(fd);
        }

        if (bytesReceived < 0) {
            std::cerr << "Failed to receive packet: " << strerror(errno) << std::endl;
            close(fd);
        }

        packetData.insert(packetData.end(), buffer, buffer + bytesReceived);
        totalBytesReceived += bytesReceived;

        std::cout << "Total data received: " << totalBytesReceived << " / " << totalDataSize << std::endl;
        if (totalBytesReceived >= totalDataSize) {
            break;
        }
    }
    // release buffer
    memset(buffer, 0, sizeof(buffer));


    std::cout << "Total bytes received: " << totalBytesReceived << std::endl;
    // Convertir les données en paquet
    Packet response(std::move(packetData));
    response.printData();
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