#ifndef _RCR_DISPLAY_H_
#define _RCR_DISPLAY_H_

#include <stdint.h>

// el modo de pantalla
#define MODE_640x480_60Hz
// #define MODE_720x480_60Hz
// #define MODE_800x600_60Hz
// #define MODE_960x540p_60Hz
// #define MODE_1280x720_30Hz

// la dimension de la zona desplegable (se centrara dentro de MODE_)
#define CHAR_ROWS      50
#define CHAR_COLS      70
#define DISPLAY_WIDTH  (CHAR_COLS * 8)
#define DISPLAY_HEIGHT (CHAR_ROWS * 8)

extern void display_init();
extern void display_start();
extern uint8_t display_buffer[CHAR_ROWS][CHAR_COLS];

#endif // _RCR_DISPLAY_H_
