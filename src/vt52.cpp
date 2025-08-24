#include <string.h>
#include "Terminal.hpp"

typedef enum { STATE_INIT = 0, STATE_Y_ROW, STATE_Y_COL } term_state_t;

bool Terminal::doVT52(char ch) {
    static term_state_t term_state = STATE_INIT;
    static uint16_t r = 0, c = 0;

    if (term_state == STATE_INIT) {
        // cursor up
        if (ch == 'A') {
            cursorUp();
        }
        // cursor down
        else if (ch == 'B') {
            cursorDown();
        }
        // cursor right
        else if (ch == 'C') {
            cursorRight();
        }
        // cursor down
        else if (ch == 'D') {
            cursorLeft();
        }
        // clear home
        else if (ch == 'E') {
            cls();
            cursorTo(0, 0);
        }
        // cursor to home
        else if (ch == 'H') {
            cursorTo(0, 0);
        }
        // reverse line feed
        else if (ch == 'I') {
            if (!cursorUp())
                insertLine();
        }
        // erase to end of screen
        else if (ch == 'J') {
            cls(curRow, curCol, rows - 1, cols - 1);
        }
        // erase to end of line
        else if (ch == 'K') {
            cls(curRow, curCol, curRow, cols - 1);
        }
        // insert blank line
        else if (ch == 'L') {
            insertLine();
        }
        // delete line
        else if (ch == 'M') {
            deleteLine();
        }
        // direct cursor address
        else if (ch == 'Y') {
            term_state = STATE_Y_ROW;
            return false;
        }
        // clear up to cursor
        else if (ch == 'd') {
            cls(0, 0, curRow, curCol);
        }
        // cursor on
        else if (ch == 'e') {
            setCursor(true);
        }
        // cursor off
        else if (ch == 'f') {
            setCursor(false);
        }
        // save cursor position
        else if (ch == 'j') {
            cursorSave();
        }
        // restore cursor to saved position
        else if (ch == 'k') {
            cursorRestore();
        }
        // clear line
        else if (ch == 'l') {
            cls(curRow, 0, curRow, cols - 1);
        }
        // clear from beginning of line
        else if (ch == 'o') {
            cls(curRow, 0, curRow, curCol);
        }
        // Switch on word wrap at line end
        else if (ch == 'v') {
            setWrap(true);
        }
        // Switch off word wrap at line end
        else if (ch == 'v') {
            setWrap(false);
        }
        // enter ANSI mode
        else if (ch == '<') {

        } else {
            doPrintableChar('[');
            doPrintableChar('^');
            doPrintableChar(ch);
            doPrintableChar(']');
        }
    }
    // direct cursor address
    else if (term_state == STATE_Y_ROW) {
        r          = ch - ' ';
        term_state = STATE_Y_COL;
        return false;
    }
    // direct cursor address
    else if (term_state == STATE_Y_COL) {
        c = ch - ' ';
        cursorTo(r, c);
    }

    term_state = STATE_INIT;
    return true;
}