#include <string.h>

#include "hardware/structs/bus_ctrl.h"
#include "pico.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

#include "display.h"
#include "logo.h"

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

    // lanza el core 1 con mayor prioridad
    hw_set_bits(&bus_ctrl_hw->priority, BUSCTRL_BUS_PRIORITY_PROC1_BITS);
    multicore_launch_core1(core1_main);

    // llenamos la pantalla con digitos
    for (uint r = 0; r < CHAR_ROWS; r++) {
        for (uint c = 0; c < CHAR_COLS; c++) {
            display_buffer[r][c] = '!' + c; //+ (c % 10);
        }
    }
    sleep_ms(3000);
    memset(display_buffer, ' ', CHAR_ROWS * CHAR_COLS);

    // mostramos el logo*-
    for (uint r = 0; r < LOGO_ROWS; r++) {
        for (uint c = 0; c < LOGO_COLS; c++) {
            display_buffer[r][(CHAR_COLS - LOGO_COLS) / 2 + c] = logo[r][c];
        }
    }

    // show time para el core 0
    while (1) {
        __wfi();
    }
    __builtin_unreachable();

    return 1;
}
