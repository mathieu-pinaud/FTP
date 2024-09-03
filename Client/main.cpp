#include <iostream>
#include <string>
#include <cstring>
#include <regex>
#include "../Socket/Socket.hpp"
#include "../Utils/Utils.hpp"

const std::string *splitIdentification(char *ipPortString) {
    std::string arg = ipPortString;
    size_t posUser = arg.find("@");
    size_t pos = arg.find(":");
    if (pos == std::string::npos || posUser == std::string::npos) {
        std::cerr << "Bad formating for user@<ip:port>" << std::endl;
        return NULL;
    }
    std::string user = arg.substr(0, posUser);
    std::string ip = arg.substr(posUser + 1, pos- posUser - 1);
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
    std::string *args = new std::string[3];
    args[0] = ip;
    args[1] = port;
    args[2] = user;
    return args;
}

enum TransferAction check_args(int ac, char **av) {
    if (ac != 4) {
        std::cerr << "Usage: " << av[0] << " <ip:port> -upload/-download <filename>" << std::endl;
        return NONE;
    }

    if(std::strcmp(av[2], "-upload") == 0) {
        return UPLOAD;
    }
    
    if(std::strcmp(av[2], "-download") == 0) {
        return DOWNLOAD;
    }

    if(std::strcmp(av[2], "-delete") == 0) {
        return DELETE;
    }
    
    std::cerr << "Usage: " << av[0] << " user@<ip:port> -upload/-download <filename>" << std::endl;
    return NONE;
}

int startClient(std::string ip, int port, Packet& p) {
    Socket client;
    if (client.createSocket() == false) {
        return 1;
    }
    if (client.connectSocket(ip.c_str(), port) == false) {
        return 1;
    }
    //convertit packet username en string
    std::vector<uint8_t> usernameVector = p.getUserName();
    std::string username(usernameVector.begin(), usernameVector.end());
    
    std::cout << username.c_str() <<" started on " << ip << ":" << port << std::endl;
    client.sendPacket(client.getSocketFd(), p);
    if (p.getPacketType() == PacketType::DOWNLOAD) {
        Packet received = client.receivePacket(client.getSocketFd());
    }
    return 0;
}

int main(int ac, char** av) {
    TransferAction action = check_args(ac, av);
    if (action == NONE) {
        return 1;
    }
    const std::string *identification = splitIdentification(av[1]);
    if (identification == NULL) {
        return 1;
    }
    Packet p = Packet(PacketType::MESSAGE, "","");
    if(action == UPLOAD) {
        p = Packet(readFileToUint8Vector(av[3], PacketType::UPLOAD, identification[2].c_str()));
    }
    
    if(action == DOWNLOAD) {
       p= Packet(PacketType::DOWNLOAD, av[3], identification[2].c_str());
    }

    return startClient(identification[0], std::stoi(identification[1]), p);
}