# 42 Git Workflow with Dual Remotes: GitHub + Vogsphere

Git is a powerful version control system and an essential tool for developers. Its a good idea to get confortable with it early on. 
This document outlines a workflow for managing your Git repositories when working on 42 projects, with a focus on using GitHub alongside Vogsphere (the 42 internal Git hosting service).

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

## Daily Workflow

### At Home

Before working on the `dev` branch, make sure you have the latest updates from `master` (pushed from 42):

```
git switch master
git pull                    # Get the latest clean version
git switch dev
git merge master            # Merge into your local dev branch
```

Then continue working:

```
# make changes
git add ft_isascii.c        # Stage only the file(s) you modified
git commit -m "feat: implement ft_isascii"
git push origin dev
```

### At 42

Pull the latest work you pushed from home:

```
git pull github dev
git switch master
git merge dev
# This brings in all commits made in dev, with their messages
git push origin master        # Push to Vogsphere
git push github master        # Sync GitHub master
```

Now continue working as needed:

```
# make changes
git add ft_strncmp.c
git commit -m "feat: implement ft_strncmp"
# Then push to both remotes:
git push origin master       # Push to Vogsphere
git push github master       # Push to GitHub
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
