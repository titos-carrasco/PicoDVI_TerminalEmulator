#ifndef _RCR_HIDKEYBOARD_HPP_
#define _RCR_HIDKEYBOARD_HPP_

#include <stdint.h>
#include "Queue.hpp"

struct KeyEvent {
    uint8_t ch;
    uint8_t modifier;
};

class HIDKeyboard {
  public:
    static HIDKeyboard &getInstance();
    KeyEvent getKeyEvent();
    void putKeyEvent(KeyEvent keyEvent);
    static const uint8_t Capacity = 24;

    HIDKeyboard(const HIDKeyboard &)            = delete;
    HIDKeyboard &operator=(const HIDKeyboard &) = delete;

  private:
    HIDKeyboard() = default;
    Queue<KeyEvent, Capacity> keyEvents;
};

#endif // _RCR_HIDKEYBOARD_HPP_