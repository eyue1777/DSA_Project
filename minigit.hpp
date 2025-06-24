
#ifndef MINIGIT_HPP
#define MINIGIT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <ctime>

namespace MiniGit {

// Constants defining repo directory and key files
namespace Constants {
    const std::string GIT_DIR = ".minigit";
    const std::string STAGING_FILE = ".minigit/staging";
    const std::string HEAD_FILE = ".minigit/HEAD";
    const std::string BRANCHES_FILE = ".minigit/branches";  
    const std::string OBJECTS_DIR = ".minigit/objects";
}

// Class to initialize repository and check if initialized
class Initializer {
public:
    static bool initializeRepository();
    static bool isInitialized();
};

// Class to handle adding files to staging area
class FileAdder {
public:
    static bool addFile(const std::string& filename);
    static std::vector<std::string> getStagedFiles();
};

// Class to handle committing staged changes with messages
class Committer {
public:
    static bool commitChanges(const std::string& message);
    static bool commit(const std::string& message, bool checkStaged = true);
};

// Class to display commit logs/history
class Logger {
public:
    static void showLog();
};

// Class to manage branches: create, delete, list, checkout, check existence
class BranchManager {
public:
    static bool createBranch(const std::string& branchName);
    static bool deleteBranch(const std::string& branchName);
    static std::vector<std::string> listBranches();
    static std::string getCurrentBranch();
    static bool checkoutBranch(const std::string& branchName);
    static bool branchExists(const std::string& branchName);
};

// Class to checkout commits or branches
class CheckoutManager {
public:
    static bool checkoutCommit(const std::string& commitHash, const std::string& branchName);
};

// Class to manage merging branches with a three-way merge strategy
class MergeManager {
public:
    static bool mergeBranch(const std::string& branchName);
    static std::unordered_map<std::string, std::string> getCommitFiles(const std::string& commitHash);
private:
    static std::string findCommonAncestor(const std::string& commit1, const std::string& commit2);
    static std::string getParentCommit(const std::string& commitHash);
};


// Utility functions used by MiniGit system
namespace Utils {
    std::string computeSHA1(const std::string& content);
    std::string generateHash();
    std::string readFile(const std::string& path);
    bool writeFile(const std::string& path, const std::string& content);
    bool makeDirectory(const std::string& path);
    bool writeObject(const std::string& hash, const std::string& content);
    std::string readBlobContent(const std::string& hash);
}

} // namespace MiniGit

#endif // MINIGIT_HPP
