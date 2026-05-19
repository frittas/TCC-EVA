# EVA - Eletromecânica, Vibração e Análise

O projeto **EVA** é um sistema embarcado baseado em ESP32-S3 projetado para o monitoramento de ativos industriais através da análise de vibração triaxial. O sistema utiliza técnicas de processamento de sinal (FFT e RMS) e está preparado para integração com modelos de TinyML para detecção precoce de falhas.

## 🚀 Quickstart

### Pré-requisitos
* [PlatformIO IDE](https://platformio.org/) (extensão para VS Code).
* Placa ESP32-S3 (configurada no `platformio.ini`).

### Configuração Inicial
1.  **Token de Acesso:** Na raiz do projeto, crie um arquivo chamado `mqtt_token.txt`.
2.  Insira o seu Access Token do ThingsBoard dentro deste arquivo (sem espaços ou aspas). O script `load_mqtt_token.py` carregará isso automaticamente durante o build.
3.  **Conectividade:** Ajuste o `WIFI_SSID` e `WIFI_PASSWORD` no arquivo `src/wifi_management.cpp`.
4.  **Servidor MQTT:** Altere o `TB_SERVER` em `src/mqtt.cpp` para o IP do seu servidor ThingsBoard.

### Compilação e Upload
```bash
# Pelo terminal do PlatformIO
pio run --target upload
pio device monitor
```

## 🏗️ Estrutura do Projeto

```text
TCC-EVA/
├── include/                # Cabeçalhos (.h) de todos os módulos
├── src/                    # Implementações (.cpp)
│   ├── main.cpp            # Loop principal e máquina de estados da UI
│   ├── telemetry_scheduler # Inteligência de envio (MQTT vs Storage)
│   ├── storage_queue       # Driver de persistência em LittleFS (Flash)
│   ├── mqtt                # Task assíncrona para comunicação ThingsBoard
│   └── sensor_mpu6050      # Aquisição de dados a 1kHz
├── load_mqtt_token.py      # Script de automação de build para segurança
├── platformio.ini          # Configuração de ambiente e dependências
└── mqtt_token.txt          # Arquivo local (não versionado) com credenciais
```

## ✨ Features Principais

*   **Amostragem de Alta Performance:** Captura triaxial a 1kHz utilizando buffers circulares.
*   **Resiliência de Dados:** Caso o Wi-Fi ou MQTT caia, o sistema armazena automaticamente até 5.000 registros na memória Flash (LittleFS) e os descarrega quando a conexão é restabelecida.
*   **Processamento de Sinal On-device:**
    *   Cálculo de **RMS Triaxial** com remoção de componente DC (gravidade).
    *   Análise espectral via **FFT** (Fast Fourier Transform) em tempo real.
*   **Interface Rica:** Display OLED SSD1306 com menus de navegação, visualização de formas de onda e estado da bateria.
*   **Gestão de Energia:** Suporte a Deep Sleep com ativação via interrupção externa (botão) e monitoramento de bateria.
*   **Comunicação Assíncrona:** Uso de tasks FreeRTOS para garantir que o envio MQTT não bloqueie a captura de sensores.

## 🛠️ Arquitetura de Software

O sistema foi desenhado seguindo o princípio de modularidade:

1.  **Camada de Aquisição:** O `sensor_mpu6050` alimenta buffers de 1024 amostras.
2.  **Camada de Processamento:** `fft_analysis` e a lógica de RMS transformam dados brutos em métricas de diagnóstico.
3.  **Scheduler de Telemetria:** Atua como um orquestrador. Ele decide se o registro deve ir para a fila do `mqtt_task` (se online) ou ser serializado e salvo no `storage_queue` (se offline).
4.  **Interface Humano-Máquina (IHM):** Uma máquina de estados no `main.cpp` gerencia menus e modos de operação (Monitoramento vs. Coleta Bruta).

## 📚 Dependências

As principais bibliotecas utilizadas são:

*   **Adafruit MPU6050 & Unified Sensor:** Gestão do acelerômetro.
*   **Adafruit SSD1306 & GFX:** Renderização de gráficos no OLED.
*   **ArduinoJson:** Serialização para o protocolo ThingsBoard.
*   **arduinoFFT:** Processamento de sinais no domínio da frequência.
*   **PubSubClient:** Cliente MQTT leve.

## ⚙️ Configurações Técnicas (`config.h`)

| Parâmetro | Valor | Descrição |
| :--- | :--- | :--- |
| `SAMPLE_INTERVAL` | 1000us | Frequência de amostragem (1kHz) |
| `WAVE_BUFFER_SIZE` | 1024 | Tamanho do buffer de sinal |
| `TELEMETRY_INTERVAL_MS` | 10000ms | Intervalo de envio de telemetria |
| `LONG_PRESS_MS` | 3000ms | Tempo para entrar em Deep Sleep |
| `MAX_QUEUE_SIZE` | 5000 | Limite de registros em Flash |

---
*Projeto desenvolvido como parte do Trabalho de Conclusão de Curso (TCC).*