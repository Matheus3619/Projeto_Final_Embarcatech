#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "ssd1306_i2c.h"

void scroll_text(uint8_t *ssd, struct render_area *area, const char *text, int16_t y, int delay_ms);

#endif
