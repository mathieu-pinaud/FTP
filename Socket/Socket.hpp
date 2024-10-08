#pragma once
#include "../Packet/Packet.hpp"
#include "../Utils/Utils.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


class Socket {
private:
  int socketFd;
  struct sockaddr_in address;
  bool isServer = false;

  Packet acceptClient(char *dataBuffer, uint64_t dataSize,
                      std::string userName);
  Packet password(char *dataBuffer, uint64_t dataSize, std::string userName);
  Packet download(std::string dataString, std::string userString);
  Packet upload(char *dataBuffer, uint64_t dataSize, std::string userName,
                std::string filename);
  Packet deleteFile(std::string userName, std::string filename);
  std::string getPassword();

public:
  Socket() : socketFd(0) { memset(&address, 0, sizeof(address)); }
  ~Socket() { closeSocket(); }

  bool createSocket();
  bool bindSocket(const char *ip, int port);
  bool listenSocket();
  int acceptSocket();
  bool closeSocket();
  bool connectSocket(const char *ip, int port);

  bool sendPacket(int clientFd, Packet message);
  Packet receivePacket(int fd);
  Packet managePacket(char *dataBuffer, uint64_t dataSize, std::string userName,
                      std::string filename, PacketType type);
  Packet connectSocket(char *dataBuffer, uint64_t dataSize,
                       std::string userName);
  Packet removeFolder(std::string userName, std::string path);
  Packet renameFolder(std::string userName, std::string path,
                      std::string folderName);
  Packet createFolder(std::string userName, std::string path,
                      std::string folderName);
  Packet listFolder(std::string userName, std::string path);
  void createFileFromPacket(char *data, std::string filename, ssize_t dataSize,
                            std::string userName = "");

  int getSocketFd() { return socketFd; };
  struct sockaddr_in getAddress() {
    return address;
  };
  bool IsServer() { return isServer; }

  void setSocketFd(int socketFd) { this->socketFd = socketFd; };
  void setAddress(struct sockaddr_in address) { this->address = address; };
  void setIsServer(bool value) { this->isServer = value; }
};
