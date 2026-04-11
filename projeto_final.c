#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "inc/menu.h"
#include "inc/display.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "inc/config.h"
#include "inc/processamento_de_audio.h"
#include <math.h>

NotaMusical afinacao_padrao[] = {
    {"MI ", 82.3},  // 6ª
    {"LA ", 110.00}, // 5ª
    {"RE ", 146.83}, // 4ª
    {"SOL", 196.00}, // 3ª
    {"SI ", 246.94}, // 2ª
    {"mi ", 329.6}  // 1ª
};
#define QTD_NOTAS 6
volatile bool estado_btnA = false;
volatile bool estado_btnB = false;
volatile bool joystick_pressed = false;

typedef enum { MENU, IDLE, SAMPLING, PROCESSING, DISPLAY_RESULT } State;


void setup(){
    i2c_init(I2C_PORT, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    adc_init();
    adc_gpio_init(MIC_PIN);
    adc_select_input(ADC_CHAN);
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);
    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    for(int i=0; i < 3; i++){
        gpio_init(LEDS[i]);
        gpio_set_dir(LEDS[i], GPIO_OUT);
    }
}

void button_callback(uint gpio, uint32_t events) {
    static uint16_t last_pressed_time = 0;  // Agora persiste entre chamadas
    uint16_t timme_now = to_ms_since_boot(get_absolute_time());
    uint16_t debounce_time = 200;  // 200ms de debounce
    
    if (timme_now - last_pressed_time >= debounce_time) {  // Condição corrigida
        last_pressed_time = timme_now;
        if (gpio == BTN_A) {
            printf("Botão A pressionado\n");
            estado_btnA = true;
        } else if (gpio == BTN_B) {
            printf("Botão B pressionado\n");
            estado_btnB = true;
        }
    }
}

int main() {
    stdio_init_all();
    printf("Starting tuner...\n");
    State estado_atual = MENU;
    uint16_t buffer_audio[BUFFER_SIZE];
    float freq_detectada = 0;
    int selected_string = 0;
    int menu_top = 0;
    
    setup();
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &button_callback);
    gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_FALL, true);
    ssd1306_init();

    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    
    restart:
    
    while(true) {
        switch (estado_atual) {
            case MENU: {
                menu_render(ssd, &frame_area, afinacao_padrao, QTD_NOTAS, selected_string, &menu_top);

                // Handle joystick
                adc_select_input(ADC_CHANNEL_X); // X axis
                uint16_t x_val = adc_read();
                adc_select_input(ADC_CHANNEL_Y); // Y axis
                uint16_t y_val = adc_read();

                printf("Menu: selected=%d, x=%d, y=%d, button=%d\n", selected_string, x_val, y_val, gpio_get(BTN_A));

                menu_update_selection(&selected_string, QTD_NOTAS, y_val, &joystick_pressed);

               
                if (estado_btnA) {
                    printf("Botao pressionado, string selecionada: %d\n", selected_string);
                    estado_atual = IDLE;
                    estado_btnA = false;
                }
                sleep_ms(10);
            }
                break;

            case IDLE:
                if (estado_btnB) {
                    estado_atual = MENU;
                    estado_btnB = false;
                } else if (abs((int)adc_read() - 2048) > 100) {
                    estado_atual = SAMPLING;
                }
                sleep_ms(10);
                break;

            case SAMPLING:
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    buffer_audio[i] = adc_read();
                    sleep_us(125);
                }
                estado_atual = PROCESSING;
                break;

            case PROCESSING:
                freq_detectada = calcular_frequencia(buffer_audio, BUFFER_SIZE);
                if (freq_detectada > 500 || freq_detectada < 50) freq_detectada = 0;
                estado_atual = DISPLAY_RESULT;
                break;

            case DISPLAY_RESULT:
               memset(ssd, 0, ssd1306_buffer_length);
               
               
                
                int melhor_nota = selected_string; // Use selected string
                
                char msg[20];
                sprintf(msg, "Nota: %s", afinacao_padrao[melhor_nota].nome);
                ssd1306_draw_string(ssd, 5, 2, msg);
                
                sprintf(msg, "Freq: %.1fHz", freq_detectada);
                ssd1306_draw_string(ssd, 5, 15, msg);

                float erro_hz = freq_detectada - afinacao_padrao[melhor_nota].freq_alvo;

                
                if (freq_detectada < 50) { 
                    ssd1306_draw_string(ssd, 20, 40, "SEM SINAL");
                } 
                else if (fabs(erro_hz) < 1.2) { // Zona de tolerância: AFINADO
                    ssd1306_draw_string(ssd, 35, 40, "OK!");
                    gpio_put(LEDS[0], 1); // Liga LED Verde (Pino 11) [cite: 36]
                } 
                else if (erro_hz < 0) { // Frequência abaixo do alvo
                    ssd1306_draw_string(ssd, 10, 40, "APERTAR CORDA ^");
                    gpio_put(LEDS[1], 1); // Liga LED Azul (Pino 12) [cite: 36]
                } 
                else { // Frequência acima do alvo
                    ssd1306_draw_string(ssd, 10, 40, "AFROUXAR CORDA v");
                    gpio_put(LEDS[2], 1); // Liga LED Vermelho (Pino 13) [cite: 36]
                }

                // 5. Interface Gráfica: Régua Dinâmica (Ponteiro) [cite: 37]
                // Mapeia o erro para a posição X (Centro em 64 pixels)
                // int x_ponteiro = 64 + (int)(erro_hz * 6); 
                // if (x_ponteiro < 5) x_ponteiro = 5;
                // if (x_ponteiro > 123) x_ponteiro = 123;

                // Desenha o alvo central (fixo) e o ponteiro atual (móvel)
                // ssd1306_draw_rect(ssd, 52, 63, 2, 10, true, false); 
                // ssd1306_draw_rect(ssd, 54, x_ponteiro, 2, 6, true, true); 

                // 6. Atualização do Hardware e Finalização do Ciclo
                render_on_display(ssd, &frame_area);
                
                sleep_ms(800); // Tempo para o usuário ler a informação
                
                // Desliga todos os LEDs para a próxima leitura
                for(int i=0; i<3; i++) gpio_put(LEDS[i], 0);
                
                estado_atual = IDLE; // Retorna à espera por som
                break;
        }
    }
    return 0;
}