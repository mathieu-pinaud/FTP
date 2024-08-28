#include <iostream>
#include <string>
#include <regex>
#include "../Socket/Socket.hpp"

const std::string *splitIpPort(char *ipPortString) {
    std::string arg = ipPortString;
    size_t pos = arg.find(":");
    if (pos == std::string::npos) {
        std::cerr << "Bad formating for <ip:port>" << std::endl;
        return NULL;
    }
    std::string ip = arg.substr(0, pos);
    std::string port = arg.substr(pos + 1);
    std::string ip_regex = "^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$";
    std::string port_regex = "^[0-9]{1,5}$";
    if (std::regex_match(ip, std::regex(ip_regex)) == false) {
        std::cerr << "Invalid ip address" << std::endl;
        return NULL;
    }
    if (std::regex_match(port, std::regex(port_regex)) == false) {
        std::cerr << "Invalid port" << std::endl;
        return NULL;
    }
    std::string *args = new std::string[2];
    args[0] = ip;
    args[1] = port;
    return args;
}

bool check_args(int ac, char **av) {
    if (ac != 2) {
        std::cerr << "Usage: " << av[0] << " <ip:port>" << std::endl;
        return false;
    }
    return true;
}

int startClient(std::string ip, int port) {
    Socket client;
    if (client.createSocket() == false) {
        return 1;
    }
    if (client.connectSocket(ip.c_str(), port) == false) {
        return 1;
    }
    std::cout << "Client started on " << ip << ":" << port << std::endl;
    Packet response = client.receivePacket();
    response.printData();
    return 0;
}

int main(int ac, char** av) {
    if (check_args(ac, av) == false) {
        return 1;
    }
    const std::string *ipPort = splitIpPort(av[1]);
    if (ipPort == NULL) {
        return 1;
    }
    std::cout << "IP: " << ipPort[0] << std::endl;
    std::cout << "Port: " << ipPort[1] << std::endl;

    return startClient(ipPort[0], std::stoi(ipPort[1]));
}