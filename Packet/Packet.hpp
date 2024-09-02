#pragma once

#include <stdint.h>
#include <vector>
#include <sys/socket.h>

enum TransferAction : uint8_t {
    NONE,
    UPLOAD,
    DOWNLOAD
};

enum class PacketType : uint8_t {
    MESSAGE,
    PASSWORD,
    FILE
};

class Packet {
    private:
        enum PacketType packetType;
        uint64_t dataSize;
        uint64_t filenameSize;
        std::vector<uint8_t> filename;
        std::vector<uint8_t> data;
    
    public:
        Packet(std::vector<uint8_t> bytes) { fromBytes(bytes); }
        Packet(PacketType packetType, const char* data) : packetType(packetType) { setDataFromStr(data); }
        ~Packet() {};

        std::vector<uint8_t> toBytes();
        void fromBytes(std::vector<uint8_t> bytes);

        PacketType getPacketType() { return packetType; }
        uint32_t getDataSize() { return dataSize; }
        std::vector<uint8_t> getData() { return data; }

        void setPacketType(const PacketType packetType) { this->packetType = packetType; }
        void setDataSize(const uint32_t dataSize) { this->dataSize = dataSize; }
        void setData(const std::vector<uint8_t> data) { this->data = data; }
        void setDataFromStr(const char* str);

        void printData();
};

struct PacketHeader {
    PacketType type;
    uint64_t filenameSize;
    uint64_t size;    
}__attribute((packed));