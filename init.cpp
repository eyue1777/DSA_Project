#include "minigit.hpp"
#include <iostream>

namespace MiniGit {

bool Initializer::initializeRepository() {
    namespace fs = std::filesystem;
    try {
        // Create directory structure
        fs::create_directory(Constants::GIT_DIR);
        fs::create_directory(Constants::GIT_DIR + "/objects");
        fs::create_directory(Constants::GIT_DIR + "/refs");
        fs::create_directory(Constants::GIT_DIR + "/refs/heads");

        // Create empty staging file
        if (!Utils::writeFile(Constants::STAGING_FILE, "")) {
            throw std::runtime_error("Failed to create staging file");
        }

        // Initialize with proper initial commit
        std::string initialCommit = Utils::generateHash();
        std::string commitContent = "message Initial commit\n";
        commitContent += "time " + std::to_string(std::time(nullptr)) + "\n";
        Utils::writeObject(initialCommit, commitContent);
        
        // Set up branch references
        Utils::writeFile(Constants::GIT_DIR + "/refs/heads/main", initialCommit);
        Utils::writeFile(Constants::BRANCHES_FILE, "main\n");
        Utils::writeFile(Constants::HEAD_FILE, "ref: refs/heads/main");

        std::cout << "Initialized empty MiniGit repository with 'main' branch\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize repository: " << e.what() << "\n";
        return false;
    }
}

bool Initializer::isInitialized() {
    return std::filesystem::exists(Constants::GIT_DIR) &&
           std::filesystem::exists(Constants::HEAD_FILE) &&
           std::filesystem::exists(Constants::STAGING_FILE) &&
           std::filesystem::exists(Constants::GIT_DIR + "/refs/heads");
}
} // namespace MiniGit