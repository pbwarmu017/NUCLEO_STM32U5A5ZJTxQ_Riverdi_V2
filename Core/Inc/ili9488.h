#ifndef ILI9488_H
#define ILI9488_H

#include "main.h"

/* Landscape orientation (TouchGFX configured for 480x320). */
#define ILI9488_WIDTH    480U
#define ILI9488_HEIGHT   320U

/* FMC-mapped command/data addresses.
 * Bank 1 NE1 base = 0x60000000.
 * FMC_A20 is bound to the ILI9488's RS (data/command) line: A20=0 -> command,
 * A20=1 -> data.  Writing to FMC base + 0x100000 sets A20=1 in the address bus. */
#define ILI9488_CMD_REG  (*(volatile uint8_t *)0x60000000)
#define ILI9488_DAT_REG  (*(volatile uint8_t *)0x60100000)

/* RGB565 helpers for quick fill / test patterns. */
#define ILI9488_BLACK    0x0000U
#define ILI9488_WHITE    0xFFFFU
#define ILI9488_RED      0xF800U
#define ILI9488_GREEN    0x07E0U
#define ILI9488_BLUE     0x001FU
#define ILI9488_CYAN     0x07FFU
#define ILI9488_MAGENTA  0xF81FU
#define ILI9488_YELLOW   0xFFE0U

void ili9488_reset(void);                                  /* drive LCD_RESET pulse */
void ili9488_init(void);                                   /* full init handshake   */
void ili9488_backlight(uint8_t on);                        /* GPIO BL on/off        */
void ili9488_sleep(uint8_t sleep_in);                      /* 1 = SLPIN, 0 = SLPOUT */
void ili9488_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ili9488_fill(uint16_t rgb565);                        /* full-screen solid     */
void ili9488_push_pixels(const uint16_t *buf, uint32_t n);
void ili9488_stripes_rgbw(void);   /* 4 vertical stripes R|G|B|W for color-order diagnosis */

#endif
