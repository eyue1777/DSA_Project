
#include "minigit.hpp"
#include <iostream>
#include <set>
#include <algorithm>
#include <sstream>

namespace MiniGit {

std::string MergeManager::findCommonAncestor(const std::string& commit1, 
                                             const std::string& commit2) {
    if (commit1.empty() || commit2.empty()) return "";

    std::set<std::string> history1;
    std::string current = commit1;
    while (!current.empty()) {
        if (history1.count(current)) break;
        history1.insert(current);
        current = getParentCommit(current);
    }

    current = commit2;
    while (!current.empty()) {
        if (history1.count(current)) return current;
        current = getParentCommit(current);
        if (current == commit2) break;
    }

    return "";
}

std::unordered_map<std::string, std::string> 
    MergeManager::getCommitFiles(const std::string& commitHash) {
    std::unordered_map<std::string, std::string> files;
    if (commitHash.empty()) return files;

    std::string content = Utils::readBlobContent(commitHash);
    std::istringstream iss(content);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.rfind("file ", 0) == 0) {
            size_t space1 = line.find(' ', 5);
            if (space1 != std::string::npos) {
                std::string filename = line.substr(5, space1 - 5);
                std::string hash = line.substr(space1 + 1);
                files[filename] = hash;
            }
        }
    }
    return files;
}

std::string MergeManager::getParentCommit(const std::string& commitHash) {
    if (commitHash.empty()) return "";

    std::string content = Utils::readBlobContent(commitHash);
    std::istringstream iss(content);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.rfind("parent ", 0) == 0) {
            return line.substr(7);
        }
    }
    return "";
}

namespace {
    void writeConflictFile(const std::string& filename,
                           const std::string& ourContent,
                           const std::string& theirContent,
                           const std::string& branchName) {
        std::ostringstream conflictContent;
        conflictContent << "<<<<<<< HEAD\n" << ourContent
                        << "=======\n" << theirContent
                        << ">>>>>>> " << branchName << "\n";
        Utils::writeFile(filename, conflictContent.str());
    }
}

bool MergeManager::mergeBranch(const std::string& branchName) {
    if (!Initializer::isInitialized()) {
        std::cerr << "Error: Repository not initialized\n";
        return false;
    }

    std::string currentBranch = BranchManager::getCurrentBranch();
    if (currentBranch.empty()) {
        std::cerr << "Error: Not on any branch\n";
        return false;
    }

    if (branchName == currentBranch) {
        std::cerr << "Error: Cannot merge branch into itself\n";
        return false;
    }

    if (!BranchManager::branchExists(branchName)) {
        std::cerr << "Error: Branch '" << branchName << "' does not exist\n";
        return false;
    }

    std::string currentCommit = Utils::readFile(
        Constants::GIT_DIR + "/refs/heads/" + currentBranch);
    std::string otherCommit = Utils::readFile(
        Constants::GIT_DIR + "/refs/heads/" + branchName);

    if (currentCommit.empty()) {
        if (!Utils::writeFile(Constants::GIT_DIR + "/refs/heads/" + currentBranch, otherCommit)) {
            std::cerr << "Error: Failed to update branch reference\n";
            return false;
        }
        std::cout << "Fast-forward merge\n";
        return true;
    }

    std::string lca = findCommonAncestor(currentCommit, otherCommit);
    if (lca.empty()) {
        std::cerr << "Error: No common ancestor found\n";
        return false;
    }

    auto baseFiles = getCommitFiles(lca);
    auto currentFiles = getCommitFiles(currentCommit);
    auto otherFiles = getCommitFiles(otherCommit);

    bool hasConflicts = false;
    std::set<std::string> allFiles;
    std::vector<std::string> modifiedFiles;

    for (const auto& pair : baseFiles) allFiles.insert(pair.first);
    for (const auto& pair : currentFiles) allFiles.insert(pair.first);
    for (const auto& pair : otherFiles) allFiles.insert(pair.first);

    for (const auto& file : allFiles) {
        std::string baseHash = baseFiles.count(file) ? baseFiles[file] : "";
        std::string currentHash = currentFiles.count(file) ? currentFiles[file] : "";
        std::string otherHash = otherFiles.count(file) ? otherFiles[file] : "";

        //  Case 0: File exists only in other branch (new file)
        if (baseHash.empty() && currentHash.empty() && !otherHash.empty()) {
            std::string content = Utils::readBlobContent(otherHash);
            if (!content.empty()) {
                Utils::writeFile(file, content);
                if (FileAdder::addFile(file)) {
                    modifiedFiles.push_back(file);
                }
            }
            continue;
        }

        // Case 1: Unchanged in current, changed in other → take from other
        if (baseHash == currentHash && baseHash != otherHash) {
            std::string content = Utils::readBlobContent(otherHash);
            if (!content.empty()) {
                Utils::writeFile(file, content);
                if (FileAdder::addFile(file)) {
                    modifiedFiles.push_back(file);
                }
            }
            continue;
        }

        // Case 2: Unchanged in other → skip
        if (baseHash == otherHash) {
            continue;
        }

        // Case 3: Changed in both → conflict
        if (currentHash != otherHash) {
            std::string currentContent = Utils::readBlobContent(currentHash);
            std::string otherContent = Utils::readBlobContent(otherHash);

            if (currentContent != otherContent) {
                writeConflictFile(file, currentContent, otherContent, branchName);
                modifiedFiles.push_back(file);
                hasConflicts = true;
                std::cerr << "CONFLICT (content): " << file << "\n";
            }
        }
    }

    for (const auto& file : modifiedFiles) {
        if (!FileAdder::addFile(file)) {
            std::cerr << "Warning: Failed to stage " << file << "\n";
        }
    }

    if (hasConflicts) {
        std::cerr << "Automatic merge failed; fix conflicts and commit the result\n";
        return false;
    }

    std::ostringstream commitContent;
    commitContent << "message Merge branch '" << branchName << "' into " << currentBranch << "\n"
                  << "time " << std::time(nullptr) << "\n"
                  << "parent " << currentCommit << "\n"
                  << "parent " << otherCommit << "\n"
                  << "branch " << currentBranch << "\n";

    auto finalFiles = getCommitFiles(currentCommit);
    auto otherFilesTemp = getCommitFiles(otherCommit);
    finalFiles.insert(otherFilesTemp.begin(), otherFilesTemp.end());

    for (const auto& [file, hash] : finalFiles) {
        commitContent << "file " << file << " " << hash << "\n";
    }

    std::string commitHash = Utils::computeSHA1(commitContent.str());
    if (!Utils::writeObject(commitHash, commitContent.str())) {
        std::cerr << "Error: Failed to create merge commit\n";
        return false;
    }

    if (!Utils::writeFile(Constants::GIT_DIR + "/refs/heads/" + currentBranch, commitHash)) {
        std::cerr << "Error: Failed to update branch reference\n";
        return false;
    }

    std::cout << "Merge made by three-way strategy\n";
    return true;
}

} // namespace MiniGit
