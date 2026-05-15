/*  ILI9488 driver over FMC 8-bit indirect-interface 8080 bus.
 *
 *  Wiring (Riverdi RVA35HI-NUC144A on NUCLEO-U5A5ZJ-Q):
 *      LCD_RESET = PE11   GPIO output (active low pulse)
 *      LCD_BL    = PB8    GPIO output (active high)
 *      LCD_RS    = PE4    FMC_A20 (hardware-driven; 0=command, 1=data)
 *      LCD_CS    = PD7    FMC_NE1
 *      LCD_WR    = PD5    FMC_NWE
 *      LCD_RD    = PD4    FMC_NOE
 *      LCD_DB0..7= PD14/PD15/PD0/PD1/PE7/PE8/PE9/PE10 (FMC_D0..7)
 *
 *  Pixel format: 16 bpp RGB565.  Each pixel is sent as 2 byte writes,
 *  high byte first (ILI9488's "RGB565 over 8-bit bus" packing).
 *
 *  After `ili9488_init` returns the display is in transfer state with the
 *  full 320x480 area as the write window; subsequent `ili9488_push_pixels`
 *  calls stream RGB565 pixels straight to the panel.
 */

#include "ili9488.h"

/* ---- Low-level command + data primitives -------------------------- */
static inline void cmd(uint8_t c)          { ILI9488_CMD_REG = c; }
static inline void dat(uint8_t d)          { ILI9488_DAT_REG = d; }
static inline void delay_ms(uint32_t ms)   { HAL_Delay(ms); }

/* ---- Hardware reset ----------------------------------------------- */
void ili9488_reset(void)
{
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
    delay_ms(20);
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
    delay_ms(20);
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
    delay_ms(120);            /* ILI9488 datasheet: 120 ms boot time */
}

/* ---- Backlight ----------------------------------------------------- */
void ili9488_backlight(uint8_t on)
{
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin,
                      on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* ---- Panel sleep --------------------------------------------------- */
/* S16.5: SLPIN/SLPOUT pair. SLPIN drops panel quiescent current; the LED
 * backlight is controlled separately via ili9488_backlight(). SLPOUT
 * requires a >=120 ms wait per the ILI9488 datasheet before any other
 * command — call this from a thread context, not from an ISR. */
void ili9488_sleep(uint8_t sleep_in)
{
    if (sleep_in) {
        cmd(0x10);              /* SLPIN — enter sleep mode */
        delay_ms(5);
    } else {
        cmd(0x11);              /* SLPOUT — leave sleep mode */
        delay_ms(120);
    }
}

/* ---- Window selection --------------------------------------------- */
void ili9488_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    cmd(0x2A);                /* Column Address Set */
    dat(x0 >> 8); dat(x0 & 0xFF);
    dat(x1 >> 8); dat(x1 & 0xFF);

    cmd(0x2B);                /* Page Address Set */
    dat(y0 >> 8); dat(y0 & 0xFF);
    dat(y1 >> 8); dat(y1 & 0xFF);

    cmd(0x2C);                /* Memory Write -- subsequent data bytes are pixels */
}

/* ---- Bulk pixel push (MSB first, standard ILI9488 order) ----------- */
void ili9488_push_pixels(const uint16_t *buf, uint32_t n)
{
    while (n--) {
        uint16_t p = *buf++;
        dat((uint8_t)(p >> 8));
        dat((uint8_t)(p & 0xFF));
    }
}

/* ---- Solid-color full-screen fill --------------------------------- */
void ili9488_fill(uint16_t rgb565)
{
    ili9488_set_window(0, 0, ILI9488_WIDTH - 1, ILI9488_HEIGHT - 1);
    const uint8_t hi = (uint8_t)(rgb565 >> 8);
    const uint8_t lo = (uint8_t)(rgb565 & 0xFF);
    for (uint32_t i = 0; i < (uint32_t)ILI9488_WIDTH * ILI9488_HEIGHT; i++) {
        dat(hi);
        dat(lo);
    }
}

/* ---- 4-stripe diagnostic: RED | GREEN | BLUE | WHITE ----------------
 * Each stripe is 80 columns wide (= 320 / 4).  Lets us see at a glance
 * how RGB565 primaries actually map onto the panel for this carrier. */
void ili9488_stripes_rgbw(void)
{
    static const uint16_t colors[4] = {
        0xF800,   /* RED   = R[4:0]=31, G=0, B=0 */
        0x07E0,   /* GREEN = R=0, G[5:0]=63, B=0 */
        0x001F,   /* BLUE  = R=0, G=0, B[4:0]=31 */
        0xFFFF,   /* WHITE = all max */
    };
    const uint32_t sw = ILI9488_WIDTH / 4;     /* 480/4 = 120 px per stripe */
    for (uint32_t s = 0; s < 4; s++) {
        ili9488_set_window(s * sw, 0, (s + 1) * sw - 1, ILI9488_HEIGHT - 1);
        const uint8_t hi = (uint8_t)(colors[s] >> 8);
        const uint8_t lo = (uint8_t)(colors[s] & 0xFF);
        for (uint32_t i = 0; i < sw * ILI9488_HEIGHT; i++) {
            dat(hi);
            dat(lo);
        }
    }
}

/* ---- Full init sequence ------------------------------------------- */
void ili9488_init(void)
{
    ili9488_reset();

    /* Software reset for good measure */
    cmd(0x01);
    delay_ms(120);

    /* Positive gamma control */
    cmd(0xE0);
    dat(0x00); dat(0x03); dat(0x09); dat(0x08); dat(0x16);
    dat(0x0A); dat(0x3F); dat(0x78); dat(0x4C); dat(0x09);
    dat(0x0A); dat(0x08); dat(0x16); dat(0x1A); dat(0x0F);

    /* Negative gamma control */
    cmd(0xE1);
    dat(0x00); dat(0x16); dat(0x19); dat(0x03); dat(0x0F);
    dat(0x05); dat(0x32); dat(0x45); dat(0x46); dat(0x04);
    dat(0x0E); dat(0x0D); dat(0x35); dat(0x37); dat(0x0F);

    /* Power control 1 */
    cmd(0xC0); dat(0x17); dat(0x15);

    /* Power control 2 */
    cmd(0xC1); dat(0x41);

    /* VCOM control */
    cmd(0xC5); dat(0x00); dat(0x12); dat(0x80);

    /* Memory Access Control (MADCTL):
     *   bit 7 (MY)  = 1  controls visible-X direction (after MV swap)
     *   bit 6 (MX)  = 1  controls visible-Y direction (after MV swap)
     *   bit 5 (MV)  = 1  row/column exchange (landscape 480x320)
     *   bit 3 (BGR) = 1  BGR pixel order (carrier R<->B swap correction)
     *   net 0xE8 -- landscape, origin top-left as TouchGFX expects. */
    cmd(0x36); dat(0xE8);

    /* Interface Pixel Format -- 16 bpp RGB565 over 8-bit DBI Type B bus */
    cmd(0x3A); dat(0x55);

    /* Interface Mode Control */
    cmd(0xB0); dat(0x00);

    /* Frame Rate Control: 70 Hz default */
    cmd(0xB1); dat(0xA0);

    /* Display Inversion Control */
    cmd(0xB4); dat(0x02);

    /* Display Function Control */
    cmd(0xB6); dat(0x02); dat(0x02); dat(0x3B);

    /* Entry Mode Set */
    cmd(0xB7); dat(0xC6);

    /* Adjust control 3 */
    cmd(0xF7); dat(0xA9); dat(0x51); dat(0x2C); dat(0x82);

    /* Sleep Out */
    cmd(0x11);
    delay_ms(120);

    /* Display Inversion ON -- this is a "Normally Black IPS" panel
     * (Riverdi RVA35HI datasheet), so colours arrive inverted unless we
     * tell the controller to invert them back. */
    cmd(0x21);

    /* Display On */
    cmd(0x29);
    delay_ms(25);
}
