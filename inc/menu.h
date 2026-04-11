#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <stdint.h>
#include "ssd1306_i2c.h"
#include "config.h"

void menu_render(uint8_t *ssd, struct render_area *area, const NotaMusical notes[], int qtd, int selected, int *menu_top);
void menu_update_selection(int *selected, int qtd, uint16_t y_val, volatile bool *button_pressed);

#endif
