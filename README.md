# MiniGit - A Lightweight Version Control System

This project is a simplified, command-line version control system written in C++ as part of a DSA assignment. It simulates core Git functionalities including init, add, commit, log, branch, checkout, and merge.

##  Project Structure

- `init.cpp` – Initializes a new repository (.minigit/)
- `add.cpp` – Stages files for commit
- `commit.cpp` – Commits staged changes
- `log.cpp` – Displays commit history
- `branch.cpp` – Manages branches
- `checkout.cpp` – Switches between branches
- `merge.cpp` – Merges branches
- `main.cpp` – Command-line interface entry point
- minigit.hpp - Contains aheader Files
- utils.cpp - Contains Utility functions

##  Features

- Create and manage local repositories
- Add and commit file changes
- Branching and checkout
- Simple merging functionality
- Maintains internal data structures like linked lists and hash maps

## Build Instructions

1. Ensure you have a C++ compiler (e.g., `g++` or Visual Studio)
2. Compile the project:

```bash
g++ -std=c++17 -o minigit main.cpp init.cpp add.cpp commit.cpp log.cpp branch.cpp checkout.cpp merge.cpp
```

3. Run the executable:

```bash
./minigit
```

> On Windows: run `minigit.exe`



##  Requirements

- C++17 or later
- Basic file I/O and command-line knowledge



## 👤 Author


---
