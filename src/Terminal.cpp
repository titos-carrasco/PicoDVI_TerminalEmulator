#include <string.h>
#include "Terminal.hpp"

typedef enum { STATE_NORMAL, STATE_ESC, STATE_VT100, STATE_VT52 } term_state_t;

Terminal::Terminal(char *screen, uint16_t rows, uint16_t cols) {
    this->screen = screen;
    this->rows   = rows;
    this->cols   = cols;
    reset();
}

void Terminal::reset() {
    curRow    = 0;
    curCol    = 0;
    curRow_s  = 0;
    curCol_s  = 0;
    cursorOn  = true;
    newlineOn = false;
    wrapOn    = true;
}

void Terminal::cls() {
    memset(screen, ' ', rows * cols);
}

void Terminal::cls(uint16_t fromRow, uint16_t fromCol, uint16_t toRow, uint16_t toCol) {
    uint16_t pos1 = fromRow * cols + fromCol;
    uint16_t pos2 = toRow * cols + toCol;

    memset(screen + pos1, ' ', pos2 - pos1 + 1);
}

void Terminal::insertLine() {
    if (curRow != rows - 1) {
        memmove(screen + (curRow + 1) * cols, screen + curRow * cols, (rows - curRow) * cols - cols);
    }
    memset(screen + curRow * cols, ' ', cols);
}

void Terminal::deleteLine() {
    if (curRow != rows - 1) {
        memmove(screen + curRow * cols, screen + (curRow + 1) * cols, (rows - curRow) * cols - cols);
    }
    memset(screen + (rows - 1) * cols, ' ', cols);
}

void Terminal::scrollUp() {
    memmove(screen, screen + cols, (rows - 1) * cols);
    memset(screen + (rows - 1) * cols, ' ', cols);
}

void Terminal::print(char ch) {
    static term_state_t term_state = STATE_NORMAL;

    if (term_state == STATE_NORMAL) {
        // inicio secuencia de escape
        if (ch == '\e') {
            term_state = STATE_ESC;
            return;
        }

        // 0 a 31
        if (ch >= 0 && ch <= 31) {
            doControlChar(ch);
            return;
        }

        // 32 a 127
        if (ch >= ' ' && ch <= '~') {
            doPrintableChar(ch);
            return;
        }

        // 128 a 255
        doPrintableChar('~');
        return;
    }

    if (term_state == STATE_ESC) {
        if (ch == '[') {
            term_state = STATE_VT100;
            return;
        }

        if (doVT52(ch))
            term_state = STATE_NORMAL;
        else
            term_state = STATE_VT52;
        return;
    }

    if (term_state == STATE_VT100) {
        if (doVT100(ch))
            term_state = STATE_NORMAL;
        return;
    }

    if (term_state == STATE_VT52) {
        if (doVT52(ch))
            term_state = STATE_NORMAL;
        return;
    }
}

void Terminal::print(const char *s) {
    while (*s)
        print(*s++);
}

void Terminal::print(uint16_t row, uint16_t col, char ch) {
    cursorTo(row, col);
    print(ch);
}

void Terminal::print(uint16_t row, uint16_t col, char *s) {
    cursorTo(row, col);
    while (*s)
        print(*s++);
}

void Terminal::cursorTo(uint16_t row, uint16_t col) {
    if (row <= rows - 1)
        curRow = row;
    if (col <= cols - 1)
        curCol = col;
}

bool Terminal::cursorUp() {
    if (curRow == 0)
        return false;
    curRow--;
    return true;
}

bool Terminal::cursorDown() {
    if (curRow == rows - 1)
        return false;
    curRow++;
    return true;
}

bool Terminal::cursorLeft() {
    if (curCol == 0)
        return false;
    curCol--;
    return true;
}

bool Terminal::cursorRight() {
    if (curCol == cols - 1)
        return false;
    curCol++;
    return true;
}

void Terminal::cursorSave() {
    curRow_s = curRow;
    curCol_s = curCol;
}

void Terminal::cursorRestore() {
    curRow = curRow_s;
    curCol = curCol_s;
}

void Terminal::setCursor(bool enable) {
    cursorOn = enable;
}

void Terminal::setNewline(bool enable) {
    newlineOn = enable;
}

void Terminal::setWrap(bool enable) {
    wrapOn = enable;
}

void Terminal::doControlChar(char ch) {
    // bell
    if (ch == 0x07) {
        // beep
    }
    // backspace
    else if (ch == 0x08) {
        cursorLeft();
    }
    // horizontal tab
    else if (ch == 0x09) {
        int tabSize = 8;
        int steps   = ((curCol / tabSize) + 1) * tabSize - curCol;
        for (int i = 0; i < steps; i++)
            cursorRight();
    }
    // line feed (new line) o vertical tab o form feed
    else if (ch == 0x0A || ch == 0x0B || ch == 0x0C) {
        if (!cursorDown())
            scrollUp();

        if (newlineOn)
            curCol = 0;
    }
    // carriage return
    else if (ch == 0x0D) {
        curCol = 0;
    }
    // activate the G1 character set
    else if (ch == 0x0E) {

    }
    // activate the G0 character set
    else if (ch == 0x0F) {

    }
    // otro
    else {
        doPrintableChar('(');
        doPrintableChar('^');
        doPrintableChar(ch);
        doPrintableChar(')');
    }
}

void Terminal::doPrintableChar(char ch) {
    screen[curRow * cols + curCol] = ch;
    if (!cursorRight())
        if (wrapOn) {
            curCol = 0;
            if (!cursorDown())
                scrollUp();
        }
}
