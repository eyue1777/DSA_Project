
   #include "minigit.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace MiniGit {

bool BranchManager::branchExists(const std::string& branchName) {
    if (branchName.empty()) return false;
    std::string branchPath = Constants::GIT_DIR + "/refs/heads/" + branchName;
    return std::filesystem::exists(branchPath);
}

std::vector<std::string> BranchManager::listBranches() {
    std::vector<std::string> branches;
    std::ifstream file(Constants::BRANCHES_FILE);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                branches.push_back(line);
            }
        }
    }
    return branches;
}

std::string BranchManager::getCurrentBranch() {
    try {
        std::string headContent = Utils::readFile(Constants::HEAD_FILE);
        if (headContent.rfind("ref: refs/heads/", 0) == 0) {
            return headContent.substr(16);
        }
        return "";
    } catch (...) {
        return "main";
    }
}

namespace {
    void AddBranchToFile(const std::string& branchName) {
        std::ofstream branches(Constants::BRANCHES_FILE, std::ios::app);
        if (branches) {
            branches << branchName << "\n";
        }
    }
    
    std::string CreateInitialCommit() {
        std::string hash = Utils::generateHash();
        std::string content = "message Initial commit\n";
        content += "time " + std::to_string(std::time(nullptr)) + "\n";
        Utils::writeObject(hash, content);
        return hash;
    }
}

bool BranchManager::createBranch(const std::string& branchName) {
    if (branchName.empty()) {
        std::cerr << "Branch name cannot be empty\n";
        return false;
    }

    if (branchExists(branchName)) {
        std::cerr << "Branch " << branchName << " already exists\n";
        return false;
    }

    std::string currentBranch = getCurrentBranch();
    std::string currentCommit = Utils::readFile(
        Constants::GIT_DIR + "/refs/heads/" + currentBranch
    );

    if (currentCommit.empty()) {
        currentCommit = CreateInitialCommit();
        if (!Utils::writeFile(
            Constants::GIT_DIR + "/refs/heads/" + currentBranch,
            currentCommit
        )) {
            std::cerr << "Failed to initialize branch\n";
            return false;
        }
    }

    if (!Utils::writeFile(
        Constants::GIT_DIR + "/refs/heads/" + branchName,
        currentCommit
    )) {
        std::cerr << "Failed to create branch " << branchName << "\n";
        return false;
    }

    AddBranchToFile(branchName);
    std::cout << "Created branch " << branchName << " from " << currentBranch << "\n";
    return true;
}

bool BranchManager::checkoutBranch(const std::string& branchName) {
    if (branchName.empty()) {
        std::cerr << "Branch name cannot be empty\n";
        return false;
    }

    if (branchName == "main" && !branchExists("main")) {
        std::string initialCommit = CreateInitialCommit();
        if (!Utils::writeFile(Constants::GIT_DIR + "/refs/heads/main", initialCommit) ||
            !Utils::writeFile(Constants::HEAD_FILE, "ref: refs/heads/main")) {
            std::cerr << "Failed to create main branch\n";
            return false;
        }
        AddBranchToFile("main");
        std::cout << "Created and switched to 'main' branch\n";
        return true;
    }

    if (!branchExists(branchName)) {
        std::cerr << "Branch " << branchName << " does not exist\n";
        return false;
    }

    std::string commitHash = Utils::readFile(
        Constants::GIT_DIR + "/refs/heads/" + branchName
    );

    return CheckoutManager::checkoutCommit(commitHash, branchName);
}

} // namespace MiniGit