#ifndef WINDOWSFUNCTIONS_HPP
#define WINDOWSFUNCTIONS_HPP


//#include "UnixFunctions.hpp"
#include <windows.h>
#include <conio.h>
// using namespace std; causes "byte" declaration ambiguity

void SetTermSettings() {
    // Enable ANSI escape sequences in Windows console. They are diabled by default.
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void Quit() {}; // Linux only
void UnQuit() {};


bool CheckForEscapeCode() {return false;} // This is only needed for UNIX because of 
                                          // the use of escape codes for arrow keys.

void GetTerminalSize(int& height, int& width) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    
    // Get the handle to the standard output device
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // If we can't get the console screen buffer info, return -1
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        std::cerr << "Error getting console screen buffer info. Error code: " 
                  << GetLastError() << std::endl;
        return;
    }
    
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}


char GetCharacter() {
    return _getch();
}

#endif // WINDOWSFUNCTIONS_HPP