# Verve

A minimal terminal text editor written in C.

Verve was created as a low-level systems programming project focused on:
- dynamic memory allocation
- terminal rendering
- text editor architecture
- buffer management
- UTF-8 handling
- manual data structures
- ANSI terminal control

The project is inspired by editors such as:
- Nano
- Kilo
- Vim
- Micro

---

# Features

## Text Editing
- Text insertion and deletion
- Multiline editing
- Automatic line merging
- Smart Backspace/Delete behavior
- Cursor navigation
- Home / End support
- Page Up / Page Down support
- Automatic indentation
- Smart TAB handling (4 spaces)

---

## File Management
- Open files
- Save files
- Vertical scrolling
- Horizontal scrolling

---

## Search
- Incremental search
- Automatic scrolling to matches

---

# Syntax Highlighting

Current syntax highlighting includes:
- C keywords
- Strings
- Numbers
- Comments
- ANSI color rendering

---

# Undo / Redo

Basic history system with:
- Undo (`Ctrl+Z`)
- Redo (`Ctrl+Y`)

---

# Selection and Clipboard

- Visual text selection
- Copy
- Cut
- Paste
- Internal clipboard

---

# UTF-8 Support

Partial UTF-8 support:
- Multibyte character navigation
- Improved Unicode handling
- Better support for accented characters

---

# Requirements

## Supported Platforms
- Linux
- Unix-like systems

## Dependencies
- GCC
- Make
- ANSI-compatible terminal

---

# Compilation

Using Make:

```bash
make

Manual compilation:

gcc -Wall -Wextra -pedantic -std=c99 \
main.c editor.c buffer.c terminal.c \
-o verve
Running

Open the editor:

./verve

Open a file:

./verve file.txt
Keyboard Shortcuts
Key	Action
Ctrl+S	Save file
Ctrl+Q	Quit editor
Ctrl+F	Search
Ctrl+Z	Undo
Ctrl+Y	Redo
Ctrl+B	Start selection
Ctrl+C	Copy selection
Ctrl+X	Cut selection
Ctrl+V	Paste
Enter	New line
Tab	Indentation
Esc	Cancel selection
Project Structure
File	Responsibility
main.c	Program initialization
editor.c	Main editor logic
buffer.c	Text buffer manipulation
terminal.c	Raw terminal mode and input
editor.h	Global structures and declarations
buffer.h	Buffer structures and declarations
Internal Architecture

Verve currently uses:

line-based text buffers
dynamic memory allocation
incremental rendering
basic syntax parsing
reversible action history

Each text row stores:

original content
rendered content
syntax highlight metadata
Memory Management

The editor heavily relies on:

malloc
realloc
free
dynamic buffer resizing

This project was specifically designed to explore efficient memory usage in C.

Current Limitations

Verve is already functional, but still has several limitations:

Incomplete UTF-8 support
Simplified undo/redo system
No mouse support
No multiple buffers/tabs
No autosave or crash recovery
No gap buffer or piece table
Performance limitations with huge files
No plugin/configuration system
Project Goals

This project was built primarily as a learning experience for:

low-level C programming
terminal applications
text editor internals
dynamic memory management
ANSI escape sequences
editor rendering pipelines

Verve is not intended to replace mature editors such as:

Vim
Neovim
Helix
Emacs
Nano

However, it already works as a real and usable terminal editor for small and medium-sized tasks.

Future Improvements

Planned or possible future features:

Complete UTF-8 support
Mouse support
Multiple buffers/tabs
Themes
Configuration files
Advanced syntax highlighting
Robust undo/redo
Autosave and recovery
Gap Buffer implementation
Piece Table implementation
Basic LSP integration
Split windows
Plugin system
License

MIT
