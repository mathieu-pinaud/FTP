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
    // Ouvrir le fichier en mode binaire
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return {};
    }

    // Lire le contenu du fichier dans un vecteur
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Obtenir la taille des données lues
    uint64_t size = buffer.size(); // Utiliser uint64_t pour garantir 8 octets

    // Créer l'en-tête
    std::vector<uint8_t> header;
    uint8_t type = 2;  // Définir le type, supposons 2 (vous pouvez ajuster selon le besoin)
    header.push_back(type);

    // Ajouter la taille des données (8 octets)
    for (int i = 7; i >= 0; --i) {
        header.push_back((size >> (8 * i)) & 0xFF);
    }

    // Créer le vecteur de résultat en réservant suffisamment d'espace
    std::vector<uint8_t> result;
    result.reserve(header.size() + buffer.size());

    // Insérer l'en-tête et le buffer dans le résultat
    result.insert(result.end(), header.begin(), header.end());
    result.insert(result.end(), buffer.begin(), buffer.end());

    // Vérification de la correspondance avec le fichier original
    std::ifstream verifyFile(filename, std::ios::binary);
    if (!verifyFile.is_open()) {
        std::cerr << "Unable to open file for verification: " << filename << std::endl;
        return {};
    }

    std::vector<uint8_t> originalData((std::istreambuf_iterator<char>(verifyFile)), std::istreambuf_iterator<char>());
    verifyFile.close();

    // Comparer les données originales avec le buffer
    if (buffer != originalData) {
        std::cerr << "Data mismatch: The data read does not match the original file content!" << std::endl;
        return {};
    }

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
