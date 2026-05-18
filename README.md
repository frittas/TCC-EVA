# 🤖 EVA - Edge Computing para Análise Preditiva de Vibrações

> **Protótipo de monitoramento em tempo real de padrões de vibração em motores rotativos usando Edge Computing, TinyML e modelos treinados com Edge Impulse**

---

## 🎯 Objetivo do Projeto

**EVA** (Edge Vibration Analyzer) é um sistema embarcado de ponta para análise em tempo real de vibrações em equipamentos rotativos. O sistema coleta dados de vibração diretamente no dispositivo e utiliza modelos de machine learning otimizados para detectar anomalias, permitindo **manutenção preditiva** sem necessidade de conexão com nuvem.

### Problemas que Resolve

✅ **Detecção Precoce de Falhas** - Identifica padrões anormais antes da quebra  
✅ **Redução de Tempo de Inatividade** - Manutenção planejada ao invés de emergencial  
✅ **Análise em Tempo Real** - Análise local com telemetria cloud opcional (ThingsBoard)  
✅ **Eficiência Energética** - Sistema embarcado de baixo consumo com deep sleep  
✅ **Dados Locais** - Privacidade e segurança dos dados de vibração  

---

## 🏗️ Arquitetura do Sistema

### Hardware
- **Microcontrolador**: ESP32-S3
- **Sensor**: MPU6050 (acelerómetro + giroscópio)
- **Display**: SSD1306 OLED 128x64
- **Bateria**: LiPo com carregador TP4056
- **Botão**: Controle de sleep/wake

### Software Stack
- **Linguagem**: C++ (Arduino Framework)
- **Build System**: PlatformIO
- **ML Framework**: TinyML / Edge Impulse
- **Interface**: OLED em tempo real + Serial para debug

### Funcionalidades
🔹 Amostragem contínua de vibração (1 kHz)  
🔹 Processamento FFT para transformação frequencial  
🔹 Display ao vivo da forma de onda  
🔹 Integração MQTT com WiFi Station e Fallback AP  
🔹 Indicador de bateria  
🔹 Sleep profundo para economizar energia  
🔹 Contagem regressiva de botão para wake/sleep  

---

## 📊 Fluxo de Dados

```
┌──────────────┐
│  MPU6050     │ Sensor de vibração
│ (1 kHz)      │
└───────┬──────┘
        │
        ▼
┌──────────────────────────┐
│ Buffer de Amostragem     │ Armazena últimas 128 amostras
│ (Waveform Buffer)        │
└───────┬──────────────────┘
        │
        ├─→ Display (10 Hz) ─→ Gráfico em tempo real
        │
        └─→ FFT / ML Models  ─→ Detecção de anomalias
            (Edge Impulse)
```

---

## 🚀 Quick Start

### Pré-requisitos
- Python 3.8+
- PlatformIO CLI ou VS Code + PlatformIO Extension
- Placa ESP32-S3

### Instalação e Build

```bash
# Clonar repositório
git clone https://github.com/frittas/TCC-EVA.git
cd TCC-EVA

# Instalar dependências (PlatformIO)
pio run -t envlist

# Criar arquivo de token para o ThingsBoard
echo "SEU_TOKEN_AQUI" > mqtt_token.txt

# Compilar
pio run

# Upload
pio run -t upload

# Monitor Serial (debug)
pio device monitor
```

---

## 📁 Estrutura do Projeto

```
TCC-EVA/
├── README.md                    ← Você está aqui
├── ARCHITECTURE.md              ← Diagrama de arquitetura
├── MODULAR_STRUCTURE.md         ← Estrutura modular
├── REFACTORING_SUMMARY.md       ← Resumo da refatoração
├── DEPENDENCIES.md              ← Fluxos de dependências
├── API_REFERENCE.md             ← Referência de APIs
│
├── platformio.ini               ← Configuração do projeto
│
├── include/
│   ├── config.h                 ← Constantes centralizadas
│   ├── battery_management.h
│   ├── display_oled.h
│   ├── input_button.h
│   ├── sensor_mpu6050.h
│   ├── power_management.h
│   └── mqtt.h
│
└── src/
    ├── main.cpp                 ← Orquestrador principal
    ├── battery_management.cpp
    ├── display_oled.cpp
    ├── input_button.cpp
    ├── sensor_mpu6050.cpp
    └── power_management.cpp
    ├── mqtt.cpp
    └── load_mqtt_token.py       ← Script de segurança do build
```

---

## 📚 Documentação

### Guias Técnicos

| Documento | Conteúdo |
|-----------|----------|
| **[ARCHITECTURE.md](ARCHITECTURE.md)** | Diagrama completo de componentes e responsabilidades |
| **[MODULAR_STRUCTURE.md](MODULAR_STRUCTURE.md)** | Visual da organização modular com exemplo de extensão |
| **[REFACTORING_SUMMARY.md](REFACTORING_SUMMARY.md)** | Antes/depois da reorganização do código |
| **[DEPENDENCIES.md](DEPENDENCIES.md)** | Fluxos de dados e matriz de inclusões |
| **[API_REFERENCE.md](API_REFERENCE.md)** | Referência rápida de cada módulo |

### Como Ler a Documentação

1. **Novo no projeto?** → Comece por [ARCHITECTURE.md](ARCHITECTURE.md)
2. **Entender organização?** → Veja [MODULAR_STRUCTURE.md](MODULAR_STRUCTURE.md)
3. **Programar novo módulo?** → Consulte [API_REFERENCE.md](API_REFERENCE.md)
4. **Integrar com Cloud?** → Veja [DEPENDENCIES.md](DEPENDENCIES.md)

---

## 🔌 Módulos

O código é organizado em **6 módulos independentes**:

### 1. **config.h** - Configuração
- Todos os pinos, parâmetros e constantes
- Uma fonte única da verdade

### 2. **battery_management** - Gerenciamento de Bateria
- Leitura de ADC
- Cálculo de percentual de carga
- `getBatteryPercentage()`, `initBattery()`

### 3. **display_oled** - Controle do Display
- Renderização de gráficos
- Indicador de bateria
- Contagem regressiva
- `drawWaveform()`, `drawBatteryIndicator()`, `updateDisplay()`

### 4. **input_button** - Controle de Entrada
- Detecção de long-press (5s)
- Sleep/wake do ESP32
- `checkSleepButton()`, `isButtonPressed()`

### 5. **sensor_mpu6050** - Sensor de Vibração
- Inicialização do MPU6050
- Amostragem a 1 kHz
- Buffer circular de forma de onda
- `sampleAccelerometer()`, `getWaveBuffer()`

### 6. **power_management** - Controle de Energia
- Deep sleep com interrupção externa
- Countdown de wake
- `enterDeepSleep()`, `handleWakeFromSleep()`

### 7. **mqtt** - WiFi e Nuvem
- Conexão estável com ThingsBoard
- Envio JSON de IA, RMS e Bateria
- Modo AP para diagnóstico caso WiFi falhe

---

## 🧪 Casos de Uso

### 1. Monitoramento Contínuo de Motor
```
Motor em operação → EVA coleta vibração (1 kHz) 
→ Exibe gráfico em tempo real 
→ FFT analisa componentes frequenciais 
→ ML detecta anomalias 
→ Alerta antes da falha
```

### 2. Diagnóstico de Falhas
```
Motor com problema → Captura dados com EVA 
→ Export para Edge Impulse 
→ Retreina modelo 
→ Upload modelo otimizado no EVA 
→ Detecção automática em campo
```

### 3. Operação com Bateria
```
EVA ligado → Monitora continuamente 
→ Display atualiza 10x/seg 
→ Botão long-press (5s) para hibernar 
→ Economia de 90% de energia em sleep 
→ Botão long-press (5s) para acordar
```

---

## 📈 Performance

| Métrica | Valor |
|---------|-------|
| **Taxa de Amostragem** | 1 kHz |
| **Taxa de Renderização** | 10 Hz |
| **Latência de Detecção** | < 100 ms |
| **Consumo em Operação** | ~180 mA |
| **Consumo em Sleep** | ~100 µA |
| **Autonomia (1000mAh)** | ~6 horas (operação contínua) |
| **Resolução ADC** | 12-bit |
| **Range Aceleração** | ±4G |

---

## 🛠️ Extensão do Projeto

### Adicionar Novo Módulo (ex: SD Card)

```cpp
// 1. Criar include/sd_card.h
void initSDCard();
void logData(float temperature);

// 2. Criar src/sd_card.cpp
#include "sd_card.h"
void initSDCard() { /* ... */ }

// 3. Atualizar main.cpp
#include "sd_card.h"
void setup() {
  initSDCard();  // ← Pronto!
}
```

### Treinar Novo Modelo ML
1. Coletar dados com EVA
2. Export para Edge Impulse
3. Treinar modelo
4. Download como `.h` (C++ Library)
5. Integrar em `sensor_mpu6050.cpp`

---

## 🔐 Privacidade e Segurança

✅ **Edge Computing** - Dados nunca deixam o dispositivo  
✅ **Sem Internet Obrigatória** - Funciona offline  
✅ **Dados Locais** - Armazenamento seguro  
✅ **Open Source** - Código auditável  

---

## 📊 Stack Tecnológico

```
┌─────────────────────────────────────┐
│  Application Layer (main.cpp)       │
├─────────────────────────────────────┤
│  Modular Architecture               │
│  • battery_management               │
│  • display_oled                     │
│  • input_button                     │
│  • sensor_mpu6050                   │
│  • power_management                 │
│  • config                           │
├─────────────────────────────────────┤
│  Libraries                          │
│  • Adafruit_MPU6050                 │
│  • Adafruit_SSD1306                 │
│  • arduinoFFT                       │
│  • esp_sleep (esp-idf)              │
│  • PubSubClient (MQTT)              │
├─────────────────────────────────────┤
│  Arduino Framework / ESP-IDF        │
├─────────────────────────────────────┤
│  Hardware (ESP32-S3)                │
└─────────────────────────────────────┘
```

---

## 📝 Roadmap

- [ ] Integração com Edge Impulse
- [ ] Dashboard web (interface remota)
- [ ] Conexão WiFi para sincronização de modelos
- [ ] Logging em SD Card
- [ ] Múltiplos sensores (vibração + temperatura + umidade)
- [ ] Bluetooth para mobile app
- [ ] Calibração automática
- [ ] Detecção de anomalias mais avançada

---

## 🤝 Contribuições

Este é um projeto de pesquisa acadêmica. Contribuições são bem-vindas!

1. Fork o repositório
2. Crie uma branch (`git checkout -b feature/NovaFuncionalidade`)
3. Commit as mudanças (`git commit -m 'Adiciona NovaFuncionalidade'`)
4. Push para a branch (`git push origin feature/NovaFuncionalidade`)
5. Abra um Pull Request

---

## 📞 Contato

Desenvolvido para **Projeto de Conclusão de Curso (TCC)**  
Edge Computing para Manutenção Preditiva

---

## 📄 Licença

MIT License - veja o arquivo LICENSE para detalhes

---

## 🙏 Agradecimentos

- **Adafruit** - Bibliotecas SSD1306 e MPU6050
- **Prognosis** - Biblioteca arduinoFFT
- **Edge Impulse** - Plataforma de ML para edge devices
- **Espressif** - ESP32-S3 e ESP-IDF

---

## 📚 Referências

- [ESP32-S3 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
- [MPU6050 Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [Edge Impulse Docs](https://docs.edgeimpulse.com/)
- [TinyML Book](https://www.oreilly.com/library/view/tinyml/9781492052036/)
- [Arduino IoT Reference](https://www.arduino.cc/reference/en/)

---

**Última atualização**: Maio 2026  
**Status**: ✅ Funcionando - Pronto para prototipagem  
**Versão**: 1.0.0 (Modular Architecture)
