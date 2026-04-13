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
#include "hardware/uart.h"
#include "inc/config.h"
#include "inc/processamento_de_audio.h"
#include "src/saida_de_audio.h"
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

// Definição dos estados do sistema
typedef enum { MENU, ESPERA, AMOSTRAGEM, PROCESSAMENTO, DISPLAY_OLED } State;


// configurações iniciais: I2C, ADC, GPIOs
void setup(){
   

    i2c_init(I2C_PORT, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    adc_init();
    adc_gpio_init(MIC_PIN);
    adc_select_input(ADC_CHAN);
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);
    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    for(int i=0; i < 3; i++){
        gpio_init(LEDS[i]);
        gpio_set_dir(LEDS[i], GPIO_OUT);
    }
}

void button_callback(uint gpio, uint32_t events) {
    static uint32_t last_pressed_time_A = 0;
    static uint32_t last_pressed_time_B = 0;
    uint32_t timme_now = to_ms_since_boot(get_absolute_time()); // Obtém o tempo atual em milissegundos desde a inicialização do sistema
    uint32_t debounce_time = 200;  // 200ms de debounce
    uint32_t *last_pressed_time = (gpio == BTN_A) ? &last_pressed_time_A : &last_pressed_time_B;

    // Verifica se o tempo desde o último pressionamento é maior ou igual ao tempo de debounce
    if (timme_now - *last_pressed_time >= debounce_time) {
        *last_pressed_time = timme_now;
        if (gpio == BTN_A) {
            estado_btnA = true;
        } else if (gpio == BTN_B) {
            estado_btnB = true;
        }
    }
}

int main() {
    stdio_init_all();
    printf("Starting tuner...\n");
    State estado_atual = MENU;
    uint16_t buffer_audio[BUFFER_SIZE]; // Buffer para amostras de áudio
    float freq_detectada = 0; // Frequência detectada após processamento
    int selected_string = 0; // Índice da corda selecionada no menu
    int menu_top = 0; // Índice do item superior visível no menu
    
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
    
    while(true){

        switch (estado_atual) {
            case MENU: {
                menu_render(ssd, &frame_area, afinacao_padrao, QTD_NOTAS, selected_string, &menu_top);

                // joystick
                adc_select_input(ADC_CHANNEL_Y); // Y axis
                uint16_t y_val = adc_read();

                printf("Menu: selected=%d, y=%d, button=%d\n", selected_string, y_val, !gpio_get(BTN_A));

                // Atualiza a seleção do menu com base no joystick e no estado do botão
                menu_update_selection(&selected_string, QTD_NOTAS, y_val, &joystick_pressed);

               
                if (estado_btnA) {
                    printf("Botao A pressionado, string selecionada: %d\n", selected_string);
                    estado_atual = ESPERA;
                    estado_btnA = false;

                    // Limpa a tela do Menu e avisa o usuário que o microfone está ativo
                    memset(ssd, 0, ssd1306_buffer_length);
                    ssd1306_draw_string(ssd, 15, 30, "Ouvindo...");
                    render_on_display(ssd, &frame_area);
                }
                sleep_ms(10);
            }
                break;

            case ESPERA:
                adc_select_input(ADC_CHAN);
                if (estado_btnB) {
                    printf("Botao B pressionado, voltando ao menu\n");
                    estado_atual = MENU;
                    estado_btnB = false;
                } else if (abs((int)adc_read() - 2048) > 100) {
                    // Filtro de ruídos curtos: verifica se o som se sustenta
                    int picos_detectados = 0;
                    // Lê 100 amostras (~12.5ms a 8kHz, garante pelo menos 1 ciclo da corda Mi mais grave)
                    for (int i = 0; i < 100; i++) { 
                        if (abs((int)adc_read() - 2048) > 100) {
                            picos_detectados++;
                        }
                        sleep_us(125); // Mesma taxa do seu ADC (8000 amostras por segundo)
                    }
                    
                    // Um estalo rápido terá poucos picos. Uma corda vibrando terá o sinal alto por mais tempo.
                    if (picos_detectados > 20) {
                        estado_atual = AMOSTRAGEM;
                    }
                }
                sleep_ms(10);
                break;

            case AMOSTRAGEM:
                adc_select_input(ADC_CHAN);
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    buffer_audio[i] = adc_read();
                    sleep_us(125);
                }
                estado_atual = PROCESSAMENTO;
                break;

            case PROCESSAMENTO:
                freq_detectada = calcular_frequencia(buffer_audio, BUFFER_SIZE);
                
                if (freq_detectada > 500 || freq_detectada < 50) {
                    freq_detectada = 0;
                } else {
                    
                    float menor_erro = 1000.0;
                    int corda_detectada = selected_string;
                    
                    
                    for (int i = 0; i < QTD_NOTAS; i++) {
                        float erro_atual = fabs(freq_detectada - afinacao_padrao[i].freq_alvo);
                        if (erro_atual < menor_erro) {
                            menor_erro = erro_atual;
                            corda_detectada = i;
                        }
                    }
                    
                    // Se o som estiver a menos de 25Hz de alguma corda, assume que é ela
                    if (menor_erro < 25.0) {
                        selected_string = corda_detectada;
                    }
                }
                estado_atual = DISPLAY_OLED;
                break;

            case DISPLAY_OLED:
                memset(ssd, 0, ssd1306_buffer_length);
                int melhor_nota = selected_string; 
                
                char msg[20];
                sprintf(msg, "Nota: %s", afinacao_padrao[melhor_nota].nome);
                ssd1306_draw_string(ssd, 5, 2, msg);
                
                sprintf(msg, "Freq: %.1fHz", freq_detectada);
                ssd1306_draw_string(ssd, 5, 15, msg);

                float erro_hz = freq_detectada - afinacao_padrao[melhor_nota].freq_alvo;

                int led_ativo = -1;

                if (freq_detectada < 70) {  // almentei o limite inferior para 70Hz para evitar que ruídos externos sejam interpretados
                    ssd1306_draw_string(ssd, 20, 40, "SEM SINAL");
                } 
                else if (fabs(erro_hz) < 1.2) { // Zona de tolerância: AFINADO
                    tocar_buzzer((uint)afinacao_padrao[selected_string].freq_alvo, 500); // com algoritimo de strobe, o buzzer só toca quando a corda estiver afinada, para não incomodar durante o processo de afinação
                    ssd1306_draw_string(ssd, 35, 40, "OK!");
                    led_ativo = LEDS[0]; 
                    gpio_put(led_ativo, 1);
                } 
                else if (erro_hz < 0) { // Frequência abaixo do alvo
                    ssd1306_draw_string(ssd, 10, 40, "APERTAR CORDA ^");
                    led_ativo = LEDS[1]; 
                } 
                else { // Frequência acima do alvo
                    ssd1306_draw_string(ssd, 10, 40, "AFROUXAR CORDA v");
                    led_ativo = LEDS[2]; 
                }
                render_on_display(ssd, &frame_area);
                
                // Efeito Estroboscópico (Blink Proporcional ao Erro)
                if (led_ativo == LEDS[1] || led_ativo == LEDS[2]) {
                    // Calcula a "frequência de batimento" baseada na distância para a nota alvo
                    float freq_piscar = fabs(erro_hz);
                    
                    // Limita a piscada entre 1Hz (quase afinado) e 20Hz (muito desafinado)
                    if (freq_piscar < 1.0f) freq_piscar = 1.0f;
                    if (freq_piscar > 20.0f) freq_piscar = 20.0f;
                    
                    int periodo_ms = (int)(1000.0f / freq_piscar);
                    int meia_onda = periodo_ms / 2;
                    int tempo_gasto = 0;
                    
                    // Executa o strobe dinâmico durante os 800ms de AMOSTRAGEM originais
                    while (tempo_gasto < 800) {
                        gpio_put(led_ativo, 1);
                        sleep_ms(meia_onda);
                        gpio_put(led_ativo, 0);
                        sleep_ms(meia_onda);
                        tempo_gasto += periodo_ms;
                    }
                } else {
                    // Se estiver afinado (Verde) ou sem sinal, apenas mantém o delay padrão
                    sleep_ms(800);
                }
                
                // Desliga todos os LEDs para a próxima leitura
                for(int i=0; i<3; i++) gpio_put(LEDS[i], 0);
                
                estado_atual = ESPERA; // Retorna à espera por som
                break;
        }
    }
}