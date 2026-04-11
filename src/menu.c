#include "menu.h"
#include "ssd1306.h"
#include <stdio.h>
#include <string.h>

void menu_render(uint8_t *ssd, struct render_area *area, const NotaMusical notes[], int qtd, int selected, int *menu_top) {
    const int visible_lines = 4;

    if (selected < *menu_top) {
        *menu_top = selected;
    } else if (selected >= *menu_top + visible_lines) {
        *menu_top = selected - visible_lines + 1;
    }

    memset(ssd, 0, ssd1306_buffer_length);
    ssd1306_draw_string(ssd, 0, 0, "Escolha a corda:");

    for (int row = 0; row < visible_lines; row++) {
        int item_index = *menu_top + row;
        if (item_index >= qtd) break;

        char line[12];
        sprintf(line, "%s %s", (item_index == selected) ? "->" : "  ", notes[item_index].nome);
        ssd1306_draw_string(ssd, 5, 12 + row * 12, line);
    }

    if (*menu_top > 0) {
        ssd1306_draw_string(ssd, 110, 12, "^");
    }
    if (*menu_top + visible_lines < qtd) {
        ssd1306_draw_string(ssd, 110, 12 + (visible_lines - 1) * 12, "v");
    }

    render_on_display(ssd, area);
}

void menu_update_selection(int *selected, int qtd, uint16_t y_val, volatile bool *button_pressed) {
    if (y_val < 1000 && !*button_pressed) {
        *selected = (*selected - 1 + qtd) % qtd;
        *button_pressed = true;
    } else if (y_val > 3000 && !*button_pressed) {
        *selected = (*selected + 1) % qtd;
        *button_pressed = true;
    } else if (y_val >= 1000 && y_val <= 3000) {
        *button_pressed = false;
    }
}
