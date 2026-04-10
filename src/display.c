#include "display.h"
#include "ssd1306.h"
#include <string.h>
#include "pico/stdlib.h"

void scroll_text(uint8_t *ssd, struct render_area *area, const char *text, int16_t y, int delay_ms) {
    int16_t text_width = strlen(text) * 8;
    for (int16_t x = ssd1306_width; x >= -text_width; x--) {
        memset(ssd, 0, ssd1306_buffer_length);
        ssd1306_draw_string(ssd, x, y, text);
        render_on_display(ssd, area);
        sleep_ms(delay_ms);
    }
}
