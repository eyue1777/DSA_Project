#include "minigit.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <ctime>

namespace MiniGit {

bool Committer::commitChanges(const std::string& message) {
    if (!Initializer::isInitialized()) {
        std::cerr << "Error: Repository not initialized\n";
        return false;
    }

         
    // Read staged files
    std::unordered_map<std::string, std::string> stagedFiles;
    std::ifstream staging(Constants::STAGING_FILE);
    if (!staging) {
        std::cerr << "Error: Could not open staging area\n";
        return false;
    }

    std::string line;
    while (std::getline(staging, line)) {
        size_t sep = line.find(' ');
        if (sep != std::string::npos) {
            stagedFiles[line.substr(0, sep)] = line.substr(sep + 1);
        }
    }

    if (stagedFiles.empty()) {
        std::cerr << "No changes staged for commit\n";
        return false;
    }

    // Get parent commit
    std::string currentBranch = BranchManager::getCurrentBranch();
    std::string parentCommit = Utils::readFile(
        Constants::GIT_DIR + "/refs/heads/" + currentBranch
    );

    // Create commit object
    std::ostringstream commitContent;
    commitContent << "message " << message << "\n"
                 << "time " << std::time(nullptr) << "\n"
                 << "parent " << parentCommit << "\n"
                 << "branch " << currentBranch << "\n";
    
    for (const auto& [file, hash] : stagedFiles) {
        commitContent << "file " << file << " " << hash << "\n";
    }

    // Store commit
    std::string content = commitContent.str();
    std::string commitHash = Utils::computeSHA1(content);
    if (!Utils::writeObject(commitHash, content)) {
        std::cerr << "Error storing commit\n";
        return false;
    }

    // Update branch reference
    if (!Utils::writeFile(
        Constants::GIT_DIR + "/refs/heads/" + currentBranch,
        commitHash
    )) {
        std::cerr << "Error updating branch reference\n";
        return false;
    }

    // Clear staging
    std::ofstream(Constants::STAGING_FILE, std::ios::trunc).close();

    std::cout << "[" << commitHash.substr(0, 7) << "] " 
              << currentBranch << ": " << message << "\n"
              << " " << stagedFiles.size() << " file(s) committed\n";
    
    return true;
}

} // namespace MiniGit