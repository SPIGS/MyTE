# MyTE - My Text Editor

![Preview of Myte](/../screenshots/screenshots/preview.png?raw=true "Preview of Myte")

MyTE (rhymes with byte) is **My** **T**ext **E**ditor. It is a small graphical text editor written in C. It was originally created as a learning exercise and was inspired by [ded](https://github.com/tsoding/ded).

## Features

MyTE is currently a work in progress and missing some crucial features (clipboard manipulation, multiple buffers/panes/tabs). Despite this, MyTE still has some advanced features that some more basic text editors lack:

- Syntax highlighting support for various languages (and a straight-forward means of adding more)
- Rudimentary user theming/color schemes (also pretty straight-forward to create)
- User configuration/theme hot reloading
- Smooth animations
- Mouse controls
- Text selection

## Controls

MyTE currently has two modes - an editor mode and a file browsing mode. The editor mode is default and works mostly how one would expect. File browsing mode presents a simple file browser in the vein of vim or emacs. 

- **Editor Mode**
    - typing inserts characters into the buffer
    - arrow keys move the cursor around the buffer
    - `CTRL + left/right` move the cursor back/forward one whole word.
    - `CTRL + BACKSPACE/DELETE` backspaces/deletes a whole word.
    - `CTRL + O` opens file browsing mode
    - `CTRL + S` saves the current file to disk
    - `CTRL + SHIFT + S` opens a save dialog to save a new file.
    - `MOUSE LEFT` moves to the cursor to the position you clicked in the buffer
    - `MOUSE SCROLL` scrolls the buffer
    - `SHIFT + arrows/mouse` selects text
- **File Browsing Mode**
    - `up/down` moves the selection up and down the directory listing.
    - `ENTER` opens the selected directory/file
    - `ESCAPE` closes the file browser and goes back to editor mode.
 - **Special global shortcuts**
    - `F5` completely hot reloads the application (user configs, themes, and the current file)
  
## Configuration

All configuration - including user settings, highlighting rules, and colorschemes - are done via TOML files. These are loaded into the program at startup and can be changed and hot-reloaded while the program is running. The formats for them are pretty self-explanatory and it should be easy to edit them.

## Installation

MyTE currently only builds on Linux and has 3 dependencies.

To build MyTE just clone this repo, install the dependencies via your package manager (`glew glfw3 freetype2`), and run `make all`. The program should be built in the newly created `build` folder.

## References

- [easy-renderer](https://github.com/PixelRifts/easy-renderer): a basic OpenGL renderer by PixelRifts. Used as the basis for the renderer for this project.
- [tomlc99](https://github.com/cktan/tomlc99): a TOML file loading library written in C by cktan. Used for loading syntax highlighting configs.
    - [TOML v1.0.0 standard](https://toml.io/en/v1.0.0): the TOML standard for the tomlc99 library.
- [ded](https://github.com/tsoding/ded): a text editor by tsoding. An inspiration for this project; [YouTube Playlist](https://www.youtube.com/playlist?list=PLpM-Dvs8t0VZVshbPeHPculzFFBdQWIFu)
- [Pico](https://github.com/jon-lipstate/pico): editor by John Lipstate. Helpful during the starting phases of this project; [YouTube Playlist](https://www.youtube.com/playlist?list=PLqN23W-K4Tn2LdgSCJOCFFiPO26mvsXm3); [Video on Ropes vs Gap Buffers](https://www.youtube.com/watch?v=xhFzu3Wm0Qs)
- [Programming a text editor from scratch](https://www.youtube.com/watch?v=oDv6DfQxhtQ): by Bitwise; [Part 2](https://www.youtube.com/watch?v=nLyBzD4_Z_4); [Part 3](https://www.youtube.com/watch?v=cgd_qGekPdI): helpful during the starting phases of this project.
- [Space Duck Color Theme](https://github.com/pineapplegiant/spaceduck): the colorscheme for this editor.
- [Gruvbox](https://github.com/morhetz/gruvbox): another colorscheme used in this editor.
- [Iosevka](https://github.com/be5invis/Iosevka): the default font for this editor.
- [GNU Free Mono](https://www.gnu.org/software/freefont/): another font used in this editor.
