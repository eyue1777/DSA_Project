#include "minigit.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace MiniGit {

bool CheckoutManager::checkoutCommit(const std::string& commitHash, const std::string& branchName) {
    namespace fs = std::filesystem;

    // Update HEAD properly depending on branch or detached HEAD
    if (branchName.empty()) {
        // Detached HEAD — write commit hash directly
        if (!Utils::writeFile(Constants::HEAD_FILE, commitHash)) {
            std::cerr << "Error: Failed to update HEAD to detached commit\n";
            return false;
        }
    } else {
        // Normal branch checkout
        if (!Utils::writeFile(Constants::HEAD_FILE, "ref: refs/heads/" + branchName)) {
            std::cerr << "Error: Failed to update HEAD reference\n";
            return false;
        }
    }

    // Get list of files that should exist in target commit
    std::vector<std::string> commitFiles;
    if (!commitHash.empty()) {
        std::string commitPath = Constants::GIT_DIR + "/objects/" + 
                               commitHash.substr(0, 2) + "/" + 
                               commitHash.substr(2);
        std::string content = Utils::readFile(commitPath);
        std::istringstream iss(content);
        std::string line;
        
        while (std::getline(iss, line)) {
            if (line.substr(0, 5) == "file ") {
                size_t space = line.find(' ', 5);
                if (space != std::string::npos) {
                    commitFiles.push_back(line.substr(5, space - 5));
                }
            }
        }
    }

    // Clean working directory (remove files not in target commit)
    try {
        for (const auto& entry : fs::directory_iterator(".")) {
            const auto& path = entry.path();
            std::string filename = path.filename().string();
            
            // Skip repository and development files
            if (filename == Constants::GIT_DIR || 
                filename == ".git" ||
                filename == "minigit.exe" ||
                path.extension() == ".cpp" || 
                path.extension() == ".hpp" ||
                filename == ".gitignore") {
                continue;
            }
            
            // Remove if not in target commit
            if (std::find(commitFiles.begin(), commitFiles.end(), filename) == commitFiles.end()) {
                fs::remove_all(path);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Could not clean working directory - " << e.what() << "\n";
    }

    // Restore files from commit
    if (commitHash.empty()) {
        std::cout << "Switched to branch '" << branchName << "'\n";
        return true;
    }

    std::string commitPath = Constants::GIT_DIR + "/objects/" + 
                           commitHash.substr(0, 2) + "/" + 
                           commitHash.substr(2);
    std::string content = Utils::readFile(commitPath);
    std::istringstream iss(content);
    std::string line;
    bool success = true;

    while (std::getline(iss, line)) {
        if (line.substr(0, 5) == "file ") {
            size_t space = line.find(' ', 5);
            if (space == std::string::npos) continue;

            std::string filename = line.substr(5, space - 5);
            std::string blobHash = line.substr(space + 1);

            try {
                std::string blobPath = Constants::GIT_DIR + "/objects/" + 
                                     blobHash.substr(0, 2) + "/" + 
                                     blobHash.substr(2);
                std::string fileContent = Utils::readFile(blobPath);
                
                fs::path parent = fs::path(filename).parent_path();
                if (!parent.empty()) {
                    fs::create_directories(parent);
                }

                if (!Utils::writeFile(filename, fileContent)) {
                    std::cerr << "Warning: Failed to write file " << filename << "\n";
                    success = false;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error restoring file " << filename 
                          << ": " << e.what() << "\n";
                success = false;
            }
        }
    }

    if (success) {
        if (branchName.empty()) {
            std::cout << "HEAD detached at " << commitHash << "\n";
        } else {
            std::cout << "Switched to branch '" << branchName << "'\n";
        }
    }
    return success;
}

} // namespace MiniGit
