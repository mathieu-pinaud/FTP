#pragma once

#include <stdint.h>
#include <vector>
#include <sys/socket.h>

class Packet {
    private:
        uint8_t packetType;
        std::vector<uint8_t> data;
        uint32_t dataSize;
    
    public:
        Packet(std::vector<uint8_t> bytes) { fromBytes(bytes); }
        Packet(uint8_t packetType, const char* data) : packetType(packetType) { setDataFromStr(data); }
        ~Packet() {};

        std::vector<uint8_t> toBytes();
        void fromBytes(std::vector<uint8_t> bytes);

        uint8_t getPacketType() { return packetType; }
        uint32_t getDataSize() { return dataSize; }
        std::vector<uint8_t> getData() { return data; }

        void setPacketType(const uint8_t packetType) { this->packetType = packetType; }
        void setDataSize(const uint32_t dataSize) { this->dataSize = dataSize; }
        void setData(const std::vector<uint8_t> data) { this->data = data; }
        void setDataFromStr(const char* str);

        void printData();
};