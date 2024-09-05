#pragma once 
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include "../Packet/Packet.hpp"
#include "../Utils/Utils.hpp"

class Socket
{
private:
    int socketFd;
    struct sockaddr_in address;
    bool isServer = false;

    Packet connectSocket(char *dataBuffer, uint64_t dataSize, std::string userName);
    Packet password(char *dataBuffer, uint64_t dataSize, std::string userName);
    Packet download(std::string dataString,std::string userString);
    Packet upload(char *dataBuffer, uint64_t dataSize, std::string userName, std::string filename);
    Packet deleteFile(std::string userName, std::string filename);
    std::string getPassword();
    
public:
    Socket(): socketFd(0) {memset(&address, 0, sizeof(address));}
    ~Socket() {closeSocket();}

    bool createSocket();
    bool bindSocket(const char* ip, int port);
    bool listenSocket();
    int acceptSocket();
    bool closeSocket();
    bool connectSocket(const char* ip, int port);
    
    bool sendPacket(int clientFd, Packet message);
    Packet receivePacket(int fd);
    Packet managePacket(char *dataBuffer, uint64_t dataSize, std::string userName, std::string filename, PacketType type);

    void createFileFromPacket(char *data, std::string filename, ssize_t dataSize, std::string userName="");

    int getSocketFd() {return socketFd;};
    struct sockaddr_in getAddress() {return address;};
    bool IsServer() { return isServer; }

    void setSocketFd(int socketFd) {this->socketFd = socketFd;};
    void setAddress(struct sockaddr_in address) {this->address = address;};
    void setIsServer(bool value) { this->isServer = value; }
};
