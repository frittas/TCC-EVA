# 📊 Diagrama de Dependências

## Hierarquia de Módulos

```
                      ┌──────────────┐
                      │  main.cpp    │
                      │ Orquestrador │
                      └──────┬───────┘
                             │
           ┌─────────────────┼─────────────────┐
           │                 │                 │
           ▼                 ▼                 ▼
    ┌────────────────┐ ┌─────────────┐ ┌──────────────┐
    │ Battery        │ │ Input       │ │ Sensor       │
    │ Management     │ │ Button      │ │ MPU6050      │
    └────────┬───────┘ └──────┬──────┘ └──────┬───────┘
             │                │               │
             └────────┬───────┴───────┬───────┘
                      │               │
                      ▼               ▼
            ┌──────────────────┐ ┌────────────┐
            │ Display          │ │Power       │
            │ OLED             │ │Management  │
            └──────────────────┘ └────────────┘
                      │
                      ▼
                ┌────────────────┐
                │   CONFIG.h     │
                │ (Constantes)   │
                └────────────────┘
```

## Fluxos de Dados

### Fluxo 1: Medição de Bateria para Display

```
loop() 
  → renderDisplayFrame()
    → drawBatteryIndicator()
      → getBatteryPercentage()
        → analogRead(BATTERY_PIN)
```

### Fluxo 2: Leitura de Sensor para Display

```
loop()
  → sampleAccelerometer()
    → mpu.getEvent()
      → waveBuffer[waveIndex]
  → renderDisplayFrame()
    → drawWaveform()
      → getWaveBuffer()
      → getWaveIndex()
```

### Fluxo 3: Botão para Sleep

```
loop()
  → checkSleepButton()
    ├─ isButtonPressed()
    ├─ getButtonPressTimeRemaining()
    └─ enterDeepSleep()
      → display (via power_management)
        → clearDisplay()
        → drawSleepMessage()
        → updateDisplay()
      → esp_sleep_enable_ext0_wakeup()
```

### Fluxo 4: Wake from Sleep

```
setup()
  → handleWakeFromSleep()
    ├─ isButtonPressed()
    ├─ drawWakeCountdown()
    └─ esp_deep_sleep_start()
```

## Matriz de Inclusões

```
main.cpp
├── config.h
├── battery_management.h
│   └── config.h
│   └── battery_management.cpp
│       └── config.h
├── display_oled.h
│   ├── config.h
│   ├── sensor_mpu6050.h
│   └── display_oled.cpp
│       ├── config.h
│       ├── battery_management.h
│       └── sensor_mpu6050.h
├── input_button.h
│   └── input_button.cpp
│       ├── config.h
│       ├── power_management.h
│       └── input_button.cpp
├── sensor_mpu6050.h
│   └── sensor_mpu6050.cpp
│       ├── config.h
│       └── Adafruit_MPU6050.h (lib)
└── power_management.h
    └── power_management.cpp
        ├── config.h
        ├── display_oled.h
        ├── input_button.h
        └── esp_sleep.h (Arduino)
```

## Princípios de Design

### ✅ SRP (Single Responsibility Principle)
Cada módulo tem **uma única responsabilidade**:
- `battery_management` = apenas bateria
- `display_oled` = apenas display
- `input_button` = apenas botão
- `sensor_mpu6050` = apenas sensor
- `power_management` = apenas power

### ✅ DIP (Dependency Inversion Principle)  
Módulos de alto nível (`main.cpp`) dependem de abstrações (interfaces `.h`), não implementações (`.cpp`)

### ✅ DRY (Don't Repeat Yourself)
Constantes **centralizadas** em `config.h` - uma fonte única da verdade

### ✅ Coesão Alta
Tudo que muda junto está junto:
- Pino de bateria → em `battery_management`
- Lógica de display → em `display_oled`

### ✅ Acoplamento Baixo
Módulos não conhecem implementação uns dos outros, apenas interfaces

## Exemplo: Adicionar WiFi

Se precisar adicionar WiFi, seria:

```
main.cpp (adiciona)
├── wifi.h
└── wifi.cpp (novo módulo)

Sem tocar em:
├── battery_management (independente)
├── display_oled (pode usar para mostrar status WiFi)
├── input_button (independente)
├── sensor_mpu6050 (independente)
└── power_management (independente)
```

## Performance

### Isolamento de Tarefas
- **1 kHz**: Amostragem (sensor_mpu6050)
- **10 Hz**: Renderização (display_oled)
- **Contínuo**: Verificação de botão (input_button)

Cada tarefa roda em seu próprio módulo, sem interferência.

## Conclusão

A estrutura modular garante que o código seja:
- 🧩 **Modular** - Peças independentes
- 🧹 **Limpo** - Cada coisa no seu lugar
- 📈 **Escalável** - Fácil adicionar features
- 🔄 **Reutilizável** - Copie módulos entre projetos
- 🧪 **Testável** - Teste módulos isoladamente
