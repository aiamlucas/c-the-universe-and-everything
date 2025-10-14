# Neovim Keybindings Cheat Sheet

---

## LSP Navigation

| Key         | Action                | Description                                         |
| ----------- | --------------------- | --------------------------------------------------- |
| `gd`        | **Go to Definition**  | Jump to where function/variable is defined          |
| `gD`        | Go to Declaration     | Jump to declaration (usually in header file)        |
| `gr`        | Find References       | Show all places this symbol is used                 |
| `gi`        | Go to Implementation  | Jump to implementation                              |
| `gy`        | Go to Type Definition | Jump to type/struct definition                      |
| `K`         | Hover Documentation   | Show function signature/docs (press twice to enter) |
| `<leader>K` | Signature Help        | Show function parameters while typing               |
| `Ctrl+o`    | **Go Back**           | Return to previous location (after jumping)         |
| `Ctrl+i`    | Go Forward            | Opposite of Ctrl+o                                  |

### LSP Code Actions

| Key          | Action           | Description                             |
| ------------ | ---------------- | --------------------------------------- |
| `<leader>ca` | **Code Action**  | Auto-import, quick fixes, refactoring   |
| `<leader>cr` | Rename Symbol    | Rename variable/function everywhere     |
| `<leader>cf` | Format File      | Auto-format current file                |
| `<leader>cF` | Format Selection | Format only selected code (visual mode) |
| `<leader>cd` | Line Diagnostics | Show error details for current line     |
| `<leader>cl` | LSP Info         | Show connected LSP servers              |

### Diagnostics (Errors & Warnings)

| Key          | Action                | Description                       |
| ------------ | --------------------- | --------------------------------- |
| `]d`         | Next Diagnostic       | Jump to next error/warning        |
| `[d`         | Previous Diagnostic   | Jump to previous error/warning    |
| `<leader>xx` | Toggle Trouble        | Show all diagnostics in nice list |
| `<leader>xw` | Workspace Diagnostics | Errors across entire project      |
| `<leader>xd` | Document Diagnostics  | Errors in current file only       |

---

## Telescope (Fuzzy Finder)

| Key          | Action            | Description                        |
| ------------ | ----------------- | ---------------------------------- |
| `;f`         | **Find Files**    | Search files (respects .gitignore) |
| `;r`         | **Live Grep**     | Search text across entire project  |
| `\\`         | List Buffers      | Show all open files                |
| `;t`         | Help Tags         | Search Neovim help                 |
| `;;`         | Resume Search     | Continue last search               |
| `;e`         | Diagnostics       | Search through errors/warnings     |
| `;s`         | Symbols           | Search functions/variables in file |
| `sf`         | File Browser      | Browse files from current location |
| `<leader>ss` | Document Symbols  | Search symbols in current file     |
| `<leader>sS` | Workspace Symbols | Search symbols in entire project   |

**Inside Telescope:**

- `Ctrl+j/k` or up/down arrows - Navigate results
- `Ctrl+/` - Show help/keybindings
- `Enter` - Open file
- `Ctrl+x` - Open in horizontal split
- `Ctrl+v` - Open in vertical split
- `Ctrl+t` - Open in new tab
- `Esc` - Close Telescope

---

## Debugging (DAP)

### Starting & Controlling Debug

| Key     | Action             | Description                           |
| ------- | ------------------ | ------------------------------------- |
| `<F1>`  | **Continue/Start** | Start debugging or continue execution |
| `<F2>`  | Step Into          | Step into function                    |
| `<F3>`  | Step Over          | Execute line, don't enter functions   |
| `<F4>`  | Step Out           | Exit current function                 |
| `<F5>`  | Step Back          | Reverse step (time travel!)           |
| `<F12>` | Restart            | Restart debug session                 |

### Breakpoints & UI

| Key          | Action                 | Description                                |
| ------------ | ---------------------- | ------------------------------------------ |
| `<leader>db` | **Toggle Breakpoint**  | Set/remove breakpoint on current line      |
| `<leader>dB` | Conditional Breakpoint | Breakpoint with condition (e.g., `i == 5`) |
| `<leader>du` | Toggle DAP UI          | Show/hide debug sidebar with variables     |
| `<leader>dt` | Terminate              | Stop debugging                             |
| `<leader>dr` | Toggle REPL            | Open debug console                         |
| `<leader>de` | Eval Expression        | Evaluate variable/expression               |

---

## Neo-tree (File Explorer)

| Key          | Action          | Description          |
| ------------ | --------------- | -------------------- |
| `<leader>e`  | Toggle Explorer | Open/close file tree |
| `<leader>fe` | Toggle Explorer | Same as above        |

**When Neo-tree is open:**

- `a` - Add file/folder
- `d` - Delete
- `r` - Rename
- `c` - Copy
- `x` - Cut
- `p` - Paste
- `y` - Copy name/path
- `Enter` - Open file
- `?` - Show all commands
- `q` - Close Neo-tree

---

## Comments & Text Manipulation

| Key           | Action              | Description                        |
| ------------- | ------------------- | ---------------------------------- |
| `gcc`         | Toggle Comment Line | Comment/uncomment current line     |
| `gc` (visual) | Toggle Comment      | Comment/uncomment selection        |
| `gco`         | Add Comment Below   | Add comment on line below          |
| `gcO`         | Add Comment Above   | Add comment on line above          |
| `<leader>o`   | Clean Line Below    | New line below (no comment prefix) |
| `<leader>O`   | Clean Line Above    | New line above (no comment prefix) |

---

## Editing & Registers

| Key         | Action                | Description                            |
| ----------- | --------------------- | -------------------------------------- |
| `<leader>p` | Paste Last Yank       | Paste from yank register (not delete)  |
| `<leader>P` | Paste Before (Yank)   | Paste before cursor from yank register |
| `x`         | Delete Char (No Yank) | Delete character without yanking       |
| `+`         | Increment Number      | Increment number under cursor          |
| `-`         | Decrement Number      | Decrement number under cursor          |

---

## Window Management

| Key      | Action             | Description                        |
| -------- | ------------------ | ---------------------------------- |
| `<C-h>`  | Go to Left Window  | Navigate to left split/tmux pane   |
| `<C-j>`  | Go to Down Window  | Navigate to bottom split/tmux pane |
| `<C-k>`  | Go to Up Window    | Navigate to top split/tmux pane    |
| `<C-l>`  | Go to Right Window | Navigate to right split/tmux pane  |
| `<C-\>`  | Previous Window    | Go to previous window/pane         |
| `<C-w>s` | Horizontal Split   | Split window horizontally          |
| `<C-w>v` | Vertical Split     | Split window vertically            |
| `<C-w>q` | Close Window       | Close current split                |
| `<C-w>=` | Equal Size         | Make all splits equal size         |

---

## Special Features

### Inlay Hints

| Key         | Action             | Description                       |
| ----------- | ------------------ | --------------------------------- |
| `<leader>i` | Toggle Inlay Hints | Show/hide type annotations inline |

### Git Integration

| Key           | Action            | Description                |
| ------------- | ----------------- | -------------------------- |
| `<leader>gg`  | Open Lazygit      | Full Git UI                |
| `<leader>gb`  | Git Blame         | Show who wrote each line   |
| `<leader>go`  | Open in Browser   | Open file in GitHub/GitLab |
| `]h`          | Next Git Hunk     | Jump to next change        |
| `[h`          | Previous Git Hunk | Jump to previous change    |
| `<leader>ghs` | Stage Hunk        | Stage current change       |
| `<leader>ghr` | Reset Hunk        | Discard current change     |

### Zen Mode

| Key         | Action          | Description                        |
| ----------- | --------------- | ---------------------------------- |
| `<leader>z` | Toggle Zen Mode | Distraction-free fullscreen coding |

### Terminal

| Key          | Action            | Description                      |
| ------------ | ----------------- | -------------------------------- |
| `<C-/>`      | Toggle Terminal   | Open/close terminal at bottom    |
| `<leader>ft` | Floating Terminal | Open terminal in floating window |

### Norminette (42 Berlin)

| Key          | Action         | Description                  |
| ------------ | -------------- | ---------------------------- |
| `<leader>nr` | Run Norminette | Check C code style (42 norm) |

---

## Essential Vim Motions

### Movement

| Key       | Action                |
| --------- | --------------------- |
| `h/j/k/l` | Left/Down/Up/Right    |
| `w`       | Next word start       |
| `b`       | Previous word start   |
| `e`       | Next word end         |
| `0`       | Start of line         |
| `$`       | End of line           |
| `gg`      | First line of file    |
| `G`       | Last line of file     |
| `{`       | Previous paragraph    |
| `}`       | Next paragraph        |
| `Ctrl+d`  | Scroll down half page |
| `Ctrl+u`  | Scroll up half page   |

### Text Objects (Use with d, c, y, v)

| Motion       | Meaning            | Example                               |
| ------------ | ------------------ | ------------------------------------- |
| `iw`         | Inner word         | `ciw` - change word                   |
| `aw`         | Around word        | `daw` - delete word + space           |
| `i"`         | Inside quotes      | `di"` - delete inside "..."           |
| `a"`         | Around quotes      | `da"` - delete "..." including quotes |
| `i(` or `ib` | Inside parentheses | `di(` - delete inside (...)           |
| `a(` or `ab` | Around parentheses | `da(` - delete (...)                  |
| `i{` or `iB` | Inside braces      | `di{` - delete inside {...}           |
| `a{` or `aB` | Around braces      | `da{` - delete {...}                  |
| `it`         | Inside tag         | `dit` - delete inside HTML tag        |
| `at`         | Around tag         | `dat` - delete HTML tag + content     |
| `ip`         | Inner paragraph    | `vip` - select paragraph              |
| `ii`         | Inner indent       | `dii` - delete indented block         |

### Operators

| Key | Action        | Example                   |
| --- | ------------- | ------------------------- |
| `d` | Delete        | `dw` - delete word        |
| `c` | Change        | `ciw` - change inner word |
| `y` | Yank (copy)   | `yy` - yank line          |
| `v` | Visual select | `viw` - select word       |
| `>` | Indent        | `>>` - indent line        |
| `<` | Unindent      | `<<` - unindent line      |

### Combinations (Vim Magic!)

ciw - Change inner word (delete word and enter insert mode)
di" - Delete inside quotes
ya{ - Yank around braces (copy {...} including braces)
dap - Delete around paragraph
vi( - Visually select inside parentheses

> ip - Indent paragraph

---

## Pro Tips

### Common LSP Workflow

1. **See error** -> Cursor on error -> `<leader>ca` (code action) -> auto-fix!
2. **Unknown function** -> Cursor on it -> `K` (hover) -> `gd` (go to def) -> `Ctrl+o` (back)
3. **Rename everywhere** -> Cursor on symbol -> `<leader>cr` -> type new name -> Enter
4. **Find all usages** -> Cursor on function -> `gr` -> see all references

### Debugging Workflow

1. **Set breakpoint** -> `<leader>db` (see red dot)
2. **Start** -> `<F1>` (select "LLDB: Launch Program")
3. **Step through** -> `<F3>` repeatedly
4. **View variables** -> `<leader>du` (open UI)
5. **Inspect variable** -> `<leader>de` -> type variable name
6. **Continue** -> `<F1>` (runs to next breakpoint)
7. **Stop** -> `<leader>dt`

### Search Workflow

1. **Find file** -> `;f` -> type filename
2. **Search text** -> `;r` -> type search term
3. **Recent files** -> `\\` -> select from list

---

## Most Used Commands (Start Here!)

If you're new, focus on these **top 10**:

1. `;f` - Find files
2. `gd` - Go to definition
3. `K` - Show docs
4. `<leader>ca` - Code action (auto-fix)
5. `gcc` - Comment line
6. `;r` - Search text
7. `<leader>e` - File tree
8. `Ctrl+o` - Go back
9. `<leader>cr` - Rename
10. `]d` - Next error

---
