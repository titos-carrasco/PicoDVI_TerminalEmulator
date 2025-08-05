#include <string.h>
#include "vt100.hpp"

VT100::VT100(char *screen, uint16_t rows, uint16_t cols) {
    this->screen  = screen;
    this->rows    = rows;
    this->cols    = cols;
    mode_autoWrap = true;
    mode_newLine  = false;
    clearScreen();
}

void VT100::clearScreen() {
    cur_row = 0;
    cur_col = 0;
    memset(screen, ' ', rows * cols);
}

void VT100::print(char ch) {
    if (isEscSequence(ch))
        return;
    if (isPrintable(ch))
        return;
    if (isControl(ch))
        return;
}

void VT100::print(char *s) {
    while (*s)
        print(*s++);
}

void VT100::print(uint16_t row, uint16_t col, char ch) {
    setPosition(row, col);
    print(ch);
}

void VT100::print(uint16_t row, uint16_t col, char *s) {
    setPosition(row, col);
    while (*s)
        print(*s++);
}

void VT100::setPosition(uint16_t row, uint16_t col) {
    setColPosition(col);
    setRowPosition(row);
}

void VT100::setRowPosition(uint16_t row) {
    if (row <= rows - 1)
        cur_row = row;
    else {
        cur_row = rows - 1;
        if (mode_autoWrap)
            autoScroll();
    }
}

void VT100::setColPosition(uint16_t col) {
    if (col <= cols - 1)
        cur_col = col;
    else {
        if (mode_autoWrap) {
            cur_col = 0;
            setRowPosition(cur_row + 1);
        } else
            cur_col = cols - 1;
    }
}

bool VT100::isPrintable(char ch) {
    if (ch < 0x20 || ch > 0x7E)
        return false;

    screen[cur_row * cols + cur_col] = ch;
    setColPosition(cur_col + 1);
    return true;
}

bool VT100::isControl(char ch) {
    // bell
    if (ch == 0x07) {
        // emitir beep
    }
    // backspace
    else if (ch == 0x08) {
        setColPosition(cur_col - 1);
    }
    // tab
    else if (ch == 0x09) {
        setColPosition((cur_col / 8 + 1) * 8);

    }
    // line feed (new line)
    else if (ch == 0x0A) {
        setRowPosition(cur_row + 1);
        if (mode_newLine)
            setColPosition(0);
    }
    // vertical tab
    else if (ch == 0x0B) {
        setRowPosition(cur_row + 1);
        if (mode_newLine)
            setColPosition(0);
    }
    // form feed
    else if (ch == 0x0C) {
        clearScreen();
    }
    // carriage return
    else if (ch == 0x0D) {
        setColPosition(0);
    } else
        return false;

    return true;
}

void VT100::autoScroll() {
    memmove(screen, screen + cols, rows * cols - cols);
    memset(screen + rows * cols - cols, ' ', cols);
}
