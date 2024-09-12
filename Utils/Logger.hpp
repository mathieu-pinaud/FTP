#pragma once

#include <memory>
#include <mutex>
#include "../Socket/Socket.hpp"
#include "../Packet/Packet.hpp"

class Logger {

private:
    static Logger* instance;
    static std::mutex mutex;

    std::ofstream file;

    Logger() {};
    ~Logger();

public:
    static Logger* getInstance();

    Logger(const Logger&) = delete;
    void operator=(const Logger&) = delete;

    void log(bool isServer, Packet packet);
    void errLog(std::string message);
};