#include <string.h>
extern "C" {
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "common_dvi_pin_configs.h"
#include "dvi.h"
#include "tmds_encode.h"
}
#include "DVIDisplay.hpp"

static struct {
    uint16_t frameHeight;                // alto zona visible (pixeles)
    uint16_t frameWidth;                 // ancho zona visible (pixeles)
    uint16_t screenOrigY;                // linea scan de inicio del despliegue (pixeles)
    uint16_t screenOrigX;                // columna de la linea scan de inicio del despliegue (pixeles)
    uint16_t screenHeight;               // alto del despliegue (pixeles)
    uint16_t screenWidth;                // ancho del despliegue (pixeles)
    uint16_t rows;                       // numero de filas
    uint16_t cols;                       // numero de caracteres por fila
    char *screen;                        // buffer de pantalla [rows][cols]
    uint8_t (*font8x8)[8];               // arreglo con el set de caracteres
    uint8_t scanline[160];               // suficiente para 1280 pixeles
    vreg_voltage vreg_vsel;              //
    const struct dvi_timing *dvi_timing; //
    struct dvi_inst dvi0;                //
} video_data;

extern "C" {
static void prepare_scanline(uint y) {
    // preparamos la linea scan
    memset(video_data.scanline, 0x00, sizeof(video_data.scanline));
    if (y >= video_data.screenOrigY && y < video_data.screenOrigY + video_data.screenHeight)
        for (uint col = 0; col < video_data.cols; col++) {
            uint8_t c = video_data.screen[((y - video_data.screenOrigY) / 8) * video_data.cols + col];
            if (c >= 32 && c <= 127)
                video_data.scanline[col + video_data.screenOrigX / 8] =
                    ~~video_data.font8x8[c][y % 8]; // ver este efecto para el cursor
        }

    // enviamos la linea scan
    uint32_t *tmdsbuf;
    queue_remove_blocking(&video_data.dvi0.q_tmds_free, &tmdsbuf);
    tmds_encode_1bpp((const uint32_t *) video_data.scanline, tmdsbuf, video_data.frameWidth);
    queue_add_blocking(&video_data.dvi0.q_tmds_valid, &tmdsbuf);
}

static void scanline_callback() {
    static uint y = 1;

    prepare_scanline(y);

    y = (y + 1) % video_data.frameHeight;
}
}

DVIDisplay &DVIDisplay::getInstance() {
    static DVIDisplay instance;
    return instance;
}
void DVIDisplay::init(VideoMode mode, char *screen, uint16_t rows, uint16_t cols, uint8_t (*font8x8)[8]) {
    video_data.rows    = rows;
    video_data.cols    = cols;
    video_data.screen  = screen;
    video_data.font8x8 = font8x8;
    completeVars(mode);

    // ajusta voltaje del regulador interno del rp2040
    vreg_set_voltage(video_data.vreg_vsel);
    sleep_ms(10);

    // configura la frecuencia del reloj del rp2040 segun lo requiere la senal dvi
    set_sys_clock_khz(video_data.dvi_timing->bit_clk_khz, true);

    // configura la salida DVI
    video_data.dvi0.timing            = video_data.dvi_timing;
    video_data.dvi0.ser_cfg           = DVI_DEFAULT_SERIAL_CONFIG;
    video_data.dvi0.scanline_callback = scanline_callback;
    dvi_init(&video_data.dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

    // esto es necesario hacerlo (no se porque, pero no funciona sin esto)
    prepare_scanline(0);
}

void DVIDisplay::start() {
    // asigna manejador de interrupciones de DVI a este core
    dvi_register_irqs_this_core(&video_data.dvi0, DMA_IRQ_0);

    // comienza la salida DVI
    dvi_start(&video_data.dvi0);
}

void DVIDisplay::completeVars(VideoMode mode) {
    switch (mode) {
    case MODE_640x480_60Hz:
        video_data.frameHeight = 480;
        video_data.frameWidth  = 640;
        video_data.vreg_vsel   = VREG_VOLTAGE_1_20;
        video_data.dvi_timing  = &dvi_timing_640x480p_60hz;
        break;
    case MODE_720x480_60Hz:
        video_data.frameHeight = 480;
        video_data.frameWidth  = 720;
        video_data.vreg_vsel   = VREG_VOLTAGE_1_20;
        video_data.dvi_timing  = &dvi_timing_720x480p_60hz;
        break;
    case MODE_800x600_60Hz:
        video_data.frameHeight = 600;
        video_data.frameWidth  = 800;
        video_data.vreg_vsel   = VREG_VOLTAGE_1_30;
        video_data.dvi_timing  = &dvi_timing_800x600p_60hz;
        break;
    case MODE_960x540p_60Hz:
        video_data.frameHeight = 540;
        video_data.frameWidth  = 960;
        video_data.vreg_vsel   = VREG_VOLTAGE_1_25;
        video_data.dvi_timing  = &dvi_timing_960x540p_60hz;
        break;
    case MODE_1280x720_30Hz:
        video_data.frameHeight = 720;
        video_data.frameWidth  = 1280;
        video_data.vreg_vsel   = VREG_VOLTAGE_1_25;
        video_data.dvi_timing  = &dvi_timing_1280x720p_30hz;
        break;
    }

    video_data.screenHeight = video_data.rows * 8;
    video_data.screenWidth  = video_data.cols * 8;
    video_data.screenOrigY  = (video_data.frameHeight - video_data.screenHeight) / 2;
    video_data.screenOrigX  = (video_data.frameWidth - video_data.screenWidth) / 2;
}