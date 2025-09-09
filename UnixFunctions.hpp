#ifndef UNIXFUNCTIONS_HPP
#define UNIXFUNCTIONS_HPP


#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>

struct termios oldt, newt; // These are global, used in SetTermSettings() and Quit()
void SetTermSettings() {
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_iflag &= ~(IXON | ICRNL); // These 2 lines set terminal settings: read byte-by-byte, no echo char, disable crtl+c, etc.
    newt.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void Quit() {
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  

}

void UnQuit() {
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); 
}

bool CheckForEscapeCode() {
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 50000; // 50ms timeout

    return (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0);
}

void GetTerminalSize(int& height, int& width) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        height = width = -1;
    }
    height = w.ws_row; // Assign height
    width =  w.ws_col; // Assign the width

}



char GetCharacter() {
    return getchar();
}

#endif // UNIXFUNCTIONS_HPP