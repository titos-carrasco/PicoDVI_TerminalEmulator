#ifndef _RCR_TERMINAL_HPP_
#define _RCR_TERMINAL_HPP_

#include <stdint.h>

class Terminal {
  public:
    Terminal(char *screen, uint16_t rows, uint16_t cols);
    virtual void reset();
    virtual void cls();
    virtual void cls(uint16_t row1, uint16_t col1, uint16_t row2, uint16_t col2);
    virtual void print(char ch);
    virtual void print(const char *s);
    virtual void print(uint16_t row, uint16_t col, char ch);
    virtual void print(uint16_t row, uint16_t col, char *s);
    virtual void cursorTo(uint16_t row, uint16_t col);
    virtual void cursorUp();
    virtual void cursorDown();
    virtual void cursorLeft();
    virtual void cursorRight();
    virtual void cursorSave();
    virtual void cursorRestore();
    virtual void setWrap(bool enable);
    virtual void setCursor(bool enable);
    virtual void setScroll(bool enable);
    virtual void setNewline(bool enable);
    virtual void setVT52Mode(bool enable);

  private:
    char *screen;
    uint16_t rows;
    uint16_t cols;
    uint16_t curRow;
    uint16_t curCol;
    uint16_t curRow_s;
    uint16_t curCol_s;
    bool wrapOn;
    bool scrollOn;
    bool cursorOn;
    bool newlineOn;
    bool vt52ModeOn;

    void doControlChar(char ch);
    void doPrintableChar(char ch);
    bool doVT52(char ch);
    bool doVT100(char ch);
};

#endif // _RCR_TERMINAL_HPP_
