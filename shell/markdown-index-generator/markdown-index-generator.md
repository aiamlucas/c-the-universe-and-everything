# Markdown Index Generator

This script scans the entire `c-the-universe-and-everything` repository for `.md` files and automatically updates the root README.md with an index of all Markdown topics found across the repository.

## How to Use

1. Make the script executable:

```
chmod +x shell/markdown-index-generator/generate-index.sh
```

2. Run it:

```
./generate-index.sh
```
The script will look two directories up (`../../README.md`) to find the root `README.md` and update it in place, replacing the content between the following markers:

```
<!-- INDEX-START -->
<!-- INDEX-END -->
```
