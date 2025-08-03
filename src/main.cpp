#include <string.h>
extern "C" {
#include "hardware/structs/bus_ctrl.h"
#include "pico/multicore.h"
#include "pico/stdio_uart.h"
#include "tusb.h"
}
#include "DVIDisplay.hpp"
#include "HIDKeyboard.hpp"
#include "font8x8.hpp"
#include "logo.hpp"

#define CHAR_ROWS 40
#define CHAR_COLS 60

static char screen[CHAR_ROWS][CHAR_COLS];

extern "C" {

// core 1: corre el despliegue DVI
void __not_in_flash("main") core1_entry() {
    // iniciamos el despliegue
    DVIDisplay::getInstance().start();

    // show time
    while (1) {
        __wfi();
    }
    __builtin_unreachable();
}

// core 0: corre el teclado USB y el resto de la logica
int __not_in_flash("main") main() {
    // inicializamos el control del despliegue HDMI/DVI
    DVIDisplay::getInstance().init(MODE_640x480_60Hz, (char *) &screen, CHAR_ROWS, CHAR_COLS, font8x8);

// inicializamos UART
#if RCR_DEBUG == 1
    stdio_uart_init();
    sleep_ms(100);
    printf("Comenzando\n");
#endif

    // inicializamos TinyUSB en modo host
    tusb_init();

    // lanzamos el core 1 con mayor prioridad
    hw_set_bits(&bus_ctrl_hw->priority, BUSCTRL_BUS_PRIORITY_PROC1_BITS);
    multicore_launch_core1(core1_entry);

    // llenamos la pantalla con digitos
    for (uint r = 0; r < CHAR_ROWS; r++) {
        for (uint c = 0; c < CHAR_COLS; c++) {
            screen[r][c] = '!' + c; //+ (c % 10);
        }
    }
    sleep_ms(2000);
    memset(screen, ' ', CHAR_ROWS * CHAR_COLS);

    // mostramos el logo
    for (uint r = 0; r < LOGO_ROWS; r++) {
        for (uint c = 0; c < LOGO_COLS; c++) {
            screen[r][(CHAR_COLS - LOGO_COLS) / 2 + c] = logo[r][c];
        }
    }
    sleep_ms(1000);
    memset(screen, ' ', CHAR_ROWS * CHAR_COLS);

    // show time
    HIDKeyboard &kbd = HIDKeyboard::getInstance();
    KeyEvent keyEvent;
    while (1) {
        tuh_task();
        keyEvent = kbd.getKeyEvent();
        if (keyEvent.ch) {
            screen[0][0] = keyEvent.ch;
#if RCR_DEBUG == 1
            printf("%c\n", keyEvent.ch);
#endif
        }
    }
    __builtin_unreachable();

    return 0;
}
}