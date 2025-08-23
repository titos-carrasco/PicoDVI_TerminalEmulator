#include <string.h>
#include "Terminal.hpp"

typedef enum {
    STATE_INIT = 0, //
    STATE_NUMBER1,  // ESC[n
    STATE_SEMI,     // ESC[n;
    STATE_NUMBER2,  // ESC[n;m
    STATE_QMARK,    // ESC[?
    STATE_QMARK2,   // ESC[?2
    STATE_QMARK5,   // ESC[?25
} term_state_t;

bool Terminal::doVT100(char ch) {
    static term_state_t term_state = STATE_INIT;
    static int number1             = 0;
    static int number2             = 0;

    if (term_state == STATE_INIT) {
        number1 = 0;
        number2 = 0;

        // ESC[n
        if (ch >= '0' && ch <= '9') {
            number1    = ch - '0';
            term_state = STATE_NUMBER1;
            return false;
        }
        // ESC[A cursor arriba
        else if (ch == 'A') {
            cursorUp();
        }
        // ESC[B cursor abajo
        else if (ch == 'B') {
            cursorDown();
        }
        // ESC[C cursor adelante
        else if (ch == 'C') {
            cursorRight();
        }
        // ESC[D cursor atras
        else if (ch == 'D') {
            cursorLeft();
        }
        // ESC[E cursor inicio siguiente linea
        else if (ch == 'E') {
            cursorTo(curRow, 0);
            cursorDown();
        }
        // ESC[F cursor inicio linea anterior
        else if (ch == 'F') {
            cursorTo(curRow, 0);
            cursorUp();
        }
        // ESC[G cursor a columna
        else if (ch == 'G') {
            cursorTo(curRow, 0);
        }
        // ESC[H cursor a inicio
        else if (ch == 'H') {
            cursorTo(0, 0);
        }
        // ESC[J limpia pantalla
        else if (ch == 'J') {
            // desde el cursor hasta el final de pantalla
            cls(curRow, curCol, rows - 1, cols - 1);
        }
        // ESC[K limpia la linea
        else if (ch == 'K') {
            // desde el cursor hasta fin de linea
            cls(curRow, curCol, curRow, cols - 1);
        }
        // ESC[s guarda posicion del cursor
        else if (ch == 's') {
            cursorSave();
        }
        // ESC[u restaura posicion del cursor
        else if (ch == 'u') {
            cursorRestore();
        }
        // ESC[? visibilidad del cursor
        else if (ch == '?') {
            term_state = STATE_QMARK;
            return false;
        }

        return true;
    }

    else if (term_state == STATE_NUMBER1) {
        // ESC[n
        if (ch >= '0' && ch <= '9') {
            number1 = number1 * 10 + (ch - '0');
            return false;
        }
        // ESC[n;
        else if (ch == ';') {
            term_state = STATE_SEMI;
            return false;
        }
        // ESC[nA cursor arriba
        else if (ch == 'A') {
            while (number1--)
                cursorUp();
        }
        // ESC[nB cursor abajo
        else if (ch == 'B') {
            while (number1--)
                cursorDown();
        }
        // ESC[nC cursor adelante
        else if (ch == 'C') {
            while (number1--)
                cursorRight();
        }
        // ESC[nD cursor atras
        else if (ch == 'D') {
            while (number1--)
                cursorLeft();
        }
        // ESC[nE inicio de la siguiente N linea
        else if (ch == 'E') {
            cursorTo(curRow, 0);
            while (number1--)
                cursorDown();
        }
        // ESC[nF inicio de la anterior N linea
        else if (ch == 'F') {
            cursorTo(curRow, 0);
            while (number1--)
                cursorUp();
        }
        // ESC[G cursor a columna
        else if (ch == 'G') {
            cursorTo(curRow, number1 == 0 ? number1 : number1 - 1);
        }
        // ESC[nH cursor a posicion
        else if (ch == 'H') {
            cursorTo(number1 == 0 ? number1 : number1 - 1, curCol);
        }
        // ESC[nJ limpia pantalla
        else if (ch == 'J') {
            // ESC[0J desde el cursor a fin de pantalla
            if (number1 == 0) {
                cls(curRow, curCol, rows - 1, cols - 1);
            }
            // ESC[1J desde el cursor hasta inicio de pantalla
            else if (number1 == 1) {
                cls(curRow, curCol, 0, 0);
            }
            // ESC[2J toda la pantalla
            else if (number1 == 2) {
                cls();
            }
        }
        // ESC[nK limpia la linea
        else if (ch == 'K') {
            // ESC[0K desde el cursor hasta el fin de linea
            if (number1 == 0) {
                cls(curRow, curCol, curRow, cols - 1);
            }
            // ESC[1K desde el cursor hasta el inicio de la linea
            else if (number1 == 1) {
                cls(curRow, curCol, curRow, 0);
            }
            // ESC[2K toda la linea
            else if (number1 == 2) {
                cls(curRow, 0, curRow, cols - 1);
            }
        }
        term_state = STATE_INIT;
        return true;
    }

    else if (term_state == STATE_SEMI) {
        // ESC[n;n
        if (ch >= '0' && ch <= '9') {
            number2    = ch - '0';
            term_state = STATE_NUMBER2;
            return false;
        }
        // ESC[n;H cursor a posicion
        else if (ch == 'H') {
            cursorTo(number1 == 0 ? number1 : number1 - 1, 0);
        }

        term_state = STATE_INIT;
        return true;
    }

    else if (term_state == STATE_NUMBER2) {
        // ESC[n;n
        if (ch >= '0' && ch <= '9') {
            number2 = number2 * 10 + (ch - '0');
            return false;
        }
        // ESC[n;nH cursor a posicion
        else if (ch == 'H') {
            cursorTo(number1 == 0 ? number1 : number1 - 1, number2 == 0 ? number2 : number2 - 1);
        }

        term_state = STATE_INIT;
        return true;
    }

    else if (term_state == STATE_QMARK) {
        // ESC[?2 visibilidad del cursor
        if (ch == '2') {
            term_state = STATE_QMARK2;
            return false;
        }

        term_state = STATE_INIT;
        return true;
    }

    else if (term_state == STATE_QMARK2) {
        // ESC[?25 visibilidad del cursor
        if (ch == '5') {
            term_state = STATE_QMARK5;
            return false;
        }

        term_state = STATE_INIT;
        return true;
    }

    else if (term_state == STATE_QMARK5) {
        // ESC[?25h cursor on
        if (ch == 'h') {
            setCursor(true);
        }
        // ESC[?25l cursor on
        if (ch == 'l') {
            setCursor(false);
        }

        term_state = STATE_INIT;
        return true;
    }

    term_state = STATE_INIT;
    return true;
}
