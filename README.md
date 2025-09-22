# MMM Text
A mini multi-modal text editor written in C++

## Features
- Minimal: less than 2000 lines of code and no 3rd party libraries.
- Cross-platform: enjoy similar functionality in both Windows and Linux.
- Keyboard-centric: with features like navigating with two command line cursors at once, mmmtext is built with the keyboard in mind.
- Text wrapping: lines too long for your window? No worries.

## Usage
mmmtext is a command line tool for Windows and Linux (Mac is not supported, but may work). To open the editor, simply execute `mmmtext`.

You may also pass a file in as an argument to open it on startup.

### Prompt / movement mode
initially, you are able to type with most keys into the prompt box at the top. In this prompt, hit `?` at any time to get a help menu. The help menu is written below:
```
HOW TO USE: 
In the prompt press: 
o to open a new file
s to save a new file (autosave is supported)
c to pass a command while not leaving the editor entirely
q to quit
w to write in the text
h, j, k, and l to move around the text while in prompt mode
In write mode: 
Use Arrow keys to move
Press ESC twice to exit and autosave.
```
### Write mode
write mode is similar to insert mode from Vim. here you change the actual contents of the file you are working with.

## About
The goal of this project was simply to make a text editor. Even simple editors like nano consist of over 10,000 lines of code as well as libraries that restrict it to certain platforms. While this editor is much smaller it is lacking in some features such as live window resizing and vertical navigation at this time.

I use the following to compile the binaries using g++:

`g++ mmmtext.cpp Line.cpp -static -o mmmtext`
