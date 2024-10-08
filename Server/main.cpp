#include "../Socket/Socket.hpp"
#include "../Utils/Logger.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <ifaddrs.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

std::string getServerAddress() {
  struct ifaddrs *ifAddrStruct = NULL;
  struct ifaddrs *ifa = NULL;
  void *tmpAddrPtr = NULL;
  std::string address;

  // Get the list of network interfaces
  getifaddrs(&ifAddrStruct);

  // Iterate through the network interfaces
  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    // Check if it is an IPv4 address and not a loopback address
    if (ifa->ifa_addr->sa_family == AF_INET &&
        strcmp(ifa->ifa_name, "lo") != 0) {
      tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      address = addressBuffer;

      // Avoid WSL internal network addresses (e.g., those starting with "172.")
      if (address.find("172.") != 0) {
        break;
      }
    }
  }

  // Free the memory allocated for network interfaces
  if (ifAddrStruct != NULL) {
    freeifaddrs(ifAddrStruct);
  }

  // Return the found address or an empty string if none found
  return address;
}

int handleClient(int clientFd, Socket &server) {
  if (clientFd == -1) {
    return 1;
  }
  std::cout << "Client connected" << std::endl;
  Packet sent = server.receivePacket(clientFd);
  server.sendPacket(clientFd, sent);
  sent = server.receivePacket(clientFd);
  server.sendPacket(clientFd, sent);
  if (sent.getDataStr() == std::string("Connexion refusé")) {
    return 1;
  }
  while (true) {
    Packet received = server.receivePacket(clientFd);
    server.sendPacket(clientFd, received);
    if (received.getPacketType() == PacketType::MESSAGE) {
      break;
    }
  }

  return 0;
}

int startServer(std::string ip, int port) {
  Socket server;
  server.setIsServer(true);
  if (server.createSocket() == false) {
    return 1;
  }
  if (server.bindSocket(ip.c_str(), port) == false) {
    return 1;
  }
  if (server.listenSocket() == false) {
    return 1;
  }
  std::cout << "Server started on " << ip << ":" << port << std::endl;

  std::vector<std::thread> clientThreads;

  while (true) {
    int clientFd = server.acceptSocket();
    if (clientFd == -1) {
      return 1;
    }
    clientThreads.emplace_back(
        std::thread(handleClient, clientFd, std::ref(server)));
  }

  for (std::thread &t : clientThreads) {
    if (t.joinable()) {
      t.join();
    }
  }

  return 0;
}

int main(int ac, char **av) {

  if (ac != 2) {
    std::cerr << "Usage: " << av[0] << "<port>" << std::endl;
    return 1;
  }
  int port = std::stoi(av[1]);
  std::string ip = getServerAddress();
  if (empty(ip)) {
    std::cerr << "Error: getIp" << std::endl;
    return 1;
  } else {
    std::cout << "Server IP: " << ip << std::endl;
  }

  return startServer(ip, port);
}