# ✨ Verve

A minimal terminal text editor written in C.

Verve was created as a low-level systems programming project focused on:

* dynamic memory allocation
* terminal rendering
* text editor architecture
* buffer management
* UTF-8 handling
* manual data structures
* ANSI terminal control

The project is inspired by editors such as:

* Nano
* Kilo
* Vim
* Micro

---

# 🚀 Features

## ✍️ Text Editing

* Text insertion and deletion
* Multiline editing
* Automatic line merging
* Smart Backspace/Delete behavior
* Cursor navigation
* Home / End support
* Page Up / Page Down support
* Automatic indentation
* Smart TAB handling (4 spaces)

---

## 📁 File Management

* Open files
* Save files
* Vertical scrolling
* Horizontal scrolling

---

## 🔎 Search

* Incremental search
* Automatic scrolling to matches

---

# 🎨 Syntax Highlighting

Current syntax highlighting includes:

* C keywords
* Strings
* Numbers
* Comments
* ANSI color rendering

---

# ↩️ Undo / Redo

Basic history system with:

* Undo (`Ctrl+Z`)
* Redo (`Ctrl+Y`)

---

# 📋 Selection and Clipboard

* Visual text selection
* Copy
* Cut
* Paste
* Internal clipboard

---

# 🌍 UTF-8 Support

Current UTF-8 support includes:

* Proper UTF-8 Backspace handling
* Multibyte character navigation
* Correct rendering of accented characters
* Improved cursor positioning
* Better compatibility with Portuguese text

Supported examples:

* ç
* á
* ã
* ê
* é
* ô

---

# 🖥️ Requirements

## ✅ Supported Platforms

* Linux
* Unix-like systems

## 📦 Dependencies

* GCC
* Make
* ANSI-compatible terminal

---

# ⚙️ Compilation

## 🛠️ Using Make

```bash
make
```

## 🧱 Manual Compilation

```bash
gcc -Wall -Wextra -pedantic -std=c99 \
main.c editor.c buffer.c terminal.c utf8.c \
-o verve
```

---

# ▶️ Running

## 📄 Open the editor

```bash
./verve
```

## 📂 Open a file

```bash
./verve file.txt
```

---

# ⌨️ Keyboard Shortcuts

| Key    | Action           |
| ------ | ---------------- |
| Ctrl+S | Save file        |
| Ctrl+Q | Quit editor      |
| Ctrl+F | Search           |
| Ctrl+Z | Undo             |
| Ctrl+Y | Redo             |
| Ctrl+B | Start selection  |
| Ctrl+C | Copy selection   |
| Ctrl+X | Cut selection    |
| Ctrl+V | Paste            |
| Enter  | New line         |
| Tab    | Indentation      |
| Esc    | Cancel selection |

---

# 🧩 Project Structure

| File         | Responsibility                     |
| ------------ | ---------------------------------- |
| `main.c`     | Program initialization             |
| `editor.c`   | Main editor logic                  |
| `buffer.c`   | Text buffer manipulation           |
| `terminal.c` | Raw terminal mode and input        |
| `utf8.c`     | UTF-8 utilities                    |
| `utf8.h`     | UTF-8 declarations                 |
| `editor.h`   | Global structures and declarations |
| `buffer.h`   | Buffer structures and declarations |

---

# 🏗️ Internal Architecture

Verve currently uses:

* line-based text buffers
* dynamic memory allocation
* incremental rendering
* basic syntax parsing
* reversible action history

Each text row stores:

* original content
* rendered content
* syntax highlight metadata

---

# 🧠 Memory Management

The editor heavily relies on:

* `malloc`
* `realloc`
* `free`
* dynamic buffer resizing

This project was specifically designed to explore efficient memory usage in C.

---

# ⚠️ Current Limitations

Verve is already functional, but still has several limitations:

* partial UTF-8 support
* simplified undo/redo system
* no mouse support
* no autosave or crash recovery
* no gap buffer or piece table
* performance limitations with huge files
* no plugin/configuration system
* syntax highlighting only for C-like syntax

---

# 🎯 Project Goals

This project was built primarily as a learning experience for:

* low-level C programming
* terminal applications
* text editor internals
* dynamic memory management
* ANSI escape sequences
* editor rendering pipelines

Verve is not intended to replace mature editors such as:

* Vim
* Neovim
* Helix
* Emacs
* Nano

However, it already works as a real and usable terminal editor for small and medium-sized tasks.

---

# 🔮 Future Improvements

Planned or possible future features:

* complete UTF-8 support
* mouse support
* themes
* configuration files
* advanced syntax highlighting
* robust undo/redo
* autosave and recovery
* gap buffer implementation
* piece table implementation
* basic LSP integration
* split windows
* plugin system

---

# 📜 License

MIT
