#include "Utils.hpp"
#include "../Packet/Packet.hpp"
#include <string.h>
#include <algorithm> 
#include <filesystem>

namespace fs = std::filesystem;

std::vector<uint8_t> readFileToUint8Vector(const char* filename, PacketType type, const std::string user) {
    std::ifstream file( filename, std::ios::binary);
    std::string filename_str = filename;
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return {0};
    }

    if (type == PacketType::UPLOAD) {
        
        const std::string prefix = "Storage/";
        
        if (filename_str.rfind(prefix, 0) == 0) {
            filename_str = filename_str.substr(prefix.length());
        }else{
            if (!fs::exists(prefix)) {
                if(fs::create_directory(prefix)) {
                    std::cout << "Directory created: " << prefix << std::endl;
                }
            }
            // verifier que le fichier user existe dans le dossier Storage
            if (!fs::exists(prefix + user)) {
                if(fs::create_directory(prefix + user)) {
                    std::cout << "Directory created: " << prefix + user << std::endl;
                }
            }
        }
    }
    
    //Change filename to vector
    std::vector<uint8_t> filename_vector(filename_str.begin(), filename_str.end());
    size_t filename_size = filename_vector.size();

    // Read file contents into a vector
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    ssize_t size = buffer.size();

    // Create header
    std::vector<uint8_t> header;
    // 1 bit for the type, assuming it's 1
    header.push_back(static_cast<uint8_t>(type));

    // Add the size of the userName (4 bytes)
    std::vector<uint8_t> user_vector(user.begin(), user.end());
    size_t userName_size = user_vector.size();
    for (int i = 3; i >= 0; --i) {
        header.push_back((userName_size >> (4 * i)) & 0xFF);
    }
    // Add the size of the filename (8 bytes)
    for (int i = 7; i >= 0; --i) {
        header.push_back((filename_size >> (8 * i)) & 0xFF);
    }

    // Add the size of the data (8 bytes)
    for (int i = 7; i >= 0; --i) {
        header.push_back((size >> (8 * i)) & 0xFF);
    }
    
    std::vector<uint8_t> result;
    result.reserve(header.size() + userName_size + filename_size + size);
    result.insert(result.end(), header.begin(), header.end());
    result.insert(result.end(), user_vector.begin(), user_vector.end());
    result.insert(result.end(), filename_vector.begin(), filename_vector.end());
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

void addUser(std::string username, std::string password) {
    std::ofstream file;
    file.open("Server/very_safe_trust_me_bro.txt", std::ios_base::app);
    if(!file.is_open()) {
        std::cout << "Could not open file" << std::endl;
        exit(1);
    }

    file << username << ":" << password << std::endl;
    std::cout << "User " << username << " successfully added!" << std::endl;

    file.close();
}

bool isPasswordValid(std::string username, std::string password) {
    std::ifstream file;
    file.open("Server/very_safe_trust_me_bro.txt");
    if(!file.is_open()) {
        std::cout << "Could not open file" << std::endl;
        exit(1);
    }

    std::string line;
    std::vector<std::string> loginInfo;
    while(std::getline(file, line)) {
        loginInfo = split(line, ":");

        trim(loginInfo[0]);
        trim(loginInfo[1]);

        // Si on trouve l'username ET le password sur la même ligne
        if(loginInfo[0] == trim(username)) {
            if(loginInfo[1] == trim(password)) {
                return true;
            }
            return false;
        }

        loginInfo.clear();
    }

    file.close();
    addUser(username, password);
    return true;
}

// trim from start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

inline std::string trim(std::string &s) {
    ltrim(s);
    rtrim(s);
    return s;
}