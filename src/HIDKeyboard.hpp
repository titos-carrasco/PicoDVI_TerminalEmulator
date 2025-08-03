#ifndef _RCR_HIDKEYBOARD_HPP_
#define _RCR_HIDKEYBOARD_HPP_

#include <stdint.h>

struct KeyEvent {
    uint8_t ch;
    uint8_t modifier;
};

class HIDKeyboard {
  public:
    static HIDKeyboard &getInstance();
    KeyEvent getKeyEvent();
    void putKeyEvent(KeyEvent keyEvent);

    HIDKeyboard(const HIDKeyboard &)            = delete;
    HIDKeyboard &operator=(const HIDKeyboard &) = delete;

  private:
    HIDKeyboard()     = default;
    KeyEvent keyEvent = {0, 0};
};

#endif // _RCR_HIDKEYBOARD_HPP_