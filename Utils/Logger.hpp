#pragma once

#include "../Socket/Socket.hpp"
#include "../Packet/Packet.hpp"

class Logger {

private:
    static std::string getCurrentTime();

public:
    static void printLog(Socket socket, Packet packet);

};