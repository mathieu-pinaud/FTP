#include "Logger.hpp"

#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>


/**
 * Ecrit un message dans le format suivant :
 * 
 * <temps au format h:m:s> [SERVER/CLIENT] <description>
 */
void Logger::printLog(Socket socket, Packet packet) {
    std::fstream file;
    file.open(".log", std::ios_base::app);
    if(!file.is_open()) {
        std::cerr << "Error while opening log file" << std::endl;
    }

    std::string message;
    message += Logger::getCurrentTime();
    message += socket.IsServer() ? " [SERVER] " : " [CLIENT] ";

    message += "à envoyé un packet de " + std::to_string(packet.getDataSize()) + " bytes de type " + Packet::packetTypeToString(packet.getPacketType());

    file << message << std::endl;

    file.close();
}

void Logger::printErrorLog(std::string message) {
    std::fstream file;
    file.open(".log", std::ios_base::app);
    if(!file.is_open()) {
        std::cerr << "Error while opening log file" << std::endl;
    }

    std::string msg;
    msg += Logger::getCurrentTime();
    msg += " [ERROR!] ";
    msg += message;
    file << msg << std::endl;
    
    file.close();
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&currentTime);
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%H:%M:%S");
    
    return oss.str();
}