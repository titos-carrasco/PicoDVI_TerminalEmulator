#include <string.h>
#include "Terminal.hpp"

typedef enum { STATE_INIT = 0, STATE_Y_ROW, STATE_Y_COL } term_state_t;

bool Terminal::doVT52(char ch) {
    static term_state_t term_state = STATE_INIT;
    static uint16_t r = 0, c = 0;

    if (term_state == STATE_INIT) {
        if (ch == 'A') {
            cursorUp();
        } else if (ch == 'B') {
            cursorDown();
        } else if (ch == 'C') {
            cursorRight();
        } else if (ch == 'D') {
            cursorLeft();
        } else if (ch == 'H') {
            cursorTo(0, 0);
        } else if (ch == 'I') {
            cursorUp();
        } else if (ch == 'J') {
            cls(curRow, curCol, rows - 1, cols - 1);
        } else if (ch == 'K') {
            cls(curRow, curCol, curRow, cols - curCol - 1);
        } else if (ch == 'Y') {
            term_state = STATE_Y_ROW;
            return false;
        } else if (ch == '<') {
            setVT52Mode(false);
        } else {
            screen[curRow * cols + curCol] = ch;
            cursorRight();
        }
        return true;
    } else if (term_state == STATE_Y_ROW) {
        r          = ch - ' ';
        term_state = STATE_Y_COL;
        return false;
    } else if (term_state == STATE_Y_COL) {
        c = ch - ' ';
        cursorTo(r, c);
        term_state = STATE_INIT;
        return true;
    }

    return true;
}