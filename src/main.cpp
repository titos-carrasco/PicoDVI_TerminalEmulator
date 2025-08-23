extern "C" {
#include "pico/multicore.h"
#include "hardware/structs/bus_ctrl.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "tusb.h"
}
#include "DVIDisplay.hpp"
#include "HIDKeyboard.hpp"
#include "Terminal.hpp"
#include "font8x8.hpp"
#include "logo.hpp"

// pareciera que tenemos mejor rendimiento con esto
#define UART_ID     uart0
#define UART_BAUD   115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// el buffer de pantalla
#define SCREEN_ROWS 40
#define SCREEN_COLS 80
char *screen[SCREEN_ROWS][SCREEN_COLS];

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
    // el emulador vt100
    Terminal terminal((char *) screen, SCREEN_ROWS, SCREEN_COLS);

    // inicializamos el control del despliegue HDMI/DVI
    DVIDisplay::getInstance().init(MODE_640x480_60Hz, (char *) screen, SCREEN_ROWS, SCREEN_COLS, font8x8);

    // inicializamos UART
    uart_init(UART_ID, UART_BAUD);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_fifo_enabled(UART_ID, true);
    sleep_ms(100);
#if RCR_DEBUG == 1
    const char *dbg_message = "\r\nShow time ...\r\n";
    uart_write_blocking(UART_ID, (uint8_t *) dbg_message, strlen(dbg_message));
#endif

    // inicializamos TinyUSB en modo host
    tusb_init();

    // lanzamos el core 1 con mayor prioridad
    hw_set_bits(&bus_ctrl_hw->priority, BUSCTRL_BUS_PRIORITY_PROC1_BITS);
    multicore_launch_core1(core1_entry);

    // llenamos la pantalla con digitos
    terminal.cls();
    terminal.cursorTo(0, 0);
    for (uint r = 0; r < SCREEN_ROWS; r++) {
        for (uint c = 0; c < SCREEN_COLS; c++) {
            terminal.print('!' + c); //+ (c % 10);
        }
    }
    sleep_ms(2000);

    // mostramos el logo
    terminal.cls();
    terminal.cursorTo(0, 0);
    for (uint r = 0; r < LOGO_ROWS; r++) {
        terminal.cursorTo(r + (SCREEN_ROWS - LOGO_ROWS) / 2, (SCREEN_COLS - LOGO_COLS) / 2);
        for (uint c = 0; c < LOGO_COLS; c++) {
            terminal.print(logo[r][c]);
        }
    }
    sleep_ms(1000);

    // show time
    terminal.cls();
    terminal.cursorTo(0, 0);

    HIDKeyboard &kbd = HIDKeyboard::getInstance();
    KeyEvent keyEvent;
    while (1) {
        tuh_task();

        keyEvent = kbd.getKeyEvent();
        if (keyEvent.ch)
            uart_putc(UART_ID, keyEvent.ch);

        if (uart_is_readable(uart0)) {
            char ch = uart_getc(uart0);
            terminal.print(ch);
        }
    }
    __builtin_unreachable();

    return 0;
}
