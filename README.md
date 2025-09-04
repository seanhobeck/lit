# LIT
**LIT** is a lightweight, linear, and localized version control system (VCS) written in native C; inspired by git, 
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