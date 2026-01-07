# LIT
![License](https://img.shields.io/badge/license-MIT-blue)
![C](https://img.shields.io/badge/language-C-blue)
![Status](https://img.shields.io/badge/status-active-brightgreen)

**Lit** is a lightweight, linear, and localized version control system (VCS) written in native C; inspired by git, 
and simplified for offline development and a **strictly linear commit history per branch**.  

---

## Features

- **Fully Local**: Data is stored in `.lit/`, meaning no network, remotes, or servers are required.
- **Linear History**: Each branch is a chain of commits; no merge commits or DAG complexity.
- **Branching & Rebasing**: Create multiple branches while also keeping your history linear with rebasing.
- **Commit Tracking**: Capture snapshots of your project with a clear message and timestamp.
- **Tags**: Pin important commits in your history with tags.
- **Rollback & Checkout**: Traverse through your commit history safely.
- **Simple CLI**: Similar commands to git for a smooth, focused and easy-to-use workflow.
- **Lightweight**: No external dependencies or libraries required, compiled into a single POSIX compatible executable.
- **Shelving**: Uses per-force style shelving to temporarily store changes on different branches.

---

## Quick Start

Here are some commands to get you started on your local repository.

```bash
    lit init                     # initialize your repository in the current working directory.
    lit add myFile               # add your file to the version control system.
    lit commit                   # commit your changes to the origin branch.
    lit add-branch dev           # create a branch called 'dev' that stems from 'origin' by default.
    lit switch-branch dev        # switch to the branch 'dev'.
    lit modified myFile          # notify lit that you have modified 'myFile'.
    lit commit                   # commit your modified changes to lit.
    lit rebase-branch origin dev # rebase the commits on dev onto origin.
    lit delete-branch dev        # delete the 'dev' branch from the repository (this cannot be undone).
    lit add-tag 7fc... rebase_1  # create a tag called 'rebase_1' for important commits and rebases.
    lit delete-tag rebase_1      # remove the tag called 'rebase_1' (this cannot be undone).
    lit clear-cache              # clear any remaining cache from the repository.
    
    # confused or lost with commands? simply run...
    lit --help
```

Flags can be added to any command to customize its behavior.

```bash
    lit add --all some_directory/               # add all files (recursively) in a directory.
    lit add --no-recurse another_directory/     # add all file in a directory.
    lit commit --m "this is a commit message!"  # you can add a message to your commit.
    lit checkout --hard 7fc...                  # checkout a specific commit, deleting all shelved changes.
    lit ... --quiet                             # suppress all output from lit.
    lit add-branch staging --from dev           # create a new branch called 'staging' that stems from 'dev'.
    lit rollback --tag rebase_1                 # rollback to the tag called 'rebase_1'.
```

---

## Building

**Lit** is written in standard C *(C17)* and is fully self-contained, requiring no external 
dependencies.
This means that all you need to build is simply a **POSIX-compatible** system, a **C compiler** (preferably GCC or Clang), and **Make**.

To build **lit**, simply clone the repository and run make.

```bash
  git clone https://github.com/seanhobeck/lit.git
  cd lit
  sudo make install # this installs the program system-wide in /usr/local/bin
```

---

## Roadmap

There are still many things to add and fix, some of which are listed below.

- Rebuilding / restoring repositories if tampered with during readonly mode.

- Better configuration support (more things other than verbose output).

- Local server for remote repositories (local upstream).

---

## License

Lit is licensed under the MIT License, see [LICENSE.txt](LICENSE) for more information.

---