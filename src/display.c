#include <string.h>

#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "pico/stdlib.h"

#include "common_dvi_pin_configs.h"
#include "dvi.h"
#include "tmds_encode.h"

#include "display.h"
#include "font_8x8.h"

#if defined(MODE_640x480_60Hz)
// DVDD 1.2V (1.1V seems ok too)
#define FRAME_WIDTH  640
#define FRAME_HEIGHT 480
#define VREG_VSEL    VREG_VOLTAGE_1_20
#define DVI_TIMING   dvi_timing_640x480p_60hz

#elif defined(MODE_720x480_60Hz)
// DVDD 1.2V
#define FRAME_WIDTH  720
#define FRAME_HEIGHT 480
#define VREG_VSEL    VREG_VOLTAGE_1_20
#define DVI_TIMING   dvi_timing_720x480p_60hz

#elif defined(MODE_800x600_60Hz)
// DVDD 1.3V, going downhill with a tailwind
#define FRAME_WIDTH  800
#define FRAME_HEIGHT 600
#define VREG_VSEL    VREG_VOLTAGE_1_30
#define DVI_TIMING   dvi_timing_800x600p_60hz

#elif defined(MODE_960x540p_60Hz)
// DVDD 1.25V (slower silicon may need the full 1.3, or just not work)
#define FRAME_WIDTH  960
#define FRAME_HEIGHT 540
#define VREG_VSEL    VREG_VOLTAGE_1_25
#define DVI_TIMING   dvi_timing_960x540p_60hz

#elif defined(MODE_1280x720_30Hz)
// 1280x720p 30 Hz (nonstandard)
// DVDD 1.25V (slower silicon may need the full 1.3, or just not work)
#define FRAME_WIDTH  1280
#define FRAME_HEIGHT 720
#define VREG_VSEL    VREG_VOLTAGE_1_25
#define DVI_TIMING   dvi_timing_1280x720p_30hz

#else
#error "Select a video mode!"
#endif

#if (DISPLAY_WIDTH % 8) != 0
#error "DISPLAY_WIDTH debe ser multiplo de 8"
#endif

#define DISPLAY_ORIG_X ((FRAME_WIDTH - DISPLAY_WIDTH) / 2)
#define DISPLAY_ORIG_Y ((FRAME_HEIGHT - DISPLAY_HEIGHT) / 2)

struct dvi_inst dvi0;
char display_buffer[CHAR_ROWS][CHAR_COLS];
uint16_t cur_col = 0, cur_row = 0;

void display_move_to(uint16_t row, uint16_t col) {
    cur_col = col >= CHAR_COLS ? CHAR_COLS - 1 : col;
    cur_row = row >= CHAR_ROWS ? CHAR_ROWS - 1 : row;
}

void display_putch(char ch) {
    display_buffer[cur_row][cur_col++] = ch;
    if (cur_col >= CHAR_COLS) {
        cur_col = 0;
        if (++cur_row >= CHAR_ROWS) {
            cur_row = CHAR_ROWS - 1;
        }
    }
}

void display_puts(char *text) {
    while (*text)
        display_putch(*text++);
}

static inline void prepare_scanline(uint y) {
    static uint8_t scanbuf[FRAME_WIDTH / 8];

    // preparamos la linea scan
    memset(scanbuf, 0x00, FRAME_WIDTH / 8);
    if (y >= DISPLAY_ORIG_Y && y < DISPLAY_ORIG_Y + DISPLAY_HEIGHT)
        for (uint col = 0; col < CHAR_COLS; col++) {
            uint8_t c = display_buffer[(y - DISPLAY_ORIG_Y) / 8][col];
            if (c >= 32 && c <= 127)
                scanbuf[col + DISPLAY_ORIG_X / 8] = ~~font_8x8[c][y % 8]; // ver este efecto para el cursor
        }

    // enviamos la linea scan
    uint32_t *tmdsbuf;
    queue_remove_blocking(&dvi0.q_tmds_free, &tmdsbuf);
    tmds_encode_1bpp((const uint32_t *) scanbuf, tmdsbuf, FRAME_WIDTH);
    queue_add_blocking(&dvi0.q_tmds_valid, &tmdsbuf);
}

static void core1_scanline_callback() {
    static uint y = 1;

    prepare_scanline(y);

    y = (y + 1) % FRAME_HEIGHT;
}

void display_init() {
    // ajusta voltaje del regulador interno del rp2040
    vreg_set_voltage(VREG_VSEL);
    sleep_ms(10);

    // configura la frecuencia del reloj del rp2040 segun lo requiere la senal dvi
    set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

    // configura la salida DVI
    dvi0.timing            = &DVI_TIMING;
    dvi0.ser_cfg           = DVI_DEFAULT_SERIAL_CONFIG;
    dvi0.scanline_callback = core1_scanline_callback;
    dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

    // limpiamos el buffer de pantalla
    memset(display_buffer, ' ', CHAR_ROWS * CHAR_COLS);

    // esto es necesario hacerlo (no se porque, pero no funciona sin esto)
    prepare_scanline(0);
}

void display_start() {
    // asigna manejador de interrupciones de DVI a este core
    dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);

    // comienza la salida DVI
    dvi_start(&dvi0);
}
