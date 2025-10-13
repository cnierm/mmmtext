#include "../include/Line.hpp"


Line::Line() {
    lineData = "";
    numVisualLinesUsed = 1;
}

Line::Line(std::string text) {
    lineData = text;
    numVisualLinesUsed = (text.size() / (termWidth - 2)) + 1; // Should be integer division
}

int Line::InsertCharacter(int n, char c) {
    lineData.insert(n, 1, c);
    if (lineData.size() > numVisualLinesUsed * (termWidth - 2)) { // If the line now takes more lines than its current numVisualLinesUsed.
        ++numVisualLinesUsed;                                     // This is more efficient than a recalculation.
        return 1;
    }
    return 0;
}

int Line::DeleteCharacter(int n) {
    lineData.erase(n, 1);
    if (lineData.size() / (termWidth - 2) + 1 != numVisualLinesUsed) { // If numVisualLinesUsed is different upon recalcution. 
        --numVisualLinesUsed;
        return 1;
    }
    return 0;    
}