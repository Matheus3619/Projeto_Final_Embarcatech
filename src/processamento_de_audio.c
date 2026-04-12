#include "processamento_de_audio.h"
#include "config.h"
#include "inc/processamento_de_audio.h"
#include <math.h>

#define PI 3.14159265358979323846

float calcular_frequencia(uint16_t *amostras, int n) {
    float sinal[n];
    float media = 0.0;

    // 1. Remove o nível DC (Offset)
    for (int i = 0; i < n; i++) {
        media += amostras[i];
    }
    media /= n;

    // 2. Aplica o Janelamento de Hann e normaliza o sinal
    // Fórmula do Janelamento de Hann: w(i) = 0.5 * (1 - cos(2 * PI * i / (N - 1)))
    for (int i = 0; i < n; i++) {
        float hann_window = 0.5 * (1.0 - cosf((2.0 * PI * i) / (n - 1)));
        sinal[i] = ((float)amostras[i] - media) * hann_window;
    }

    // 3. Autocorrelação para encontrar o período
    float max_correlacao = 0.0;
    int melhor_atraso = -1;
    
    // Limites para guitarra (ex: de ~50Hz a ~400Hz em sampling rate de 8000Hz)
    int min_atraso = SAMPLING_RATE / 400; // ~20 amostras
    int max_atraso = SAMPLING_RATE / 50;  // ~160 amostras

    // Array para guardar as correlações e usar na interpolação depois
    float correlacao[max_atraso + 1]; 

    for (int atraso = min_atraso; atraso <= max_atraso; atraso++) {
        float soma = 0.0;
        for (int i = 0; i < n - atraso; i++) {
            soma += sinal[i] * sinal[i + atraso];
        }
        correlacao[atraso] = soma;

        if (soma > max_correlacao) {
            max_correlacao = soma;
            melhor_atraso = atraso;
        }
    }

    
    if (melhor_atraso == -1 || max_correlacao < 10000.0) { // Threshold para evitar ruído
        return 0.0;
    }

    // 4. INTERPOLAÇÃO PARABÓLICA
    
    float atraso_exato = (float)melhor_atraso;

    // Só faz a interpolação se tiver vizinhos para comparar
    if (melhor_atraso > min_atraso && melhor_atraso < max_atraso) {
        float y1 = correlacao[melhor_atraso - 1]; // Ponto anterior
        float y2 = correlacao[melhor_atraso];     // Ponto máximo (inteiro)
        float y3 = correlacao[melhor_atraso + 1]; // Ponto posterior

        // Fórmula da vértice da parábola
        float denominador = 2.0 * (y1 - 2.0 * y2 + y3);
        if (denominador != 0.0) {
            atraso_exato += (y1 - y3) / denominador;
        }
    }

    // 5. Calcula e retorna a frequência precisa
    return (float)SAMPLING_RATE / atraso_exato;
}
