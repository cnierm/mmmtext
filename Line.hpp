#ifndef LINE_HPP
#define LINE_HPP

#include <string>

extern int termWidth; // get termWidth from main cpp


class Line {
    private:
        std::string lineData;
        int numVisualLinesUsed;
    public:
        Line();
        Line(std::string text);
        int GetNumVisualLinesUsed() {return numVisualLinesUsed;}
        std::string GetLineData() {return lineData;}
        int InsertCharacter(int n, char c); // int so that if numVisualLinesUsed changes, WriteMode() will know to rerender the whole text.
        int DeleteCharacter(int n);

};

#endif // LINE_HPP