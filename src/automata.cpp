#include <string.h>
#include "vt100.hpp"

typedef enum {
    STATE_IDLE = 0,
    STATE_ESC,
    STATE_CSI,
    STATE_NUMBER1,
    STATE_SEMI,
    STATE_NUMBER2,
    STATE_QMARK,
    STATE_QMARK2,
    STATE_QMARK5
} vt_state_t;

bool VT100::isEscSequence(char ch) {
    static vt_state_t vt_state = STATE_IDLE;
    static int number1         = 0;
    static int number2         = 0;

    switch (vt_state) {
    case STATE_IDLE:
        if (ch == 0x1B) {
            vt_state = STATE_ESC;
            number1  = 0;
            number2  = 0;
        }
        break;

    case STATE_ESC:
        if (ch == '[') {
            vt_state = STATE_CSI;
        } else {
            vt_state = STATE_IDLE;
        }
        break;

    case STATE_CSI:
        if (ch >= '0' && ch <= '9') {
            number1  = ch - '0';
            vt_state = STATE_NUMBER1;
        } else if (ch == '?') {
            vt_state = STATE_QMARK;
        } else if (ch == 'K') {
            memset(screen + cur_row * cols + cur_col, ' ', cols - cur_col);
            vt_state = STATE_IDLE;
            return true;
        } else {
            vt_state = STATE_IDLE;
        }
        break;

    case STATE_NUMBER1:
        if (ch >= '0' && ch <= '9') {
            number1 = number1 * 10 + (ch - '0');
        } else if (ch == ';') {
            vt_state = STATE_SEMI;
        } else {
            vt_state = STATE_IDLE;
            if (ch == 'J') {
                if (number1 == 2) {
                    clearScreen();
                    return true;
                }
            }
        }
        break;

    case STATE_SEMI:
        if (ch >= '0' && ch <= '9') {
            number2  = ch - '0';
            vt_state = STATE_NUMBER2;
        } else {
            vt_state = STATE_IDLE;
        }
        break;

    case STATE_NUMBER2:
        if (ch >= '0' && ch <= '9') {
            number2 = number2 * 10 + (ch - '0');
        } else {
            vt_state = STATE_IDLE;
            if (ch == 'H') {
                setPosition(number1, number2);
                return true;
            }
        }
        break;

    case STATE_QMARK:
        if (ch == '2') {
            vt_state = STATE_QMARK2;
        } else {
            vt_state = STATE_IDLE;
        }
        break;

    case STATE_QMARK2:
        if (ch == '5') {
            vt_state = STATE_QMARK5;
        } else {
            vt_state = STATE_IDLE;
        }
        break;

    case STATE_QMARK5:
        vt_state = STATE_IDLE;
        break;
    }

    return vt_state != STATE_IDLE;
}
