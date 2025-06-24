#include "minigit.hpp"
#include <iostream>
#include <fstream>

namespace MiniGit {

// Display commit history
void Logger::showLog() {
    std::string currentBranch = BranchManager::getCurrentBranch();
    std::string currentCommit = Utils::readFile(Constants::GIT_DIR + "/refs/heads/" + currentBranch);

    if (currentCommit.empty()) {
        std::cout << "No commits yet\n";
        return;
    }

    // Walk commit history backwards
    while (!currentCommit.empty()) {
        std::string commitPath = Constants::GIT_DIR + "/objects/" + 
                               currentCommit.substr(0, 2) + "/" + 
                               currentCommit.substr(2);
        
        // Read commit metadata
        std::string content = Utils::readFile(commitPath);
        std::istringstream iss(content);
        std::string line;
        std::string message, timestamp, parent;

        while (std::getline(iss, line)) {
            if (line.substr(0, 8) == "message ") {
                message = line.substr(8);
                if (message.find("Merge branch") != std::string::npos) {
                    message += " (merge)";
                }
            }
            else if (line.substr(0, 5) == "time ") timestamp = line.substr(5);
            else if (line.substr(0, 7) == "parent ") parent = line.substr(7);
        }

    
        if (timestamp.empty()) {
            std::cerr << "Warning: Missing timestamp in commit " << currentCommit << "\n";
            timestamp = "0"; // default to epoch time if missing
        }
        

        // Format timestamp
        std::time_t time = std::stol(timestamp);
        char timeStr[100];
        std::strftime(timeStr, sizeof(timeStr), "%c", std::localtime(&time));

        // Print commit info
        std::cout << "commit " << currentCommit << "\n"
                  << "Date:   " << timeStr << "\n"
                  << "\n    " << message << "\n\n";

        currentCommit = parent; // Move to parent commit
    }
}
} // namespace MiniGit