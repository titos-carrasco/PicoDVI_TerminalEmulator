#include <string.h>
#include "vt100.hpp"

typedef enum {
    STATE_NORMAL = 0, //
    STATE_ESC,        // ESC
    STATE_CSI,        // ESC[
    STATE_NUMBER1,    // ESC[n
    STATE_SEMI,       // ESC[n;
    STATE_NUMBER2,    // ESC[n;m
    STATE_QMARK,      // ESC[?
    STATE_QMARK2,     // ESC[?2
    STATE_QMARK5,     // ESC[?25
} vt_state_t;

bool VT100::isEscSequence(char ch) {
    static vt_state_t vt_state = STATE_NORMAL;
    static int number1         = 0;
    static int number2         = 0;
    static uint16_t save_row   = 0;
    static uint16_t save_col   = 0;

    if (vt_state == STATE_NORMAL) {
        // ESC
        if (ch == 0x1B) {
            number1  = 0;
            number2  = 0;
            save_row = 0;
            save_col = 0;
            vt_state = STATE_ESC;
            return true;
        }
        return false;
    }

    if (vt_state == STATE_ESC) {
        // ESC[
        if (ch == '[') {
            vt_state = STATE_CSI;
            return true;
        }
    }

    else if (vt_state == STATE_CSI) {
        // ESC[n
        if (ch >= '0' && ch <= '9') {
            number1  = ch - '0';
            vt_state = STATE_NUMBER1;
            return true;
        }
        // ESC[A cursor arriba
        if (ch == 'A') {
            setRowPosition(cur_row - 1);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[B cursor abajo
        if (ch == 'B') {
            setRowPosition(cur_row + 1);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[C cursor adelante
        if (ch == 'C') {
            setColPosition(cur_col + 1);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[D cursor atras
        if (ch == 'D') {
            setColPosition(cur_col - 1);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[E cursor inicio siguiente linea
        if (ch == 'E') {
            setPosition(cur_row + 1, 0);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[F cursor inicio linea anterior
        if (ch == 'F') {
            setPosition(cur_row - 1, 0);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[G cursor a columna
        if (ch == 'G') {
            setColPosition(0);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[H cursor a inicio
        if (ch == 'H') {
            setPosition(10, 10);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[J limpia pantalla
        if (ch == 'J') {
            // desde el cursor hasta el final de pantalla
            int pos = cur_row * cols + cur_col;
            memset(screen + pos, ' ', rows * cols - pos);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[K limpia la linea
        if (ch == 'K') {
            // desde el cursor hasta fin de linea
            int pos = cur_row * cols + cur_col;
            memset(screen + pos, ' ', cols - cur_col);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[s guarda posicion del cursor
        if (ch == 's') {
            save_col = cur_col;
            save_row = cur_row;
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[u restaura posicion del cursor
        if (ch == 'u') {
            cur_col  = save_col;
            cur_row  = save_row;
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[? visibilidad del cursor
        if (ch == '?') {
            vt_state = STATE_QMARK;
            return true;
        }
    }

    else if (vt_state == STATE_NUMBER1) {
        // ESC[n
        if (ch >= '0' && ch <= '9') {
            number1 = number1 * 10 + (ch - '0');
            return true;
        }
        // ESC[n;
        if (ch == ';') {
            vt_state = STATE_SEMI;
            return true;
        }
        // ESC[nA cursor arriba
        if (ch == 'A') {
            setRowPosition(cur_row - number1);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[nB cursor abajo
        if (ch == 'B') {
            setRowPosition(cur_row + number1);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[nC cursor adelante
        if (ch == 'C') {
            setColPosition(cur_col + number1);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[nD cursor atras
        if (ch == 'D') {
            setColPosition(cur_col - number1);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[nE inicio de la siguiente N linea
        if (ch == 'E') {
            setPosition(cur_row + number1, 0);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[nF inicio de la anterior N linea
        if (ch == 'F') {
            setPosition(cur_row - number1, 0);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[G cursor a columna
        if (ch == 'G') {
            if (number1 == 0)
                number1 = 1;
            setColPosition(number1 - 1);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[nH cursor a posicion
        if (ch == 'H') {
            if (number1 == 0)
                number1 = 1;
            setPosition(number1 - 1, 0);
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[nJ limpia pantalla
        if (ch == 'J') {
            int pos = cur_row * cols + cur_col;

            // ESC[0J desde el cursor a fin de pantalla
            if (number1 == 0) {
                memset(screen + pos, ' ', rows * cols - pos);
                vt_state = STATE_NORMAL;
                return true;
            }
            // ESC[1J desde el cursor hasta inicio de pantalla
            if (number1 == 1) {
                memset(screen, ' ', pos + 1);
                vt_state = STATE_NORMAL;
                return true;
            }
            // ESC[2J toda la pantalla
            if (number1 == 2) {
                memset(screen, ' ', rows * cols);
                vt_state = STATE_NORMAL;
                return true;
            }
        }
        // ESC[nK limpia la linea
        if (ch == 'K') {
            int pos = cur_row * cols + cur_col;

            // ESC[0K desde el cursor hasta el fin de linea
            if (number1 == 0) {
                // from cursor to EOL
                memset(screen + pos, ' ', cols - cur_col);
                vt_state = STATE_NORMAL;
                return true;
            }
            // ESC[1K desde el cursor hasta el inicio de la linea
            if (number1 == 1) {
                memset(screen + cur_row * cols, ' ', cur_col + 1);
                vt_state = STATE_NORMAL;
                return true;
            }
            // ESC[2K toda la linea
            if (number1 == 2) {
                memset(screen + cur_row * cols, ' ', cols);
                vt_state = STATE_NORMAL;
                return true;
            }
        }
    }

    else if (vt_state == STATE_SEMI) {
        // ESC[n;n
        if (ch >= '0' && ch <= '9') {
            number2  = ch - '0';
            vt_state = STATE_NUMBER2;
            return true;
        }
        // ESC[n;H cursor a posicion
        if (ch == 'H') {
            if (number1 == 0)
                number1 = 1;
            setPosition(number1 - 1, 0);
            vt_state = STATE_NORMAL;
            return true;
        }
    }

    else if (vt_state == STATE_NUMBER2) {
        // ESC[n;n
        if (ch >= '0' && ch <= '9') {
            number2 = number2 * 10 + (ch - '0');
            return true;
        }
        // ESC[n;nH cursor a posicion
        if (ch == 'H') {
            if (number1 == 0)
                number1 = 1;
            if (number2 == 0)
                number2 = 1;
            setPosition(number1 - 1, number2 - 1);
            vt_state = STATE_NORMAL;
            return true;
        }
    }

    else if (vt_state == STATE_QMARK) {
        // ESC[?2 visibilidad del cursor
        if (ch == '2') {
            vt_state = STATE_QMARK2;
            return true;
        }
    }

    else if (vt_state == STATE_QMARK2) {
        // ESC[?25 visibilidad del cursor
        if (ch == '5') {
            vt_state = STATE_QMARK5;
            return true;
        }
    }

    else if (vt_state == STATE_QMARK5) {
        // ESC[?25h cursor on
        if (ch == 'h') {
            cursorOn();
            vt_state = STATE_NORMAL;
            return true;
        }
        // ESC[?25l cursor on
        if (ch == 'l') {
            cursorOff();
            vt_state = STATE_NORMAL;
            return true;
        }
    }

    vt_state = STATE_NORMAL;
    return false;
}
