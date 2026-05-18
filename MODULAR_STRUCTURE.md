# 📦 Organização Modular do EVA

## Estrutura de Módulos

```
APLICAÇÃO PRINCIPAL (main.cpp)
    ↓
    ├→ 🔌 BATTERY_MANAGEMENT
    │   ├─ getBatteryPercentage()
    │   └─ initBattery()
    │
    ├→ 📱 DISPLAY_OLED  
    │   ├─ drawBatteryIndicator()
    │   ├─ drawStatusText()
    │   ├─ drawSleepCountdown()
    │   ├─ drawWaveform()
    │   └─ updateDisplay()
    │
    ├→ 🔘 INPUT_BUTTON
    │   ├─ isButtonPressed()
    │   ├─ checkSleepButton()
    │   ├─ getButtonPressTimeRemaining()
    │   └─ isButtonPressActive()
    │
    ├→ 📊 SENSOR_MPU6050
    │   ├─ initMPU6050()
    │   ├─ sampleAccelerometer()
    │   └─ getWaveBuffer()
    │
    ├→ ⚡ POWER_MANAGEMENT
    │   ├─ handleWakeFromSleep()
    │   └─ enterDeepSleep()
    │
    ├→ ☁️ MQTT
    │   ├─ initMQTT()
    │   ├─ updateMQTT()
    │   ├─ isMQTTConnected()
    │   └─ enviarDadosParaNuvem()
    │
    └→ ⚙️ CONFIG
        └─ Defines e constantes centralizadas
```

## Arquivos

### Headers (`include/`)
| Arquivo | Propósito |
|---------|-----------|
| `config.h` | Todas as constantes e pinos |
| `battery_management.h` | Leitura de bateria |
| `display_oled.h` | Controle do OLED |
| `input_button.h` | Detecção de botão |
| `sensor_mpu6050.h` | Acelerómetro |
| `power_management.h` | Sleep/wake |
| `mqtt.h` | WiFi e telemetria |

### Implementações (`src/`)
| Arquivo | Módulo |
|---------|--------|
| `main.cpp` | Orquestrador |
| `battery_management.cpp` | Bateria |
| `display_oled.cpp` | Display |
| `input_button.cpp` | Botão |
| `sensor_mpu6050.cpp` | Sensor |
| `power_management.cpp` | Power |
| `mqtt.cpp` | WiFi e MQTT |

## Fluxo de Setup

```
setup()
  ├─ initBattery()          → Configura pino ADC
  ├─ initButton()           → Configura pino GPIO + LED
  ├─ initDisplay()          → Inicializa OLED via I2C
  ├─ initMPU6050()          → Configura sensor I2C
  ├─ handleWakeFromSleep()  → Processa acordar
  └─ delay(100)
```

## Fluxo do Loop

```
loop()
  ├─ checkSleepButton()     → Verifica long-press (5s)
  ├─ sampleAccelerometer()  → Coleta dados 1 kHz
  └─ if (10 Hz)
      └─ renderDisplayFrame()
          ├─ clearDisplay()
          ├─ drawBatteryIndicator()
          ├─ drawStatusText() ou drawSleepCountdown()
          ├─ drawWaveform()
          └─ updateDisplay()
```

## Independência de Módulos

✅ Cada módulo gerencia:
- Suas próprias variáveis estáticas
- Suas próprias bibliotecas (#include)
- Suas próprias inicializações
- Apenas expõe funções públicas

❌ Evita:
- Variáveis globais compartilhadas
- Acoplamento direto entre módulos
- Dependências circulares

## Exemplo: Adicionar Nova Funcionalidade

### Para adicionar Bluetooth (novo módulo):

**1. Criar `include/bluetooth.h`**
```cpp
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

void initBluetooth();
void sendData(const char* data);
void processBluetooth();

#endif
```

**2. Criar `src/bluetooth.cpp`**
```cpp
#include "bluetooth.h"
#include "config.h"

void initBluetooth() { /* ... */ }
void sendData(const char* data) { /* ... */ }
void processBluetooth() { /* ... */ }
```

**3. Atualizar `main.cpp`**
```cpp
#include "bluetooth.h"

void setup() {
  // ... outros inits
  initBluetooth();  // ← Adicionar
}

void loop() {
  // ... outras tasks
  processBluetooth();  // ← Adicionar
}
```

## Benefícios

✨ **Organização** - Cada coisa no seu lugar  
🔧 **Manutenção** - Fácil encontrar e corrigir bugs  
♻️ **Reutilização** - Módulos podem ser copiados para outros projetos  
🧪 **Testes** - Cada módulo pode ser testado independentemente  
📈 **Escalabilidade** - Adicionar features sem quebrar código existente  
🎯 **Clareza** - main.cpp é apenas um orquestrador
