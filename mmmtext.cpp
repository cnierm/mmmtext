/*                                    _            _   
 *     _ __ ___  _ __ ___  _ __ ___ | |_ _____  _| |_ 
 *   | '_ ` _ \| '_ ` _ \| '_ ` _ \| __/ _ \ \/ / __|
 *  | | | | | | | | | | | | | | | | ||  __/>  <| |_ 
 * |_| |_| |_|_| |_| |_|_| |_| |_|\__\___/_/\_\\__|
 * 
 * Created by Caleb Nierman, Spring 2025
 *
 */



#include <iostream>
#include <fstream>
#include <string>
#include "Line.hpp"
#include <vector>

#ifdef _WIN32 // Runs platform-specific #includes and defines specific functions.

#include "WindowsFunctions.hpp"

#else // Sets up for a unix-like OS (MacOS, Linux, BSD, onlineGDB, etc)

#include "UnixFunctions.hpp"

#endif

/***************GLOBALS**************/
int termHeight;
int termWidth;

int posX = 3; // Current position of cursor in the text buffer.
int posY = 4;
int posLine = 0; // Current index of the vector the cursor is looking at.
int textPosX = 0;

std::string activeFile = "";

std::vector<Line> text; // This is the text document



/************************************/

void exit(int n);

void SetCursorCoords(int x, int y) {
    std::cout << "\x1b[" << y << ";" << x << "H"; 
}
std::string SetCursorCoordsStr(int x, int y) {
    return "\x1b[" + std::to_string(y) + ";" 
                   + std::to_string(x) + "H";
}

void OpenFile(std::string file) { // Note that for both opening and saving, this program will not keep the file
    std::ifstream ifs;            // open, so editing from another text editor while having the file open in here
    std::string currLine;         // may cause desync.

    ifs.open(file);

    if (!ifs.is_open()) {

        std::cout << SetCursorCoordsStr(5,2) << file 
                  << ": file not found. Press Any Key to continue." << std::flush;
        GetCharacter();
        return;
    }

    text.clear(); // clear the current file open.

    getline(ifs, currLine);
    while (!ifs.fail()) {
        Line line(currLine);
        text.push_back(line);
        getline(ifs, currLine);
    }

    activeFile = file; // Since it succeeded, we are now looking at a new file.

    ifs.close();
}

void SaveFile(std::string file) {
    std::ofstream ofs;
    int i;

    ofs.open(file);
    
    if (!ofs.is_open() && activeFile != "") {
        std::cout << SetCursorCoordsStr(5,2) << file 
                  << ": Unable to open file. Press Any Key to continue." << std::flush;
        GetCharacter();
        return;
    }

    for (i = 0; i < text.size(); ++i) {
        ofs << text.at(i).GetLineData() << "\n";
    }
    ofs.close();

}


void ClearScreen() {
    std::cout << "\x1b[2J";
    SetCursorCoords(1, 1);
    std::cout << std::flush;
}

void ClearLinesAndIndicators() {
    int i;

    std::cout << "\x1b[s";
    SetCursorCoords(1, 4);
    for (i = 3; i < termHeight; ++i) {
        std::cout << std::string(termWidth, ' ') << SetCursorCoordsStr(1, i) << "\x1b[B";
    }
    std::cout << "\x1b[u";
}

void RenderLineIndicators(std::vector<Line> doc) {
    int i;
    int j;
    int heightUsed = 0;
    std::cout << "\x1b[s\x1b[2m" << SetCursorCoordsStr(1, 4); // Save cursor position, faint mode.
    
    for (i = 0; i < doc.size(); ++i) {
        int lineHeight = doc.at(i).GetNumVisualLinesUsed();
        if (heightUsed + lineHeight > termHeight - 3) {
            lineHeight = termHeight - 3 - heightUsed; // If lines visual height exceeds console space, truncate the height.
        }
        std::cout << "~\x1b[D";
        for (j = 0; j < lineHeight; ++j) {
            std::cout << "\x1b[B";
        }
        heightUsed += lineHeight;
    }

    std::cout << "\x1b[0m\x1b[u" << std::flush;

}

void RenderAllLines(std::vector<Line> doc, int startIndex) {
    int i;
    int j;
    int heightUsed = 0;
    std::cout << "\x1b[s"; // Backup cursor
    SetCursorCoords(3, 4); // Text window top left
    int posY = 4;

    for (i = startIndex; i < doc.size(); ++i) {
        int lineHeight = doc.at(i).GetNumVisualLinesUsed();
        if (heightUsed == termHeight - 3) return;             // If the height limit has been reached, terminate the function.
        else if (heightUsed + lineHeight <= termHeight - 3) { // if we are not hitting the height limit by rendering next line
            if (lineHeight == 1) {
                std::cout << doc.at(i).GetLineData(); // Get whole line
                ++posY; 
                
                SetCursorCoords(3, posY);
            }
            else if (lineHeight > 1) {
                for (j = 0; j < lineHeight; ++j) {
                    std::cout << doc.at(i).GetLineData().substr(0 + j * (termWidth - 2), termWidth - 2);
                    ++posY;
                    SetCursorCoords(3, posY);
                }
                
            }
        }
        else { // Only part of the final line can be rendered
            for (j = 0; j < termHeight - 3 - heightUsed; ++j) {
                std::cout << doc.at(i).GetLineData().substr(0 + j * (termWidth - 2), termWidth - 2);
                ++posY;
                SetCursorCoords(3, posY);
            }
            return;
        }
    }
    std::cout << "\x1b[u";
}



void LoadPrompt() {
    SetCursorCoords(1,1);
    if (termWidth < 10 || termHeight < 10) { // Error Checking
        std::cerr << "Termianl Size could not be obtained or is too small.";
        return;
    }
    std::cout << std::string(termWidth, '-') << "\n";
    std::cout << "\x1b[1;32m>>> \x1b[0m" << "\n";
    std::cout << std::string(termWidth, '-') << std::endl;
    SetCursorCoords(5, 2);
    return;
}

// COORDINATE FUNCTIONS
// This program uses two coordinate systems: visual coordinates and text coordinates
// Visual coordinates help with moving the cursor
// Text coordinates help with inserting text
// Below are translation functions to keep them updated

void GetVisualFromText(int textX, int textY, int& visX, int& visY) {
    visY = 4; // Top of text window
    int i;
    for (i = 0; i < textY; ++i) { // Gets us to the top of the text line were on
        visY += text.at(i).GetNumVisualLinesUsed();
    }

    // If the line is wrapped and we are in the wrapped section, adjust visY
    visY += textX / (termWidth - 2); // Integer Division

    visX = 3 + (textX % (termWidth - 2)); // Get X as well
}

void GetTextFromVisual(int visX, int visY, int& textX, int& textY) {
    int scanY = 4; // This value will scan the text and textY will be adjusted
    int i;


    for (i = 0; i < text.size(); ++i) {
        int lineHeight = text.at(i).GetNumVisualLinesUsed();
        if (visY < scanY + lineHeight) { // If adding this line will pass over visY
            textY = i;
            break; // exits the for loop
        }
        scanY += lineHeight;
    }

    if (textY > text.size()) textY = text.size() - 1; // cap at last line

    // if there is still a difference b/n the scanning head and visY, that is from text wrapping, and textX is adjusted.
    textX = ((visY - scanY) * (termWidth - 2)) + (visX - 3);
}

void RenderLine(int n) { // A Smaller function compared to RenderAllLines() for when rerendering everything is not necesarry.
    if (n >= text.size()) return; // TODO: Clear out line for backspace

    int lineHeight = text.at(n).GetNumVisualLinesUsed();
    std::cout << "\x1b[s";
    int textPosX = 0; // redeclare these variable because we dont want to actually overwrite the globals
    int i, posX, posY;

    GetVisualFromText(textPosX, n, posX, posY);
    SetCursorCoords(posX, posY);
    for (i = 0; i < lineHeight; ++i) {
        std::cout << text.at(n).GetLineData().substr(0 + i * (termWidth - 2), termWidth - 2);
        ++posY;
        SetCursorCoords(3, posY);
    }
    std::cout << "\x1b[u"; 
    // TODO: add support for checking and rendering partially, much of the code should be in RenderAllLines
}

void ClearLine(int n) {
    if (n >= text.size()) return; // TODO: Clear out line for backspace

    int lineHeight = text.at(n).GetNumVisualLinesUsed();
    std::cout << "\x1b[s";
    int textPosX = 0; // redeclare these variable because we dont want to actually overwrite the globals
    int i, posX, posY;

    GetVisualFromText(textPosX, n, posX, posY);
    SetCursorCoords(posX, posY);
    for (i = 0; i < lineHeight; ++i) {
        std::cout << std::string(termWidth - 2, ' ');
        ++posY;
        SetCursorCoords(3, posY);
    }
    std::cout << "\x1b[u"; 
}

char CharAtPos() { // Get the character at the current text postition.
    std::string lineData = text.at(posLine).GetLineData();
    if (posLine >= text.size() || textPosX >= lineData.size()) {
        return ' '; // Give ' ' if we are at the end of text or line
    }
    else {
        return lineData[textPosX];
    }
}

void PromptMode(std::string& input) { // TODO: When you press '?', put up a help window
    input.clear();
    int promptPosX = 5;
    
    char cursorChar = CharAtPos(); // Draw initial cursor
    
    std::cout << "\x1b[s" << SetCursorCoordsStr(posX, posY) << "\x1b[30m\x1b[47m"
              << cursorChar << "\x1b[0m\x1b[u" << std::flush;
    
    while (true) {
        char inChar = GetCharacter();
        //cursorChar = CharAtPos();

        switch (inChar) {
            case '?':
                ClearScreen();
                SetCursorCoords(1, 1);
                std::cout << "HOW TO USE: \n"
                          << "In the prompt press: \n"
                          << "o to open a new file\n"
                          << "s to save a new file (autosave is supported)\n"
                          << "c to pass a command while not leaving the editor entirely\n"
                          << "q to quit\n"
                          << "w to write in the text\n"
                          << "h, j, k, and l to move around the text while in prompt mode\n"
                          << "In write mode: \n"
                          << "Use Arrow keys to move\n"
                          << "Press ESC twice to exit and autosave.\n\n"
                          << "Press Any key to return to the text editor." << std::flush;
                GetCharacter();
                ClearScreen();
                LoadPrompt();
                RenderLineIndicators(text);
                RenderAllLines(text, 0);
                SetCursorCoords(promptPosX, 2);
                std::cout << "\x1b[s" << SetCursorCoordsStr(posX, posY) << "\x1b[30m\x1b[47m"
                          << cursorChar << "\x1b[0m\x1b[u" << std::flush
                          << "\x1b[s" << SetCursorCoordsStr(5, 2) 
                          << input << "\x1b[u";

                break;
            case 'h': {
                if (textPosX > 0) {
                    std::cout << "\x1b[s" << SetCursorCoordsStr(posX, posY)
                              << cursorChar;
                    --textPosX; // Move left

                    GetVisualFromText(textPosX, posLine, posX, posY);
                    cursorChar = CharAtPos(); // Update cursor character
                    std::cout << SetCursorCoordsStr(posX, posY) << "\x1b[30m\x1b[47m"
                              << cursorChar << "\x1b[0m\x1b[u" << std::flush;
                }
                else if (posLine > 0) {
                    std::cout << "\x1b[s" << SetCursorCoordsStr(posX, posY)
                              << cursorChar;
                    --posLine;
                    textPosX = text.at(posLine).GetLineData().size();
                    GetVisualFromText(textPosX, posLine, posX, posY);
                    cursorChar = CharAtPos();
                    std::cout << SetCursorCoordsStr(posX, posY) << "\x1b[30m\x1b[47m"
                                << cursorChar << "\x1b[0m\x1b[u" << std::flush;
                }
                break;
            }
            case 'j': {
                if (posLine < text.size() - 1) {
                    std::cout << "\x1b[s" << SetCursorCoordsStr(posX, posY)
                              << cursorChar;
                    ++posLine;

                    if (textPosX > text.at(posLine).GetLineData().size()) {
                        textPosX = text.at(posLine).GetLineData().size();
                        //if (textPosX > 0) --textPosX; // safety so empty strings will not have a position of -1
                    }

                    GetVisualFromText(textPosX, posLine, posX, posY);
                    cursorChar = CharAtPos(); // Update cursor character
                    std::cout << SetCursorCoordsStr(posX, posY) << "\x1b[30m\x1b[47m"
                              << cursorChar << "\x1b[0m\x1b[u" << std::flush;
                }
                break;
            }

            case 'k': {
                if (posLine > 0) {
                    std::cout << "\x1b[s" << SetCursorCoordsStr(posX, posY)
                              << cursorChar;
                    --posLine;

                    if (textPosX > text.at(posLine).GetLineData().size()) {
                        textPosX = text.at(posLine).GetLineData().size();
                        //if (textPosX > 0) --textPosX; // so empty strings will not have a position of -1
                    }

                    GetVisualFromText(textPosX, posLine, posX, posY);
                    cursorChar = CharAtPos(); // Update cursor character
                    std::cout << SetCursorCoordsStr(posX, posY) << "\x1b[30m\x1b[47m"
                              << cursorChar << "\x1b[0m\x1b[u" << std::flush;
                }
                break;
            }

            case 'l': {
                    if (textPosX < text.at(posLine).GetLineData().size()) {
                        std::cout << "\x1b[s" << SetCursorCoordsStr(posX, posY)
                                << cursorChar;
                        ++textPosX; // Move Right

                        GetVisualFromText(textPosX, posLine, posX, posY);
                        cursorChar = CharAtPos(); // Update cursor character
                        std::cout << SetCursorCoordsStr(posX, posY) << "\x1b[30m\x1b[47m"
                                  << cursorChar << "\x1b[0m\x1b[u" << std::flush;
                    }
                    else if (posLine < text.size() - 1) {
                        std::cout << "\x1b[s" << SetCursorCoordsStr(posX, posY)
                                  << cursorChar;
                        textPosX = 0;
                        ++posLine;
                        GetVisualFromText(textPosX, posLine, posX, posY);
                        cursorChar = CharAtPos();
                        std::cout << SetCursorCoordsStr(posX, posY) << "\x1b[30m\x1b[47m"
                                  << cursorChar << "\x1b[0m\x1b[u" << std::flush;
                    }

                break;
            }

            case 8:   // Windows Backspace
            case 127: // Unix Backspace
                if (promptPosX > 5) {
                    --promptPosX;
                    input.erase(promptPosX - 5, 1);
                    // Rewrite prompt input, add a space to remove the extra character.
                    std::cout << "\x1b[s" << SetCursorCoordsStr(5, 2)
                            << input << " \x1b[u\x1b[D" << std::flush;
                }
                break;
            case 9: break; // Do nothing with tab
            case 10: // Enter key (cross platform)
            case 13:
                std::cout << "\x1b[s" << SetCursorCoordsStr(posX, posY)
                          << cursorChar << "\x1b[u" << std::flush; // safety flush
                return; // ^ deletes cursor, in case we go into write mode.

            case 27: { // 27 is ESC. Unix arrow keys (ex: ^[[D)
                if (GetCharacter() == '[') {
                    char arrowKeyUnix = GetCharacter();
                    switch (arrowKeyUnix) {
                        case 'C': // Right
                            if (promptPosX - 5 < input.size()) { 
                                std::cout << "\x1b[C";
                                ++promptPosX;
                            }
                            break;
                        case 'D': // Left
                            if (promptPosX > 5) {
                                std::cout << "\x1b[D";
                                --promptPosX;
                            }
                            break;
                        default:
                            break;       
                    }
                }
                break;
            }

            case -32: { // Windows Arrow Keys
                char arrowKeyWin = GetCharacter();
                switch (arrowKeyWin) {
                    case 77: // Right
                        if (promptPosX - 5 < input.size()) { 
                            std::cout << "\x1b[C";
                            ++promptPosX;
                        }  
                        break;
                    case 75: // Left
                        if (promptPosX > 5) {
                            std::cout << "\x1b[D";
                            --promptPosX;
                        }
                        break;
                    default:
                        break;
                }
                break;
            }

            default:
                input.insert(promptPosX - 5, 1, inChar);
                std::cout << "\x1b[s" << SetCursorCoordsStr(5, 2) 
                          << input << "\x1b[u\x1b[C";
                ++promptPosX;
                break;

        }
        // Debug
        // std::cout << "\x1b[s" << SetCursorCoordsStr(termWidth - 12, 4) << "( " << posX << " , " << posY << " )  "
        //           << SetCursorCoordsStr(termWidth - 12, 5) << "( " << textPosX << " , " << posLine << " )  \x1b[u"; 
    }

}


void ClearPrompt() {
    SetCursorCoords(5, 2);
    std::cout << std::string(termWidth - 5, ' ') << std::flush;
    SetCursorCoords(5, 2);
}



void WriteMode() {
    SetCursorCoords(posX, posY); // Jump to text buffer cursor
    while (true) {
        char inChar = GetCharacter();

        switch (inChar) {
            
            case 10: // Enter
            case 13:
                text.insert(text.begin() + posLine + 1, text.at(posLine).GetLineData().substr(textPosX));
                text.at(posLine) = Line(text.at(posLine).GetLineData().substr(0,textPosX));
                ++posLine;
                textPosX = 0;
                GetVisualFromText(textPosX, posLine, posX, posY);
                SetCursorCoords(posX, posY);
                ClearLinesAndIndicators();
                RenderLineIndicators(text);
                RenderAllLines(text, 0);
                break;
            case 8: // Backspace
            case 127: {
                if (textPosX > 0) {
                    int rerenderNeeded = text.at(posLine).DeleteCharacter(textPosX - 1); // so that the character to the left of the cursor is deleted
                    --textPosX;
                    GetVisualFromText(textPosX, posLine, posX, posY);
                    SetCursorCoords(posX, posY);
                    if (rerenderNeeded != 0) {
                        ClearLinesAndIndicators();
                        RenderLineIndicators(text);
                        RenderAllLines(text, 0); // When scrolling is added, DELETE THE 0!!!
                        break;
                    }
                    ClearLine(posLine);
                    RenderLine(posLine);
                }
                else if (posLine > 0) { // Delete the line and append the data to the previous line.
                    textPosX = text.at(posLine - 1).GetLineData().size();
                    text.at(posLine - 1) = Line(text.at(posLine - 1).GetLineData() + 
                                                        text.at(posLine).GetLineData());
                    text.erase(text.begin() + posLine);
                    --posLine;
                    GetVisualFromText(textPosX, posLine, posX, posY);
                    SetCursorCoords(posX, posY);
                    ClearLinesAndIndicators();
                    RenderLineIndicators(text);
                    RenderAllLines(text, 0);
                }
                break;
            }
            case 27: { // 27 is ESC. Unix arrow keys (ex: ^[[D)
                char c = GetCharacter();
                if (c == '[') {
                    char arrowKeyUnix = GetCharacter();
                    switch (arrowKeyUnix) {
                        case 'A': // Up
                            if (posLine > 0) {
                                --posLine;

                                if (textPosX > text.at(posLine).GetLineData().size()) {
                                    textPosX = text.at(posLine).GetLineData().size();
                                    //if (textPosX > 0) --textPosX; // so empty strings will not have a position of -1
                                }

                                GetVisualFromText(textPosX, posLine, posX, posY);
                                
                                SetCursorCoords(posX, posY);
                            }
                            break;
                        case 'B': // Down
                            if (posLine < text.size() - 1) {
                                ++posLine;
            
                                if (textPosX > text.at(posLine).GetLineData().size()) {
                                    textPosX = text.at(posLine).GetLineData().size();
                                    //if (textPosX > 0) --textPosX; // safety so empty strings will not have a position of -1
                                }
            
                                GetVisualFromText(textPosX, posLine, posX, posY);
                                
                                SetCursorCoords(posX, posY);
                            }
                            
                            break;
                        
                        case 'C': // Right
                            if (textPosX < text.at(posLine).GetLineData().size()) {
                                ++textPosX; // Move Right
                                GetVisualFromText(textPosX, posLine, posX, posY);
                                SetCursorCoords(posX, posY);
                            }
                            else if (posLine < text.size() - 1) {
                                textPosX = 0;
                                ++posLine;
                                GetVisualFromText(textPosX, posLine, posX, posY);
                                SetCursorCoords(posX, posY);
                            }
                            break;
                        case 'D': // Left
                            if (textPosX > 0) {
                                --textPosX; // Move Left
                                GetVisualFromText(textPosX, posLine, posX, posY);
                                SetCursorCoords(posX, posY);
                            }
                            else if (posLine > 0) {
                                --posLine;
                                textPosX = text.at(posLine).GetLineData().size();
                                GetVisualFromText(textPosX, posLine, posX, posY);
                                SetCursorCoords(posX, posY);
                            }
                            break;
                        default:
                            break;       
                    }
                }
                else if (c == 27) { // double escape
                    SaveFile(activeFile); // Auto save
                    return;
                }
                break;
            }


            case -32: { // Windows Arrow Keys
                char arrowKeyWin = GetCharacter();
                switch (arrowKeyWin) {
                    case 72: // Up
                        if (posLine > 0) {
                            --posLine;

                            if (textPosX > text.at(posLine).GetLineData().size()) {
                                textPosX = text.at(posLine).GetLineData().size();
                                //if (textPosX > 0) --textPosX; // so empty strings will not have a position of -1
                            }

                            GetVisualFromText(textPosX, posLine, posX, posY);
                            
                            SetCursorCoords(posX, posY);
                        }
                        break;
                    case 80: // Down
                        if (posLine < text.size() - 1) {
                            ++posLine;
        
                            if (textPosX > text.at(posLine).GetLineData().size()) {
                                textPosX = text.at(posLine).GetLineData().size();
                                //if (textPosX > 0) --textPosX; // safety so empty strings will not have a position of -1
                            }
        
                            GetVisualFromText(textPosX, posLine, posX, posY);
                            
                            SetCursorCoords(posX, posY);
                        }
                        break;
                    case 77: // Right
                        if (textPosX < text.at(posLine).GetLineData().size()) {
                            ++textPosX; // Move Right
                            GetVisualFromText(textPosX, posLine, posX, posY);
                            SetCursorCoords(posX, posY);
                        }
                        else if (posLine < text.size() - 1) {
                            textPosX = 0;
                            ++posLine;
                            GetVisualFromText(textPosX, posLine, posX, posY);
                            SetCursorCoords(posX, posY);
                        }
                        break;
                    case 75: // Left
                        if (textPosX > 0) {
                            --textPosX; // Move Left
                            GetVisualFromText(textPosX, posLine, posX, posY);
                            SetCursorCoords(posX, posY);
                        }
                        else if (posLine > 0) {
                            --posLine;
                            textPosX = text.at(posLine).GetLineData().size();
                            GetVisualFromText(textPosX, posLine, posX, posY);
                            SetCursorCoords(posX, posY);
                        }
                        break;
                    default:
                        break;
                }
                break;
            }

            case 9: { // For tab, just insert 4 spaces
                for (int i = 0; i < 3; ++i) {
                    text.at(posLine).InsertCharacter(textPosX, ' ');
                }
                int rerenderNeeded = text.at(posLine).InsertCharacter(textPosX, ' ');
                textPosX += 4;
                GetVisualFromText(textPosX, posLine, posX, posY);
                SetCursorCoords(posX, posY);
                if (rerenderNeeded != 0) {
                    ClearLinesAndIndicators();
                    RenderLineIndicators(text);
                    RenderAllLines(text, 0); // When scrolling is added, DELETE THE 0!!!
                }
                else {
                    RenderLine(posLine);
                }
                break;
            }

            
            default: {
                int rerenderNeeded = text.at(posLine).InsertCharacter(textPosX, inChar);
                ++textPosX;
                GetVisualFromText(textPosX, posLine, posX, posY);
                SetCursorCoords(posX, posY);
                if (rerenderNeeded != 0) {
                    ClearLinesAndIndicators();
                    RenderLineIndicators(text);
                    RenderAllLines(text, 0); // When scrolling is added, DELETE THE 0!!!
                }
                else {
                    RenderLine(posLine);
                }
                break;
            }

        }
    }
}

int main(int argc, char* argv[]) { 

    

    if (argc == 2) { // If one file is passed through
        activeFile = argv[1];
        
    }
    else if (argc > 2) { // Too many arguments passed
        std::cout << "Too many arguments. This program takes in one file name argument." << std::endl;
        return 1;
    }



    GetTerminalSize(termHeight, termWidth); // This is only done one time in the program. Resizing window may break functionality

    
    SetTermSettings();
    if (termWidth < 5 || termHeight < 12) {
        std::cerr << "ERROR: Terminal size is too small or could not be obtained. "
                  << "\nPress any key to enter into safe mode.";
        GetCharacter();
        termWidth = 80; // Safe mode is to be able to use this with onlineGDB
        termHeight = 15;
    }

    ClearScreen();
    LoadPrompt();
    
    if (activeFile != "") {
        OpenFile(activeFile);
    }

    if (text.size() == 0) {
        text.push_back(Line(""));
    }

    RenderLineIndicators(text);
    RenderAllLines(text, 0);



    std::string input;

    while (true) {
        ClearPrompt();
        
        LoadPrompt();
        
        PromptMode(input);
        //std::cout << "\n\n\n  " << text.size();


        

        if (input[0] == 'w') { // write mode
            WriteMode();
        }
        else if (input[0] == 's') { // save
            ClearPrompt();
            std::cout << SetCursorCoordsStr(5, 2) << "File Name: ";
            std::string file;
            Quit();
            getline(std::cin, file);
            UnQuit();
            SaveFile(file);
            ClearLinesAndIndicators();
            RenderLineIndicators(text);
            RenderAllLines(text, 0);
        }
        else if (input[0] == 'o') { // open
            ClearPrompt();
            std::cout << SetCursorCoordsStr(5, 2) << "File Name: ";
            std::string potentialFile;
            Quit();
            getline(std::cin, potentialFile);
            UnQuit();
            OpenFile(potentialFile);

            ClearLinesAndIndicators();
            RenderLineIndicators(text);
            RenderAllLines(text, 0);


        }
        else if (input[0] == 'c') { // cmd
            ClearPrompt();
            std::cout << SetCursorCoordsStr(5, 2) << "Command: ";
            std::string cmd;
            Quit();
            getline(std::cin, cmd);
            ClearScreen();
            //SetCursorCoords(1,1);
            system(cmd.c_str());
            UnQuit();
            std::cout << "Command completed. Press any key to continue.";
            GetCharacter();

            ClearLinesAndIndicators();
            RenderLineIndicators(text);
            RenderAllLines(text, 0);
            
        }
        else if (input[0] == 'q') { // quit
            Quit();
            ClearScreen();
            return 0;
        }
        else {
            std::cout << SetCursorCoordsStr(1, termHeight - 1) << "Syntax Error. type ? for help";
        }
        
    }



    Quit();
    ClearScreen();
    return 0;
}