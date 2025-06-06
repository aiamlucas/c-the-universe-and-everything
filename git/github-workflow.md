# 42 Git Workflow with Dual Remotes: GitHub + Vogsphere

Git is a version control system and an essential tool for developers. Its a good idea to get confortable with it early on. 
This document outlines a workflow for managing your Git repositories when working on 42 projects, with a focus on using GitHub alongside Vogsphere (the 42 internal Git hosting service).

If you're interested in the background of Git and why it works the way it does, [this talk by Linus Torvalds](https://www.youtube.com/watch?v=idLyobOhtO4) offers a clear explanation from the creator himself.

## Important: GitHub and 42 Project Policy

Before making any 42 project public on GitHub, check your campus’s academic integrity policies to understand what you are allowed to share.

When in doubt, keep the repository private and only include your own code.

## Project Structure

```
~/code/
├── libft/            # Clean project: pushed to both GitHub and Vogsphere
└── libft-tests/      # Personal testing repo (only on GitHub), with mains, debug files, experiments
```

## Initial Setup (once per project)

### Inside the 42 project folder (e.g., libft/)

```
cd ~/code/libft
git init
```

### Add remotes

```
git remote add origin git@vogsphere.42yourcampus.de:vogsphere/intra-uuid-your-id
git remote add github git@github.com:youruser/libft.git
```

### Check remotes

```
git remote -v
```

Expected output:

```
origin  git@vogsphere.42yourcampus.de:... (fetch)
origin  git@vogsphere.42yourcampus.de:... (push)
github  git@github.com:youruser/libft.git (fetch)
github  git@github.com:youruser/libft.git (push)
```

## Branch Strategy

- master: clean, submission-ready; updated only at 42
- dev: working branch; used at home, pushed to GitHub
- libft-tests/: separate repo for test mains, notes, debug tools — not pushed to Vogsphere

## Home workflow (on dev branch)

Before working on the `dev` branch, make sure you have the latest updates from `master` (pushed from 42):

```
git switch master 
git pull github master     // to get the latest clean state
```

```
git switch dev
git rebase master          // rebase dev on top of latest master
```

Then continue working:

```
git add ft_isascii.c       // stage only the file(s) you modified
git commit -m "feat: implement ft_isascii"     // commit with a clear message
```

Push work to GitHub dev branch

```
git push github dev
```

## 42 Campus workflow (update master from dev)

Pull dev branch from GitHub and fast-forward merge

```
git fetch github           // fetch latest changes from GitHub
git checkout master
git merge --ff-only github/dev     // fast-forward master to match dev (no merge commit)
```

Push updated master to both remotes

```
git push origin master
git push github master
```

Or, if you've defined the alias (for pushing to both remotes at the same time):

```
gittudo
```

## Commit Message Standards

Follow a clear, consistent structure for your commit messages. This helps both you and others understand the history of the project at a glance.

### Basic Format

```
<type>: <short summary>

[optional body explaining the change in more detail]
```

### Common Types

- ``` feat ```: New feature or function implementation  
- ``` fix ```: Bug fix or logic correction  
- ``` refactor ```: Code cleanup or reorganization (no behavior change)  
- ``` style ```: Formatting, whitespace, comments, Norminette fixes  
- ``` test ```: Add or update test files  
- ``` docs ```: Documentation changes (README, code comments, etc.)  
- ``` chore ```: Routine changes like config updates or project setup

### Examples

```
feat: implement ft_isprint

Added ft_isprint function to check if a character is printable,
based on ASCII values ranging from 32 to 126.
```

```
fix: handle NULL pointers in ft_strrchr

Returned early if s is NULL to prevent segmentation fault.
```

```
style: fix Norminette issues in ft_strlen

Reindented function and removed unnecessary blank lines.
```

```
refactor: extract helper function in ft_isalpha
```

```
docs: update README with workflow instructions
```

### Tips

- Use the imperative mood (e.g., "add", not "added" or "adds").
- Keep the first line under 50 characters.
- Leave a blank line between the summary and the body (if you include a body).
- You don’t need a body for small or obvious commits.
- Keep commits focused — don’t mix unrelated changes in one commit.
- For multi-line commit messages, it's best to run `git commit` without `-m`.
That opens your editor so you can format the summary and body clearly.

## Git Alias: `gittudo`

The `gittudo` alias is a shortcut to push the `master` branch to **both Vogsphere and GitHub** in a single command.

### Why use it?

- Prevents forgetting to push to one remote
- Saves time when submitting from 42
- Keeps GitHub and Vogsphere in sync

### How to define it (on your 42 computer):

#### For Zsh

Add this to your `~/.zshrc`:

```
alias gittudo='git push origin master && git push github master'
```

Then reload your shell config:

```
source ~/.zshrc
```

### For Bash

Add the same line to your `~/.bashrc`:

```
alias gittudo='git push origin master && git push github master'
```

Then reload your Bash config:

```
source ~/.bashrc
```

### Usage

Once defined, you can run:

```
gittudo
```

This will:

1. Push the `master` branch to your Vogsphere remote (`origin`)
2. Push the same branch to GitHub (`github`)

Make sure both remotes are correctly configured before using it. You can check with:

```
git remote -v
```

# Git Merges Explained: Regular vs Fast-Forward vs Rebase

To keep a clean and readable history, this workflow uses fast-forward merges instead of regular merges. Combined with rebase in your `dev` branch, this avoids unnecessary merge commits while keeping all your changes and commit messages intact.

## Regular Merge

A **regular merge** happens when two branches have diverged.

```
git merge dev
```

This creates a **merge commit**, like this:

      A---B---C  (master)
       \     \
        D---E  (dev)
             \
              M  ← merge commit

You now have a new commit `M` that combines the histories.  
**Downside**: These merge commits can clutter your history, especially when working solo.

---

## Fast-Forward Merge

A **fast-forward merge** happens when the current branch has no unique commits, it can just "move forward" to match the target branch.

```
git merge --ff-only github/dev
```

Before:
      A---B  (master)
           \
            C---D  (github/dev)

After:
      A---B---C---D  (master = github/dev)

No merge commit is created — just a straight history.

**Benefit**: Cleaner, linear history. This is ideal when working alone and syncing dev → master.

---

## Rebase (used in `dev` to stay up to date with `master`)

```
git switch dev
git rebase master
```

This rewrites the `dev` branch so that your new work sits "on top of" the latest `master`.

Before:
      A---B  (master)
       \
        C---D  (dev)

After rebase:
      A---B---C'---D'  (dev)

Commits `C` and `D` are reapplied as new commits `C'` and `D'`.

**Benefit**: Keeps your `dev` history up to date without introducing a merge commit.

---

## Summary

| Method                 | Merge Commit?   | History Clean? | Ideal Use                        |
|------------------------|-----------------|----------------|----------------------------------|
| `git merge`            |    Yes          |    No          | Teams, complex merges            |
| `git merge --ff-only`  |    No           |    Yes         | Syncing clean branches           |
| `git rebase`           |    No           |    Yes         | Keeping `dev` on top of `master` |

Use **`rebase` in `dev`**, then **`--ff-only` merge into `master`** — for a clean commit history.
