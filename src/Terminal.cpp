#include <string.h>
#include "Terminal.hpp"

Terminal::Terminal(char *screen, uint16_t rows, uint16_t cols) {
    this->screen = screen;
    this->rows   = rows;
    this->cols   = cols;
    reset();
}

void Terminal::reset() {
    curRow     = 0;
    curCol     = 0;
    curRow_s   = 0;
    curCol_s   = 0;
    wrapOn     = true;
    scrollOn   = true;
    cursorOn   = true;
    newlineOn  = false;
    vt52ModeOn = true;
}

void Terminal::cls() {
    memset(screen, ' ', rows * cols);
}

void Terminal::cls(uint16_t row1, uint16_t col1, uint16_t row2, uint16_t col2) {
    if (row1 > row2) {
        uint16_t tmp = row1;
        row1         = row2;
        row2         = tmp;
    }
    if (col1 > col2) {
        uint16_t tmp = col1;
        col1         = col2;
        col2         = tmp;
    }

    if (row2 >= rows)
        row2 = rows - 1;
    if (col2 >= cols)
        col2 = cols - 1;

    uint16_t width = col2 - col1 + 1;

    for (uint16_t r = row1; r <= row2; r++) {
        char *line = screen + r * cols + col1;
        memset(line, ' ', width);
    }
}

typedef enum { STATE_NORMAL, STATE_ESC, STATE_VT100, STATE_VT52 } term_state_t;

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
        screen[curRow * cols + curCol] = '?';
        cursorRight();
        return;
    }

    if (term_state == STATE_ESC) {
        if (ch == '[') {
            term_state = STATE_VT100;
            return;
        } else if (vt52ModeOn) {
            term_state = STATE_VT52;
            if (doVT52(ch))
                term_state = STATE_NORMAL;
            return;
        }

        screen[curRow * cols + curCol] = '?';
        cursorRight();
        term_state = STATE_NORMAL;
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

    screen[curRow * cols + curCol] = ch;
    cursorRight();
    term_state = STATE_NORMAL;
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

void Terminal::cursorUp() {
    if (curRow > 0)
        curRow--;
}

void Terminal::cursorDown() {
    if (curRow < rows - 1)
        curRow++;
    else if (scrollOn) {
        memmove(screen, screen + cols, (rows - 1) * cols);
        memset(screen + (rows - 1) * cols, ' ', cols);
    }
}

void Terminal::cursorLeft() {
    if (curCol > 0)
        curCol--;
}

void Terminal::cursorRight() {
    if (curCol < cols - 1)
        curCol++;
    else if (wrapOn) {
        curCol = 0;
        cursorDown();
    }
}

void Terminal::cursorSave() {
    curRow_s = curRow;
    curCol_s = curCol;
}

void Terminal::cursorRestore() {
    curRow = curRow_s;
    curCol = curCol_s;
}

void Terminal::setWrap(bool enable) {
    wrapOn = enable;
}

void Terminal::setCursor(bool enable) {
    cursorOn = enable;
}

void Terminal::setScroll(bool enable) {
    scrollOn = enable;
}

void Terminal::setNewline(bool enable) {
    newlineOn = enable;
}

void Terminal::setVT52Mode(bool enable) {
    vt52ModeOn = enable;
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
    // tab
    else if (ch == 0x09) {
        int tabSize = 8;
        int steps   = ((curCol / tabSize) + 1) * tabSize - curCol;
        for (int i = 0; i < steps; i++)
            cursorRight();
    }
    // line feed (new line) o form feed
    else if (ch == 0x0A || ch == 0x0C) {
        cursorDown();
        if (newlineOn)
            curCol = 0;
    }
    // carriage return
    else if (ch == 0x0D) {
        curCol = 0;
    }
    // shift out
    else if (ch == 0x0E) {
    }
    // shift in
    else if (ch == 0x0F) {
    }
}

void Terminal::doPrintableChar(char ch) {
    screen[curRow * cols + curCol] = ch;
    cursorRight();
}
