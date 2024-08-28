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
    return buffer;
}