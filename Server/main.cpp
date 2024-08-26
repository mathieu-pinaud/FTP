#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <cstring>

std::string getServerAddress() {
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;
    std::string address;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        // check it is IP4 and not a loopback address
        if (ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, "lo") != 0) { 
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            address = addressBuffer;
            break;
        }
    }
    if (ifAddrStruct != NULL) {
        freeifaddrs(ifAddrStruct);
        return address;
    }
    return NULL;
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
    return 0;
}