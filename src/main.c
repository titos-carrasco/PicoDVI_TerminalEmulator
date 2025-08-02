#include <string.h>

#include "hardware/structs/bus_ctrl.h"
#include "pico.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "tusb.h"

#include "display.h"
#include "logo.h"

static uint8_t const keycode2ascii[128][2] = {HID_KEYCODE_TO_ASCII};

// cuando una tecla es presionada
void process_kbd_report(hid_keyboard_report_t const *report) {
    bool ctrl  = report->modifier & (0x11); // Ctrl izquierdo o derecho
    bool shift = report->modifier & (0x22); // Shift izquierdo o derecho
    bool alt   = report->modifier & (0x44); // Ctrl izquierdo o derecho
    bool meta  = report->modifier & (0x88); // Meta izquierdo o derecho

    for (int i = 0; i < 6; i++) {
        uint8_t keycode = report->keycode[i];
        if (!keycode)
            continue;

        if (keycode > 127)
            continue;

        char ch = keycode2ascii[keycode][0];
        if (!ch)
            continue;

        if (meta || alt) {
            continue;
        } else if (shift) {
            continue;
        } else if (ctrl) {
            continue;
        }

        display_putch(ch);
    }
}

// cuando se conecta un teclado
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
    tuh_hid_receive_report(dev_addr, instance);
}

// cuando se desconecta un teclado
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
}

// reporte de un nuevo dispositivo HID
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
    process_kbd_report((hid_keyboard_report_t const *) report);
    tuh_hid_receive_report(dev_addr, instance);
}

// main del core 1
void __not_in_flash("main") core1_main() {
    // inicia el despliegue
    display_start();

    // show time para el core 1
    while (1) {
        __wfi();
    }
    __builtin_unreachable();
}

// main del core 0
int __not_in_flash("main") main() {
    // prepara el despliegue
    display_init();

    // inicializamos TinyUSB en modo host
    // board_init();
    tusb_init();

    // lanza el core 1 con mayor prioridad
    hw_set_bits(&bus_ctrl_hw->priority, BUSCTRL_BUS_PRIORITY_PROC1_BITS);
    multicore_launch_core1(core1_main);

    // llenamos la pantalla con digitos
    for (uint r = 0; r < CHAR_ROWS; r++) {
        for (uint c = 0; c < CHAR_COLS; c++) {
            display_buffer[r][c] = '!' + c; //+ (c % 10);
        }
    }
    sleep_ms(2000);
    memset(display_buffer, ' ', CHAR_ROWS * CHAR_COLS);

    // mostramos el logo*-
    for (uint r = 0; r < LOGO_ROWS; r++) {
        for (uint c = 0; c < LOGO_COLS; c++) {
            display_buffer[r][(CHAR_COLS - LOGO_COLS) / 2 + c] = logo[r][c];
        }
    }
    sleep_ms(1000);
    memset(display_buffer, ' ', CHAR_ROWS * CHAR_COLS);

    // show time para el core 0
    while (1) {
        // tinyusb host task
        tuh_task();

        //__wfi();
    }
    __builtin_unreachable();

    return 1;
}
