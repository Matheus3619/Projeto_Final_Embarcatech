#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Pinos I2C e ADC
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define MIC_PIN 28
#define ADC_CHAN 2
#define ADC_CHANNEL_X 0
#define ADC_CHANNEL_Y 1

// LEDs (G, B, R)
static const unsigned int LEDS[] = {11, 12, 13};

// Joystick
#define JOYSTICK_X 26
#define JOYSTICK_Y 27

// BOTÕES
#define BTN_A 5
#define BTN_B 6

// Parâmetros de Áudio
#define SAMPLING_RATE 8000
#define BUFFER_SIZE 1024

typedef struct {
    char nome[4];
    float freq_alvo;
} NotaMusical;

#endif