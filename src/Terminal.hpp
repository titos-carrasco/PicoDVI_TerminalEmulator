#ifndef _RCR_TERMINAL_HPP_
#define _RCR_TERMINAL_HPP_

#include <stdint.h>

class Terminal {
  public:
    Terminal(char *screen, uint16_t rows, uint16_t cols);
    void reset();
    void cls();
    void cls(uint16_t fromRow, uint16_t fromCol, uint16_t toRow, uint16_t toCol);
    void insertLine();
    void deleteLine();
    void scrollUp();
    void print(char ch);
    void print(const char *s);
    void print(uint16_t row, uint16_t col, char ch);
    void print(uint16_t row, uint16_t col, char *s);
    void cursorTo(uint16_t row, uint16_t col);
    bool cursorUp();
    bool cursorDown();
    bool cursorLeft();
    bool cursorRight();
    void cursorSave();
    void cursorRestore();
    void setWrap(bool enable);
    void setCursor(bool enable);
    void setNewline(bool enable);

  private:
    char *screen;
    uint16_t rows;
    uint16_t cols;
    uint16_t curRow;
    uint16_t curCol;
    uint16_t curRow_s;
    uint16_t curCol_s;
    bool cursorOn;
    bool newlineOn;
    bool wrapOn;

    void doControlChar(char ch);
    void doPrintableChar(char ch);
    bool doVT52(char ch);
    bool doVT100(char ch);
};

#endif // _RCR_TERMINAL_HPP_
