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

#define BUZZER_PIN 10
#define NOTE_A4 440.0f
#define NOTE_B4 493.88f
#define NOTE_C5 523.25f
#define NOTE_D5 587.33f
#define NOTE_E5 659.25f
#define NOTE_F5 698.46f
#define NOTE_G5 783.99f
#define NOTE_A5 880.0f


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