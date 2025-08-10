#ifndef _RCR_VT100_HPP_
#define _RCR_VT100_HPP_

#include <stdint.h>

class VT100 {
  public:
    VT100(char *screen, uint16_t rows, uint16_t cols);
    void clearScreen();
    void print(char ch);
    void print(const char *s);
    void print(uint16_t row, uint16_t col, char ch);
    void print(uint16_t row, uint16_t col, char *s);
    void setPosition(uint16_t row, uint16_t col);
    void setRowPosition(uint16_t row);
    void setColPosition(uint16_t col);
    void cursorOn();
    void cursorOff();

  private:
    char *screen;
    uint16_t rows;
    uint16_t cols;
    uint16_t cur_row;
    uint16_t cur_col;
    bool mode_autoWrap;
    bool mode_newLine;
    bool mode_cursor;
    bool isPrintable(char ch);
    bool isControl(char ch);
    bool isEscSequence(char ch);
    void autoScroll();
};

#endif // _RCR_VT100_HPP_
