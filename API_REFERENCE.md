# 🔍 Quick Reference - API de Cada Módulo

## 📍 config.h
**Constantes centralizadas - Não tem código, apenas #defines**

```cpp
I2C_SDA = 8
I2C_SCL = 9
BUTTON_PIN = 4
BATTERY_PIN = 1
LONG_PRESS_MS = 5000
BATTERY_MAX_VOLTAGE = 4.2
BATTERY_MIN_VOLTAGE = 3.0
SCREEN_WIDTH = 128
SCREEN_HEIGHT = 64
WAVE_BUFFER_SIZE = 128
SAMPLE_INTERVAL = 1000 µs
DISPLAY_INTERVAL = 100 ms
```

---

## 🔌 battery_management

### Arquivo: `battery_management.h/cpp`

**Responsabilidade:** Leitura do ADC e cálculo de bateria

### Funções Públicas

```cpp
void initBattery()
// Inicializa o pino de leitura de bateria como INPUT
// Chamado em: setup()

int getBatteryPercentage()
// Retorna: percentual de bateria (0-100)
// Usa: analogRead(BATTERY_PIN)
// Retorna valor escalonado entre BATTERY_MIN_VOLTAGE e BATTERY_MAX_VOLTAGE
```

### Variáveis Privadas
```cpp
static - (nenhuma, apenas usa pino)
```

---

## 📱 display_oled

### Arquivo: `display_oled.h/cpp`

**Responsabilidade:** Gerenciar display SSD1306 OLED

### Funções Públicas

```cpp
void initDisplay()
// Inicializa Wire (I2C) e o display SSD1306
// Chamado em: setup()

void clearDisplay()
// Limpa buffer do display e seta cor/tamanho de texto
// Preprara para desenhar

void drawBatteryIndicator()
// Desenha ícone de bateria no canto superior direito
// Mostra percentual ao lado

void drawStatusText(const char* text)
// Desenha texto de status na linha 16
// Exemplo: "EVA - Monitorando..."

void drawSleepCountdown(unsigned long remainingMs)
// Desenha contagem regressiva do sleep (durante loop)
// Formato: "Segure 5s para dormir" + "Faltam: X s"

void drawWakeCountdown(unsigned long remainingMs)
// Desenha contagem regressiva do wake (após acordar)
// Formato: "Acordar: mantenha" + "Faltam: X s"

void drawSleepMessage()
// Desenha mensagem ao entrar em deep sleep
// Formato: 3 linhas de texto informativos

void drawWaveform(int waveOffset)
// Desenha gráfico da onda do acelerómetro
// waveOffset: posição Y onde começa o gráfico

void updateDisplay()
// Atualiza tela física com buffer desenho
// display.display()
```

### Variáveis Privadas
```cpp
static Adafruit_SSD1306 display(...)
// Objeto do display (encapsulado)
```

---

## 🔘 input_button

### Arquivo: `input_button.h/cpp`

**Responsabilidade:** Gerenciar botão e entrada de usuário

### Funções Públicas

```cpp
void initButton()
// Inicializa LED_BUILTIN e BUTTON_PIN como INPUT_PULLUP
// Chamado em: setup()

bool isButtonPressed()
// Retorna: true se botão está pressionado
// Lê: digitalRead(BUTTON_PIN) == LOW

void checkSleepButton(unsigned long currentMillis)
// Verifica long-press (5 segundos)
// Se > 5s: chama enterDeepSleep()
// Chamado em: loop() a cada iteração

void resetButtonState()
// Reseta contador de pressão
// Uso manual se precisar resetar estado

unsigned long getButtonPressTimeRemaining(unsigned long currentMillis)
// Retorna: tempo restante em ms até completar 5s
// Uso: para exibir contagem regressiva no display

bool isButtonPressActive()
// Retorna: true se há pressão ativa em andamento
// Uso: para renderizar UI diferente durante pressão
```

### Variáveis Privadas
```cpp
static unsigned long buttonPressStart = 0
// Marca quando começou a pressão do botão
```

---

## 📊 sensor_mpu6050

### Arquivo: `sensor_mpu6050.h/cpp`

**Responsabilidade:** Sensor de aceleração

### Funções Públicas

```cpp
void initMPU6050()
// Inicializa sensor via I2C (0x68)
// Configura ranges de aceleração e giroscópio
// Imprime status via Serial
// Chamado em: setup()

void sampleAccelerometer()
// Coleta dados do MPU6050 se passou SAMPLE_INTERVAL (1 kHz)
// Armazena em: waveBuffer[waveIndex]
// Chamado em: loop() a cada iteração

float* getWaveBuffer()
// Retorna: ponteiro ao buffer de onda
// Uso: para desenhar gráfico (display_oled)

int* getWaveIndex()
// Retorna: ponteiro ao índice atual no buffer
// Uso: para saber onde estamos no buffer circular
```

### Variáveis Privadas
```cpp
static Adafruit_MPU6050 mpu
static float waveBuffer[WAVE_BUFFER_SIZE]    // Buffer circular
static int waveIndex = 0                     // Índice atual
static unsigned long lastSampleTime = 0      // Timing de amostragem
```

---

## ⚡ power_management

### Arquivo: `power_management.h/cpp`

**Responsabilidade:** Sleep, wake e controle de energia

### Funções Públicas

```cpp
void handleWakeFromSleep()
// Processa acordar do deep sleep
// Mostra contagem regressiva para confirmar wake
// Se soltar botão cedo: volta a dormir
// Se manter 5s: continua acordado
// Chamado em: setup()

void enterDeepSleep()
// Mostra mensagem no display
// Aguarda soltar botão
// Ativa interrupção EXT0 no BUTTON_PIN
// Entra em esp_deep_sleep_start()
// Chamado em: input_button.checkSleepButton() após 5s
```

### Dependências Internas
```cpp
Usa: isButtonPressed() de input_button
Usa: clearDisplay(), drawSleepMessage(), drawWakeCountdown(), updateDisplay() de display_oled
```

---

## 🚀 main.cpp

### Arquivo: `main.cpp`

**Responsabilidade:** Orquestração

### Funções

```cpp
void setup()
// Inicializa Serial
// Chama init de todos os módulos:
// - initBattery()
// - initButton()
// - initDisplay()
// - initMPU6050()
// - handleWakeFromSleep()

void loop()
// Task 1: Verifica botão (contínuo)
// - checkSleepButton()
// 
// Task 2: Amostra sensor (1 kHz)
// - sampleAccelerometer()
//
// Task 3: Renderiza display (10 Hz = 100 ms)
// - renderDisplayFrame()

void renderDisplayFrame(unsigned long currentMillis)
// Limpa display
// Desenha bateria
// Desenha status ou contagem regressiva
// Desenha onda
// Atualiza tela física
```

### Variáveis Privadas
```cpp
static unsigned long lastDisplayTime = 0   // Timing do display
```

---

## 📞 Como Usar Cada Módulo

### Exemplo 1: Apenas Ler Bateria
```cpp
#include "battery_management.h"
#include "config.h"

void setup() {
  initBattery();
}

void loop() {
  int percent = getBatteryPercentage();
  Serial.println(percent);
  delay(1000);
}
```

### Exemplo 2: Apenas Exibir no Display
```cpp
#include "display_oled.h"

void setup() {
  initDisplay();
}

void loop() {
  clearDisplay();
  drawStatusText("Hello World!");
  updateDisplay();
  delay(1000);
}
```

### Exemplo 3: Apenas Ler Sensor
```cpp
#include "sensor_mpu6050.h"
#include "config.h"

void setup() {
  initMPU6050();
}

void loop() {
  sampleAccelerometer();
  float* buffer = getWaveBuffer();
  Serial.println(buffer[0]);  // Primeiro valor
}
```

---

## ✅ Checklist de Uso

- [ ] Todas as constantes em `config.h`?
- [ ] Cada módulo tem `.h` e `.cpp`?
- [ ] `#ifndef` guards nos headers?
- [ ] Funções públicas declaradas em `.h`?
- [ ] Variáveis privadas são `static` em `.cpp`?
- [ ] `main.cpp` apenas orquestra?
- [ ] Sem variáveis globais compartilhadas?
- [ ] Sem `#include` de `.cpp` em `.cpp`?
