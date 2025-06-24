#include "minigit.hpp"
#include <openssl/sha.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>
#include <filesystem>
#include <iostream>

namespace MiniGit {

// Compute SHA-1 hash of file content (used to identify file versions)
std::string Utils::computeSHA1(const std::string& content) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*)content.c_str(), content.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();
}

// Generate a unique hash (based on time and random value) — used for commit IDs
std::string Utils::generateHash() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(
                   now.time_since_epoch()).count();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1000000);
    
    std::string unique_data = std::to_string(time) + 
                              std::to_string(dis(gen)) + 
                              std::to_string(rand());
    
    return computeSHA1(unique_data);
}

// Read content from a file (returns empty string if file can't be opened)
std::string Utils::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    return {std::istreambuf_iterator<char>(file), 
            std::istreambuf_iterator<char>()};
}

// Write content to a file
bool Utils::writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file) return false;
    file << content;
    return true;
}

// Create a directory (used for .minigit and object folders)
bool Utils::makeDirectory(const std::string& path) {
    try {
        if (std::filesystem::exists(path)) {
            return true; 
        }
        return std::filesystem::create_directories(path);
    } catch (...) {
        return false;
    }
}

// Write blob object to object store (.minigit/objects/XX/...)
bool Utils::writeObject(const std::string& hash, const std::string& content) {
    if (hash.length() != 40) return false;

    std::string objectDir = Constants::OBJECTS_DIR + "/" + hash.substr(0, 2);
    if (!makeDirectory(objectDir)) return false;

    std::string objectPath = objectDir + "/" + hash.substr(2);
    return writeFile(objectPath, content);
}

// Read content of a blob object from object store
std::string Utils::readBlobContent(const std::string& hash) {
    if (hash.empty() || hash.length() != 40) return "";
    std::string path = Constants::OBJECTS_DIR + "/" + 
                       hash.substr(0, 2) + "/" + 
                       hash.substr(2);
    return readFile(path);
}

} // namespace MiniGit
