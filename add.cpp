#include "minigit.hpp"
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>

namespace MiniGit {

// Normalize path relative to current working directory
static std::string normalizePath(const std::string& path) {
    try {
        auto absPath = std::filesystem::absolute(path);
        auto relPath = std::filesystem::relative(absPath);
        return relPath.generic_string();
    } catch (...) {
        return path; // fallback
    }
}

bool FileAdder::addFile(const std::string& filename) {
    if (!Initializer::isInitialized()) {
        std::cerr << "Error: Repository not initialized\n";
        return false;
    }

    std::string normalized = normalizePath(filename);

    // Check if file exists
    if (!std::filesystem::exists(normalized)) {
        std::cerr << "Error: File " << normalized << " does not exist\n";
        return false;
    }

    // Prevent duplicate staging entries
    std::unordered_set<std::string> stagedFiles;
    {
        std::ifstream staging(Constants::STAGING_FILE);
        std::string line;
        while (std::getline(staging, line)) {
            if (!line.empty()) {
                size_t pos = line.find(' ');
                if (pos != std::string::npos) {
                    std::string stagedFile = line.substr(0, pos);
                    stagedFiles.insert(stagedFile);
                }
            }
        }
    }
    if (stagedFiles.count(normalized)) {
        // Already staged
        return true;
    }

    std::string content = Utils::readFile(normalized);
    if (content.empty()) {
        std::cerr << "Warning: File " << normalized << " is empty or cannot be read\n";
    }

    std::string hash = Utils::computeSHA1(content);

    // Write blob object
    if (!Utils::writeObject(hash, content)) {
        std::cerr << "Error: Failed to write object for file " << normalized << "\n";
        return false;
    }

    // Append to staging file: format "filename hash\n"
    std::ofstream staging(Constants::STAGING_FILE, std::ios::app);
    if (!staging) {
        std::cerr << "Error: Could not open staging file\n";
        return false;
    }
    staging << normalized << " " << hash << "\n";

    std::cout << "Added file " << normalized << "\n";
    return true;
}

std::vector<std::string> FileAdder::getStagedFiles() {
    std::vector<std::string> files;
    std::ifstream staging(Constants::STAGING_FILE);
    std::string line;
    while (std::getline(staging, line)) {
        if (!line.empty()) {
            size_t pos = line.find(' ');
            if (pos != std::string::npos) {
                files.push_back(line.substr(0, pos));
            }
        }
    }
    return files;
}


} // namespace MiniGit
