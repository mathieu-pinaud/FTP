#include "Utils.hpp"

// void test(char* filename) {
//     std::ifstream file( &filename, std::ios::binary);
//     std::vector<uint8_t> content;


//     if (!file.is_open()) {
//         std::cerr << "Unable to open file: " << filename << std::endl;
//     }
//     if(myFile.is_open()) {
//         while(myFile.good()) {
//             myFile.binary();
//             std::cout << content << std::endl;
//         }
//     }
// }

std::vector<uint8_t> readFileToUint8Vector(const char* filename) {
    std::ifstream file( filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return {0};
    }

    // Read file contents into a vector
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    ssize_t size = buffer.size();

    // Create header
    std::vector<uint8_t> header;
    uint8_t type = 1; // 1 bit for the type, assuming it's 1
    header.push_back(type);

    // Add the size of the data (4 bytes)
    for (int i = 3; i >= 0; --i) {
        header.push_back((size >> (8 * i)) & 0xFF);
    }
    std::vector<uint8_t> result;
    result.reserve(header.size() + buffer.size());
    result.insert(result.end(), header.begin(), header.end());
    result.insert(result.end(), buffer.begin(), buffer.end());

    return result;
}

std::vector<std::string> split(std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}
