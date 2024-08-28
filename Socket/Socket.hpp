#pragma once 
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include "../Packet/Packet.hpp"

class Socket
{
private:
    int socketFd;
    struct sockaddr_in address;

public:
    Socket(): socketFd(0) {memset(&address, 0, sizeof(address));};
    ~Socket() {closeSocket();};

    bool createSocket();
    bool bindSocket(const char* ip, int port);
    bool listenSocket();
    int acceptSocket();
    bool closeSocket();
    bool connectSocket(const char* ip, int port);
    
    bool sendPacket(int clientFd, Packet message, int size);
    Packet receivePacket();

    int getSocketFd() {return socketFd;};
    struct sockaddr_in getAddress() {return address;};

    void setSocketFd(int socketFd) {this->socketFd = socketFd;};
    void setAddress(struct sockaddr_in address) {this->address = address;};

};
