#include "minigit.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <regex>

// Helper: Validate SHA1 commit hash format
bool isValidCommitHash(const std::string& hash) {
    static const std::regex sha1_regex("^[a-fA-F0-9]{40}$");
    return std::regex_match(hash, sha1_regex);
}

// Print help menu
void showHelp() {
    std::cout << "MiniGit - A minimal Git implementation\n\n"
              << "Usage: minigit <command> [<args>]\n\n"
              << "Commands:\n"
              << "  init                     Initialize new repository\n"
              << "  add <file> [<file2>...]  Add files to staging\n"
              << "  commit  -m \"<message>\" Commit staged changes\n"
              << "  log                      Show commit history\n"
              << "  branch [<name>]          List/create branches\n"
              << "  checkout <branch|commit> Switch branches or checkout commit (detached HEAD)\n"
              << "  merge <branch>           Merge branches\n"
              << "  help                     Show this help\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2 || std::string(argv[1]) == "help") {
        showHelp();
        return argc < 2 ? 1 : 0;
    }

    std::string command = argv[1];

    try {
        // Command: init
        if (command == "init") {
            if (!MiniGit::Initializer::initializeRepository()) {
                return 1;
            }
        }

        //   Command: add
        else if (command == "add") {
            if (argc < 3) {
                std::cerr << "Error: No files specified to add\n";
                return 1;
            }
            bool allSucceeded = true;
            for (int i = 2; i < argc; i++) {
                if (!MiniGit::FileAdder::addFile(argv[i])) {
                    allSucceeded = false;
                }
            }
            if (!allSucceeded) return 1;
        }

        // Command: commit [-a] -m "message"
        else if (command == "commit") {
            std::string message;
            bool messageFound = false;
        
            // Parse commit options
            for (int i = 2; i < argc; ++i) {
                std::string arg = argv[i];
                 if (arg == "-m") {
                    if (i + 1 < argc) {
                        message = argv[++i];
                        messageFound = true;
                    } else {
                        std::cerr << "Error: Commit message required after -m\n";
                        return 1;
                    }
                }
            }
            
            if (!messageFound) {
                std::cerr << "Usage: minigit commit [-a] -m \"message\"\n";
                return 1;
            }


            if (!MiniGit::Committer::commitChanges(message)) {
                std::cerr << "Error: Commit failed.\n";
                return 1;
            }
        }

        // Command: log
        else if (command == "log") {
            MiniGit::Logger::showLog();
        }

        // Command: branch or branch <name>
        else if (command == "branch") {
            if (argc == 2) {
                auto branches = MiniGit::BranchManager::listBranches();
                std::string current = MiniGit::BranchManager::getCurrentBranch();
                for (const auto& branch : branches) {
                    std::cout << (branch == current ? "* " : "  ") << branch << "\n";
                }
            } else if (argc == 3) {
                if (!MiniGit::BranchManager::createBranch(argv[2])) return 1;
            } else {
                std::cerr << "Error: Invalid branch command\n";
                return 1;
            }
        }

        // Command: checkout <branch|commit>
        else if (command == "checkout" && argc == 3) {
            std::string target = argv[2];
            if (MiniGit::BranchManager::branchExists(target)) {
                std::string commitHash = MiniGit::Utils::readFile(MiniGit::Constants::GIT_DIR + "/refs/heads/" + target);
                if (!MiniGit::CheckoutManager::checkoutCommit(commitHash, target)) return 1;
            } else {
                std::string objPath = MiniGit::Constants::GIT_DIR + "/objects/" + target.substr(0, 2) + "/" + target.substr(2);
                if (!std::filesystem::exists(objPath)) {
                    std::cerr << "Error: Commit hash '" << target << "' does not exist\n";
                    return 1;
                }
                if (!MiniGit::CheckoutManager::checkoutCommit(target, "")) {
                    std::cerr << "Error: Failed to checkout commit " << target << "\n";
                    return 1;
                }
                if (!MiniGit::Utils::writeFile(MiniGit::Constants::HEAD_FILE, target)) {
                    std::cerr << "Warning: Failed to update HEAD to detached commit\n";
                }
                std::cout << "HEAD detached at " << target << "\n";
            }
        }

        // Command: merge <branch>
        else if (command == "merge" && argc == 3) {
            if (!MiniGit::MergeManager::mergeBranch(argv[2])) {
                std::cerr << "Merge failed. Resolve conflicts and commit.\n";
                return 2;
            }
        }

        // Unknown command
        else {
            std::cerr << "Error: Unknown command '" << command << "'\n";
            showHelp();
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
