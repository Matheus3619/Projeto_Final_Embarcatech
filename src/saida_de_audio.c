#include "hardware/pwm.h"
#include "config.h"
#include <stdint.h>
#include "pico/types.h"
#include "pico/time.h"

void tocar_buzzer(uint frequencia, uint duracao_ms) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint32_t clock = 125000000; // Clock padrão do RP2040

    // Primeiro, toca um harmônico breve (2x frequência) para simular o ataque de uma corda
    uint32_t wrap_harm = clock / (frequencia * 2) - 1;
    pwm_set_wrap(slice_num, wrap_harm);
    pwm_set_gpio_level(BUZZER_PIN, wrap_harm / 2);
    pwm_set_enabled(slice_num, true);
    sleep_ms(100); // Duração breve do harmônico

    // Agora, toca a frequência fundamental com decaimento
    uint32_t wrap = clock / frequencia - 1;
    pwm_set_wrap(slice_num, wrap);

    // Simula som de corda com decaimento exponencial
    int steps = 20; // Número de passos para o decaimento
    int delay_per_step = (duracao_ms - 100) / steps; // Ajusta para o tempo restante
    for (int i = 0; i < steps; i++) {
        float decay_factor = 1.0f - (float)i / steps; // De 1.0 a 0.0
        uint32_t level = (uint32_t)(wrap / 2 * decay_factor * decay_factor); // Decaimento quadrático para som mais natural
        pwm_set_gpio_level(BUZZER_PIN, level);
        sleep_ms(delay_per_step);
    }

    pwm_set_enabled(slice_num, false); // Desliga o som
}