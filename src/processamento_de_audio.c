#include "processamento_de_audio.h"
#include "config.h"
#include <math.h>

float calcular_frequencia(uint16_t *amostras, int n) {
    long maior_correlacao = 0;
    int melhor_atraso = 0;

    for (int atraso = 20; atraso < n / 2; atraso++) {
        long correlacao = 0;
        for (int i = 0; i < n / 2; i++) {
            correlacao += ((int16_t)amostras[i] - 2048) * ((int16_t)amostras[i + atraso] - 2048);
        }
        if (correlacao > maior_correlacao) {
            maior_correlacao = correlacao;
            melhor_atraso = atraso;
        }
    }
    return (melhor_atraso > 0) ? (float)SAMPLING_RATE / melhor_atraso : 0;
}