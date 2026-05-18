# Arquitetura do Projeto EVA - Monitoramento de Vibração

## Estrutura Modular

O projeto foi reorganizado em módulos separados com responsabilidades bem definidas:

### 📋 Módulos

#### 1. **config.h** - Configurações Globais
- Todas as constantes e defines do projeto
- Pinos (I2C, GPIO, sensores)
- Parâmetros de bateria
- Configurações de OLED e amostragem

#### 2. **battery_management** (battery_management.h / .cpp)
- Leitura do ADC para medição de bateria
- Cálculo de percentual de carga (0-100%)
- Inicialização do pino de bateria

**Funções públicas:**
- `initBattery()` - Inicializa pino de leitura
- `getBatteryPercentage()` - Retorna percentual de bateria

#### 3. **display_oled** (display_oled.h / .cpp)
- Gerenciamento completo do display OLED SSD1306
- Desenho de interface, indicadores e gráficos
- Buffer isolado para display

**Funções públicas:**
- `initDisplay()` - Inicializa OLED via I2C
- `clearDisplay()` - Limpa e prepara para desenho
- `drawBatteryIndicator()` - Desenha ícone de bateria
- `drawStatusText()` - Escreve texto de status
- `drawSleepCountdown()` - Exibe contagem regressiva de sleep
- `drawWakeCountdown()` - Exibe contagem regressiva de wake
- `drawSleepMessage()` - Mensagem ao entrar em sleep
- `drawWaveform()` - Desenha gráfico da onda
- `updateDisplay()` - Atualiza tela

#### 4. **input_button** (input_button.h / .cpp)
- Gerenciamento de botão e entrada de usuário
- Detecção de long-press (5 segundos)
- Rastreamento de estado do botão

**Funções públicas:**
- `initButton()` - Inicializa pino de botão
- `isButtonPressed()` - Verifica se botão está pressionado
- `checkSleepButton()` - Verifica long-press para sleep
- `resetButtonState()` - Reseta estado
- `getButtonPressTimeRemaining()` - Tempo restante para sleep
- `isButtonPressActive()` - Verifica se há pressão ativa

#### 5. **sensor_mpu6050** (sensor_mpu6050.h / .cpp)
- Comunicação I2C com MPU6050
- Amostragem a 1 kHz
- Buffer circular de forma de onda

**Funções públicas:**
- `initMPU6050()` - Inicializa sensor e configura ranges
- `sampleAccelerometer()` - Coleta dados do acelerómetro
- `getWaveBuffer()` - Referência ao buffer de onda
- `getWaveIndex()` - Índice atual no buffer

#### 6. **power_management** (power_management.h / .cpp)
- Controle de sleep/wake do ESP32
- Deep sleep com interrupção externa
- Contagem regressiva de wake

**Funções públicas:**
- `handleWakeFromSleep()` - Processa acordar do deep sleep
- `enterDeepSleep()` - Entra em deep sleep

#### 7. **mqtt** (mqtt.h / .cpp)
- Conexão com broker MQTT (ThingsBoard) usando Access Token.
- Publicação de telemetria (TinyML, RMS, bateria e status) a cada 5 segundos.
- Injeção de segurança: Token carregado de arquivo local via script de pré-build.

**Funções públicas:**
- `initMQTT()`: Inicializa o cliente e tenta conexão.
- `updateMQTT()`: Mantém o loop MQTT e gerencia o timer de envio de telemetria.
- `isMQTTConnected()`: Verifica status da conexão para o display.
- `enviarDadosParaNuvem()`: Processa dados brutos, calcula RMS e agenda o envio do payload JSON.

#### 8. **wifi_management** (wifi_management.h / .cpp)
- Gestão de conexão WiFi (STA com fallback para AP).
- Centralização de credenciais de rede.

**Funções públicas:**
- `initWiFi()`: Inicializa a rede.
- `ensureWiFiConnected()`: Verifica e recupera conexão.
- `getWiFiRSSI()`: Força do sinal.

#### 9. **main.cpp** - Orquestrador
- Inicialização de todos os módulos
- Loop principal com tarefas priorizadas
- Renderização de frames

### 📊 Diagrama de Responsabilidades

```
┌─────────────────────────────────────┐
│          main.cpp                   │
│  (Orquestração e loop principal)    │
└─────────────────┬───────────────────┘
                  │
        ┌─────────┼─────────┬─────────────┬──────────┐
        │         │         │             │          │
        ▼         ▼         ▼             ▼          ▼
    ┌───────┐ ┌────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐
    │Battery│ │Display │ │ Button   │ │Sensor    │ │  MQTT    │
    │Mgmt   │ │OLED    │ │Input     │ │MPU6050   │ │ (Cloud)  │
    └───────┘ └────────┘ └──────────┘ └──────────┘ └──────────┘
        │         │         │             │          │
        └─────────┼─────────┼─────────────┴──────────┘
                  │
            ┌─────┴──────┐
            │            │
            ▼            ▼
        ┌────────────┐  ┌──────────────┐
        │Power Mgmt  │  │Config (pins, │
        │Sleep/Wake  │  │ params, etc) │
        └────────────┘  └──────────────┘
```

### 🔄 Fluxo do Loop Principal

```
loop()
├── Task 1: Verificar botão (long-press)
│   └── checkSleepButton() → pode chamar enterDeepSleep()
│
├── Task 2: Amostrar acelerómetro (1 kHz - alta prioridade)
│   └── sampleAccelerometer() → preenche buffer de onda
│
└── Task 3: Atualizar display (10 Hz - baixa prioridade)
    └── renderDisplayFrame()
        ├── clearDisplay()
        ├── drawBatteryIndicator()
        ├── drawStatusText() ou drawSleepCountdown()
        ├── drawWaveform()
        └── updateDisplay()
```

### 📁 Estrutura de Arquivos

```
include/
├── config.h
├── battery_management.h
├── display_oled.h
├── input_button.h
├── sensor_mpu6050.h
├── power_management.h
└── mqtt.h

src/
├── main.cpp
├── battery_management.cpp
├── display_oled.cpp
├── input_button.cpp
├── sensor_mpu6050.cpp
└── power_management.cpp
└── mqtt.cpp
```

### ✅ Benefícios da Estrutura Modular

1. **Separação de Responsabilidades** - Cada módulo tem um único propósito
2. **Reutilização** - Módulos podem ser reutilizados em outros projetos
3. **Testabilidade** - Módulos individuais podem ser testados isoladamente
4. **Manutenibilidade** - Código mais limpo e organizado
5. **Escalabilidade** - Fácil adicionar novos recursos sem contaminar main.cpp
6. **Isolamento de Dependências** - Cada módulo gerencia suas próprias bibliotecas

### 🔧 Como Estender

Para adicionar uma nova funcionalidade (ex: SD card, Wi-Fi):

1. Criar `include/nova_funcionalidade.h` com interface pública
2. Criar `src/nova_funcionalidade.cpp` com implementação
3. Adicionar `#include "nova_funcionalidade.h"` em main.cpp
4. Chamar funções de inicialização em `setup()`
5. Integrar lógica no `loop()` conforme necessário

### 📝 Notas

- Todos os #defines de pinos e configurações estão centralizados em `config.h`
- Estados e buffers são privados para cada módulo (usando `static`)
- Apenas funções necessárias são expostas nos headers
- Sem variáveis globais compartilhadas entre módulos (exceto config)
