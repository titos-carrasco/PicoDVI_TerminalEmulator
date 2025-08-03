extern "C" {
#include "tusb.h"
}
#include "HIDKeyboard.hpp"
#include "HIDKeyboard_US.hpp"

HIDKeyboard &HIDKeyboard::getInstance() {
    static HIDKeyboard instance;
    return instance;
}

KeyEvent HIDKeyboard::getKeyEvent() {
    KeyEvent _keyEvent = keyEvent;
    keyEvent           = {0, 0};
    return _keyEvent;
}
void HIDKeyboard::putKeyEvent(KeyEvent _keyEvent) {
    keyEvent = _keyEvent;
}

extern "C" {
void process_kbd_report(hid_keyboard_report_t const *report) {
    bool ctrl  = report->modifier & (KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_RIGHTCTRL);
    bool shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
    bool alt   = report->modifier & (KEYBOARD_MODIFIER_LEFTALT | KEYBOARD_MODIFIER_RIGHTALT);
    bool meta  = report->modifier & (KEYBOARD_MODIFIER_LEFTGUI | KEYBOARD_MODIFIER_RIGHTGUI);

    for (int i = 0; i < 6; i++) {
        uint8_t keycode = report->keycode[i];
        if (!keycode)
            continue;

        if (keycode > 127)
            continue;

        // normal, shift, control, alt, meta
        uint8_t ch;
        if (meta)
            ch = kbd_layout[keycode][4];
        else if (alt)
            ch = kbd_layout[keycode][3];
        else if (ctrl)
            ch = kbd_layout[keycode][2];
        else if (shift)
            ch = kbd_layout[keycode][1];
        else
            ch = kbd_layout[keycode][0];
        if (!ch)
            continue;

        KeyEvent keyEvent = {ch, report->modifier};
        HIDKeyboard::getInstance().putKeyEvent(keyEvent);
#if RCR_DEBUG == 1
        printf("KEY: %02X - CH: %02X - EVT.CH: %02X\n", keycode, ch, keyEvent.ch);
#endif
    }
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
    tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
    process_kbd_report((hid_keyboard_report_t const *) report);
    tuh_hid_receive_report(dev_addr, instance);
}
}
