#ifndef _RCR_VT100_HPP_
#define _RCR_VT100_HPP_

#include <stdint.h>
#include <string.h>

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
class VT100 {
  public:
    static VT100 &getInstance();

    VT100(const VT100 &)            = delete;
    VT100 &operator=(const VT100 &) = delete;

    char (*getScreen())[SCREEN_COLS];
    void clearScreen();
    void print(char ch);
    void print(char *s);
    void print(uint16_t row, uint16_t col, char ch);
    void print(uint16_t row, uint16_t col, char *s);
    void setRowPosition(uint16_t row);
    void setColPosition(uint16_t col);

  private:
    VT100();
    void autoScroll();
    bool isPrintable(char ch);
    bool isControl(char ch);
    bool isEscSequence(char ch);
    char screen[SCREEN_ROWS][SCREEN_COLS];
    uint16_t cur_row;
    uint16_t cur_col;
    bool mode_autoWrap;
    bool mode_newLine;
};

// implementacion

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
VT100<SCREEN_ROWS, SCREEN_COLS>::VT100() {
    mode_autoWrap = true;
    mode_newLine  = false;
    clearScreen();
};

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
VT100<SCREEN_ROWS, SCREEN_COLS> &VT100<SCREEN_ROWS, SCREEN_COLS>::getInstance() {
    static VT100<SCREEN_ROWS, SCREEN_COLS> instance;
    return instance;
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
char (*VT100<SCREEN_ROWS, SCREEN_COLS>::getScreen())[SCREEN_COLS] {
    return screen;
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
void VT100<SCREEN_ROWS, SCREEN_COLS>::clearScreen() {
    cur_row = 0;
    cur_col = 0;
    memset(screen, ' ', SCREEN_ROWS * SCREEN_COLS);
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
void VT100<SCREEN_ROWS, SCREEN_COLS>::print(char ch) {
    if (isPrintable(ch))
        return;
    if (isControl(ch))
        return;
    if (isEscSequence(ch))
        return;
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
void VT100<SCREEN_ROWS, SCREEN_COLS>::print(char *s) {
    while (*s)
        print(*s++);
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
void VT100<SCREEN_ROWS, SCREEN_COLS>::print(uint16_t row, uint16_t col, char ch) {
    setRowPosition(row);
    setColPosition(col);
    print(ch);
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
void VT100<SCREEN_ROWS, SCREEN_COLS>::print(uint16_t row, uint16_t col, char *s) {
    setRowPosition(row);
    setColPosition(col);
    while (*s)
        print(*s++);
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
void VT100<SCREEN_ROWS, SCREEN_COLS>::autoScroll() {
    char *scr = &screen[0][0];
    memmove(scr, scr + SCREEN_COLS, SCREEN_ROWS * SCREEN_COLS - SCREEN_COLS);
    memset(scr + SCREEN_ROWS * SCREEN_COLS - SCREEN_COLS, ' ', SCREEN_COLS);
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
void VT100<SCREEN_ROWS, SCREEN_COLS>::setRowPosition(uint16_t row) {
    if (row <= SCREEN_ROWS - 1)
        cur_row = row;
    else {
        cur_row = SCREEN_ROWS - 1;
        if (mode_autoWrap)
            autoScroll();
    }
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
void VT100<SCREEN_ROWS, SCREEN_COLS>::setColPosition(uint16_t col) {
    if (col <= SCREEN_COLS - 1)
        cur_col = col;
    else {
        if (mode_autoWrap) {
            cur_col = 0;
            setRowPosition(cur_row + 1);
        } else
            cur_col = SCREEN_COLS - 1;
    }
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
bool VT100<SCREEN_ROWS, SCREEN_COLS>::isPrintable(char ch) {
    if (ch < 0x20 || ch > 0x7E)
        return false;

    screen[cur_row][cur_col] = ch;
    setColPosition(cur_col + 1);
    return true;
}

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
bool VT100<SCREEN_ROWS, SCREEN_COLS>::isControl(char ch) {
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

template <uint16_t SCREEN_ROWS, uint16_t SCREEN_COLS>
bool VT100<SCREEN_ROWS, SCREEN_COLS>::isEscSequence(char ch) {
    return false;
}

#endif // _RCR_VT100_HPP_
