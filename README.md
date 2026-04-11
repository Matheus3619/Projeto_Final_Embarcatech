# 🎸 Afinador de Guitarra - Raspberry Pi Pico

Um afinador de guitarra portátil e de baixo custo desenvolvido com o **Raspberry Pi Pico (RP2040)**, utilizando processamento de sinais em tempo real para detectar e exibir a frequência das cordas.

> **Projeto desenvolvido para o Embarcatech - Desafio Final**

---

## 📋 Sumário

- [Características](#características)
- [Hardware](#hardware)
- [Instalação e Setup](#instalação-e-setup)
- [Como Usar](#como-usar)
- [Estrutura do Projeto](#estrutura-do-projeto)
- [Documentação Técnica](#documentação-técnica)
- [Compilação e Deploy](#compilação-e-deploy)
- [Troubleshooting](#troubleshooting)

---

## ✨ Características

- **Detecção de Frequência em Tempo Real**: Algoritmo de autocorrelação para identificar a nota musical com precisão.
- **Interface OLED Intuitiva**: Display SSD1306 128x64 mostrando a corda selecionada, frequência detectada e status de afinação.
- **Feedback Visual com LEDs**:
  - 🟢 Verde: Corda afinada (dentro da tolerância)
  - 🔵 Azul: Corda muito baixa (apertar)
  - 🔴 Vermelho: Corda muito alta (afrouxar)
- **Navegação por Joystick**: Seleção suave e contínua das 6 cordas de guitarra.
- **Botões de Controle**: Confirmação (btnA) e retorno ao menu (btnB).
- **Padrão EADGBE**: Afinação convencional de 6 cordas.
- **Processamento Eficiente**: Otimizado para arquitetura ARM Cortex-M0+ do RP2040.

---

## 🔧 Hardware

### Componentes Necesários

| Componente | Quantidade | Notas |
|---|---|---|
| Raspberry Pi Pico W | 1 | Microcontrolador principal |
| Display OLED SSD1306 | 1 | Interface 128x64 I2C |
| Microfone ADC | 1 | Para capturar som da corda |
| Joystick Analógico | 1 | Navegação no menu |
| Botões Momentâneos | 2 | Confirmação e retorno |
| LEDs | 3 | Feedback de status |
| Resistores | Diversos | ~10kΩ para BTNs, ~220Ω para LEDs |
| Capacitores | Diversos | Decoupling e filtragem |

### Pinagem

```c
// I2C (Display OLED)
I2C_SDA = GPIO 14
I2C_SCL = GPIO 15

// ADC (Entrada de Áudio)
MIC_PIN = GPIO 28 (ADC Channel 2)
JOYSTICK_X = GPIO 26 (ADC Channel 0)
JOYSTICK_Y = GPIO 27 (ADC Channel 1)

// Botões (GPIO com Pull-Up Interno)
BTN_A = GPIO 5  (Confirmar corda)
BTN_B = GPIO 6  (Voltar ao menu)

// LEDs (GPIO com Saída)
LED_GREEN  = GPIO 11  (Corda afinada)
LED_BLUE   = GPIO 12  (Corda baixa)
LED_RED    = GPIO 13  (Corda alta)
```

### Diagrama de Conexão

```
Pico Pin    | Componente      | Função
-----------|-----------------|-----------
GP28 (ADC2) | Microfone       | Captura de áudio
GP26 (ADC0) | Joystick X      | Navegação (não usado)
GP27 (ADC1) | Joystick Y      | Navegação vertical
GP5         | Botão A         | Confirmar
GP6         | Botão B         | Retornar
GP11        | LED Verde       | Status afinado
GP12        | LED Azul        | Status baixo
GP13        | LED Vermelho    | Status alto
GP14 (I2C1-SDA) | OLED SDA    | Display
GP15 (I2C1-SCL) | OLED SCL    | Display
GND         | GND             | Terra comum
3.3V        | GND/3.3V        | Alimentação
```

---

## 📦 Instalação e Setup

### Pré-requisitos

- **Sistema Operacional**: Windows, macOS ou Linux
- **Ferramentas**:
  - CMake (v3.13+)
  - ARM GCC Toolchain (arm-none-eabi-gcc)
  - Ninja ou Make
  - Pico SDK v2.2.0
  - VS Code (recomendado) com extensão Raspberry Pi Pico

### Passos de Instalação

1. **Clone o repositório**:
   ```bash
   git clone https://github.com/Matheus3619/Projeto_Final_Embarcatech.git
   cd Projeto_Final_Embarcatech
   ```

2. **Configure variáveis de ambiente** (Windows):
   ```powershell
   $env:PICO_SDK_PATH = "$env:USERPROFILE\.pico-sdk\sdk\2.2.0"
   ```

3. **Crie o diretório de build**:
   ```bash
   mkdir build
   cd build
   ```

4. **Configure com CMake**:
   ```bash
   cmake -G Ninja ..
   ```

5. **Compile**:
   ```bash
   ninja
   ```
   
   **Resultado**: Arquivo `Projeto_final.uf2` gerado em `build/`.

---

## 🎮 Como Usar

### Fluxo de Operação

```
┌─────────────────────┐
│      MENU           │
│  Joystick: Navega   │
│  BtnA: Confirma     │
└─────────────────────┘
         │
         ▼
┌─────────────────────┐
│      IDLE           │
│  Microfone: Aguarda │
│  BtnB: Voltar menu  │
│  Joystick: Amostra  │
└─────────────────────┘
         │
         ▼
┌─────────────────────┐
│    SAMPLING         │
│  Captura 1024       │
│  amostras (128ms)   │
└─────────────────────┘
         │
         ▼
┌─────────────────────┐
│   PROCESSING        │
│  Autocorrelação     │
│  Detecta frequência │
└─────────────────────┘
         │
         ▼
┌─────────────────────┐
│ DISPLAY_RESULT      │
│  Mostra no OLED     │
│  Acende LED status  │
└─────────────────────┘
```

### Passo a Passo

1. **Power On**: Conecte o Pico via USB. O display OLED deve ligar.

2. **Selecione a Corda**:
   - Move o **Joystick para cima/baixo** para navegar entre as 6 cordas.
   - A seta ">>" indica a corda selecionada.
   - Posição do joystick = posição na tela (mapeamento contínuo).

3. **Confirme a Corda**:
   - Pressione **Botão A** para confirmar a corda desejada.
   - O display entra em modo de afinação.

4. **Toque a Corda**:
   - Puxe/plucke a corda selecionada.
   - O microfone captura o som (~128ms).
   - O algoritmo detecta a frequência fundamental.

5. **Interprete o Resultado**:
   - **Display OLED** mostra:
     - Nome da nota (ex: "SOL")
     - Frequência detectada (ex: "196.5Hz")
     - Status: "OK!", "APERTAR CORDA ^", "AFROUXAR CORDA v", "SEM SINAL"
   - **LEDs indicam status**:
     - 🟢 Verde = Afinado (erro < ±1.2 Hz)
     - 🔵 Azul = Muito baixo (apertar)
     - 🔴 Vermelho = Muito alto (afrouxar)

6. **Volte ao Menu**:
   - Pressione **Botão B** para retornar ao menu e selecionar outra corda.

---

## 📂 Estrutura do Projeto

```
Projeto_Final_Embarcatech/
├── CMakeLists.txt                    # Configuração de build
├── pico_sdk_import.cmake            # Import do Pico SDK
├── LICENSE                          # Licença MIT
├── README.md                        # Este arquivo
├── Projeto_final.c                  # Arquivo principal (máquina de estados)
│
├── inc/                             # Headers do projeto
│   ├── config.h                     # Definições de pinos e parâmetros
│   ├── menu.h                       # Interface do menu
│   ├── display.h                    # Interface do display
│   ├── processamento_de_audio.h     # Interface do processamento de áudio
│   ├── ssd1306.h                    # Driver OLED (header)
│   ├── ssd1306_i2c.h                # Camada I2C para OLED
│   └── ssd1306_font.h               # Fontes para display
│
├── src/                             # Implementações
│   ├── menu.c                       # Lógica do menu (navegação)
│   ├── display.c                    # Funções do display
│   └── processamento_de_audio.c     # Detecção de frequência (autocorrelação)
│
├── inc/ssd1306_i2c.c                # Driver OLED I2C (implementação)
│
└── build/                           # Diretório de build (gerado)
    ├── Projeto_final.elf            # Executável ELF
    ├── Projeto_final.uf2           # Firmware (para upload)
    └── ...
```

### Explicação dos Arquivos Principais

| Arquivo | Descrição |
|---------|-----------|
| `Projeto_final.c` | Máquina de estados principal (MENU, IDLE, SAMPLING, PROCESSING, DISPLAY_RESULT) |
| `processamento_de_audio.c` | Implementa o algoritmo de autocorrelação para detectar frequência |
| `menu.c` | Gerencia navegação por joystick e renderização no OLED |
| `display.c` | Funções auxiliares para desenhar strings e atualizações no display |
| `config.h` | Centraliza todas as definições (pinos, taxa de amostragem, tolerâncias) |

---

## 🔬 Documentação Técnica

### Algoritmo de Detecção: Autocorrelação

O afinador utiliza **autocorrelação** para detectar a frequência fundamental de uma nota musical.

#### Conceito

Para um sinal periódico (como uma nota musical), medir a correlação entre o sinal e uma versão atrasada dele identifica o **período**, que é inversamente proporcional à frequência.

$$f = \frac{f_s}{\text{período}}$$

### Passo a Passo

#### 1. Preparação do Sinal

O ADC captura valores de 0 a 4095, centrados em 2048. Remove-se o offset DC:

$$\text{sinal}[i] = \text{adc\_valor}[i] - 2048$$

Isso isola apenas a oscilação da onda (picos positivos e negativos).

#### 2. Busca por Atrasos

O algoritmo testa atrasos de 20 a 512 amostras:
- **Atraso mínimo (20)**: Filtra ruído de alta frequência (> 400 Hz).
- **Atraso máximo (512)**: Detecta frequências baixas (~15.6 Hz em diante).

#### 3. Cálculo da Correlação

Para cada atraso, computa o **produto ponto** entre o sinal original e a versão atrasada:

$$\text{correlação}(\text{atraso}) = \sum_{i=0}^{n/2-1} [\text{sinal}[i] \times \text{sinal}[i + \text{atraso}]]$$

Se o período real = atraso, as duas versões se alinham perfeitamente, resultando em correlação máxima.

#### 4. Identificação do Melhor Atraso

Encontra o `atraso` com maior correlação:

$$\text{melhor\_atraso} = \arg\max(\text{correlação})$$

#### 5. Cálculo da Frequência

$$f_{\text{detectada}} = \frac{f_s}{\text{melhor\_atraso}} = \frac{8000}{\text{melhor\_atraso}}$$

Se `melhor_atraso = 100`, então $f = 80$ Hz.

### Tolerância de Afinação

O status é determinado pela diferença em Hz:

```c
float erro_hz = freq_detectada - freq_alvo;

if (erro_hz < 50) {
    // SEM SINAL
} else if (fabs(erro_hz) < 1.2) {
    // AFINADO (🟢 Verde)
} else if (erro_hz < 0) {
    // MUITO BAIXO - APERTAR (🔵 Azul)
} else {
    // MUITO ALTO - AFROUXAR (🔴 Vermelho)
}
```

**Tolerância**: ±1.2 Hz para afinação precisa.

#### Complexidade e Performance

- **Complexidade**: O(n²) onde n = 1024
- **Operações**: ~500k multiplicações por medição
- **Tempo**: ~50-100ms no RP2040 (aceitável para UX interativa)
- **Alternativas**: FFT reduce para O(n log n), mas autocorrelação é simples e funcional para este caso.

### Parâmetros de Afinação

Notas padrão em Hz (Afinação 12-TET):

| Corda | Nota | Frequência (Hz) |
|-------|------|-----------------|
| 6ª | Mi (E2) | 82.30 |
| 5ª | Lá (A2) | 110.00 |
| 4ª | Ré (D3) | 146.83 |
| 3ª | Sol (G3) | 196.00 |
| 2ª | Si (B3) | 246.94 |
| 1ª | Mi (E4) | 329.62 |

---

## 🛠️ Compilação e Deploy

### Build via VS Code (Windows)

1. **Abra VS Code** com a extensão Raspberry Pi Pico ativada.
2. **Clique em "Compile Project"** (ou use Ctrl+Shift+B).
3. **Aguarde** a compilação finalizar (build normalmente leva ~10s).
4. **Verificação**: Se vir "build successful", o arquivo `.uf2` foi gerado em `build/`.

### Build via Terminal

```bash
cd build
ninja  # ou: make
```

### Upload para Raspberry Pi Pico

#### Método 1: Bootloader BOOTSEL (Recomendado)

1. **Desconecte** o Pico.
2. **Segure BOOTSEL** (botão na placa) e reconecte via USB.
3. **Arraste** `Projeto_final.uf2` para o volume BOOT1 (após 2-3s, entra em modo memory storage).
4. **Pronto**: Pico rebootará automaticamente e executará o firmware.

#### Método 2: Picotool (via Terminal)

```bash
# Com Pico conectado em modo normal
picotool load Projeto_final.uf2 -fx
```

#### Método 3: VS Code Task

Clique em "Run Project" (já está configurado).

### Verificação

- **Serial Output**: Abra terminal serial a 115200 baud (via USB).
- **Display OLED**: Deve mostrar "Escolha a corda:" com menu.
- **LEDs**: Devem acender ao detectar frequência.

---

## 🚨 Troubleshooting

### Problema: Display OLED não aparece

**Causas Possíveis**:
- I2C não inicializado corretamente
- Pinos SDA/SCL trocados
- Problemas de alimentação no display

**Solução**:
```c
// Verifique em config.h:
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_PORT i2c1

// Verifique se i2c_init foi chamado e pull-ups foram ativados
gpio_pull_up(I2C_SDA);
gpio_pull_up(I2C_SCL);
```

### Problema: Botões não respondem

**Causas Possíveis**:
- Debounce muito agressivo
- Interrupção não configurada antes de gpio_init
- Pinos GPIO não inicializados

**Solução**:
```c
// Certifique-se da ordem:
setup();  // Inicializa GPIO
gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &button_callback);
gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, &button_callback);
```

### Problema: Frequência não detectada (sempre "SEM SINAL")

**Causas Possíveis**:
- Microfone não conectado ou com volume baixo
- ADC não configurado corretamente
- Ruído muito alto ou muito baixo

**Solução**:
```bash
# Adicione printf para debug em Projeto_final.c:
printf("y_val=%d, freq_detectada=%f, freq_alvo=%f\n", y_val, freq_detectada, afinacao_padrao[selected_string].freq_alvo);

# Teste com onda senoidal gerada (ex: via app de gerador de tons)
```

### Problema: Joystick captura valores invertidos

**Causa**: Mapeamento Y do ADC invertido.

**Solução**:
```c
// Em src/menu.c, se o joystick estiver invertido:
*selected = ((4095 - y_val) * qtd) / 4096;  // Inverte
// ou troque por:
*selected = (y_val * qtd) / 4096;  // Normal
```

### Problema: LEDs não acendem

**Causas Possíveis**:
- GPIOs de LED não configurados como OUTPUT
- LED ou resistor com problemas
- Polaridade invertida

**Solução**:
```c
// Verifique em setup():
for(int i=0; i < 3; i++){
    gpio_init(LEDS[i]);
    gpio_set_dir(LEDS[i], GPIO_OUT);  // IMPORTANTE: GPIO_OUT
}

// Teste acender LED no main:
gpio_put(LEDS[0], 1);  // Verde ON
sleep_ms(1000);
gpio_put(LEDS[0], 0);  // Verde OFF
```

---

## 📊 Performance

### Benchmarks

| Métrica | Valor |
|---------|-------|
| Taxa de Amostragem | 8 kHz (125 µs entre amostras) |
| Tempo de Processamento | ~50-100 ms |
| Taxa de Atualização | ~1-2 medições/segundo |
| Memória RAM Usada | ~50-60 kB de 264 kB |
| Memória Flash Usada | ~120-150 kB de 2 MB |
| Consumo de Energia | ~30-50 mA (ativo), ~1-5 mA (idle) |

### Otimizações Aplicadas

✅ Selecionado `-O3` no CMake (otimização agressiva)
✅ Remoção de printf em release
✅ Debounce de 200ms para evitar processamento desnecessário
✅ Buffer estático para ADC
✅ Máquina de estados eficiente

---

## 📝 Licença

Este projeto está licenciado sob a **MIT License**. Veja [LICENSE](LICENSE) para detalhes.

---

## 👤 Autor

**Matheus Souza** - Projeto Final Embarcatech 2024-2025

- GitHub: [@Matheus3619](https://github.com/Matheus3619)
- Repositório: [Projeto_Final_Embarcatech](https://github.com/Matheus3619/Projeto_Final_Embarcatech)

---

## 🤝 Contribuindo

Contribuições são bem-vindas! Para sugerir melhorias:

1. Fork o repositório
2. Crie uma branch (`git checkout -b feature/nova-feature`)
3. Commit as mudanças (`git commit -m 'Adiciona nova feature'`)
4. Push para a branch (`git push origin feature/nova-feature`)
5. Abra um Pull Request

### Ideias para Melhoria

- [ ] Implementar FFT para melhor detecção
- [ ] Suporte a outros padrões de afinação (low-E, drop-D, etc.)
- [ ] Calibração do microfone via EEPROM
- [ ] Modo bateria com low-power
- [ ] Interface de histórico em OLED
- [ ] Conectividade BLE para app mobile

---

## 📚 Referências

- [Raspberry Pi Pico Documentation](https://www.raspberrypi.com/documentation/microcontrollers/pico.html)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
- [Pico SDK API](https://raspberrypi.github.io/pico-sdk-doxygen/)
- [SSD1306 OLED Driver](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

---

**Última atualização**: Abril de 2026

