#include <PubSubClient.h>
#include <cmath>
#include "battery_management.h"
#include "wifi_management.h"
#include "telemetry_scheduler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

const char *TB_SERVER = "192.168.3.2"; // Ou o IP do seu servidor local

#ifndef MQTT_TOKEN
#error "MQTT_TOKEN não definido. Certifique-se de que o arquivo mqtt_token.txt existe na raiz do projeto."
#endif
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)
static const char *TOKEN = STRINGIFY(MQTT_TOKEN);

static WiFiClient espClient;
static PubSubClient mqttClient(espClient);

// Filas/Tarefas FreeRTOS para envio assíncrono
typedef struct
{
    char payload[512];
    unsigned long timestamp;
} TelemetryMessage;

static QueueHandle_t mqttQueue = NULL;
static const int MQTT_QUEUE_LENGTH = 8;

// Intervalos e reconexão
static const unsigned long MQTT_SEND_INTERVAL_MS = 1000;      // usado como timeout de dequeue
static const unsigned long MQTT_RECONNECT_INTERVAL_MS = 5000; // Tentar reconectar a cada 5 segundos
static unsigned long lastMQTTReconnectAttempt = 0;

// Tenta conectar ao MQTT, mas não bloqueia indefinidamente.
// Retorna true se conectado, false caso contrário.
bool conectarMQTT()
{
    if (mqttClient.connected())
        return true;

    if (!ensureWiFiConnected())
        return false;

    // No ThingsBoard, o "username" é o próprio Access Token e a senha fica em branco
    if (mqttClient.connect("ESP32S3_Sensor", TOKEN, NULL))
    {
        Serial.println("Conectado ao MQTT do ThingsBoard!");
        return true;
    }
    else
    {
        Serial.print("Falha na conexão MQTT. Erro: ");
        Serial.println(mqttClient.state());
        // Não há delay aqui para evitar bloqueio
        return false;
    }
}

// Inicializa o módulo MQTT
void initMQTT()
{
    mqttClient.setServer(TB_SERVER, 1883);

    // Create queue for telemetry messages
    if (mqttQueue == NULL)
    {
        mqttQueue = xQueueCreate(MQTT_QUEUE_LENGTH, sizeof(TelemetryMessage));
        if (mqttQueue == NULL)
        {
            Serial.println("[MQTT] Falha ao criar fila MQTT");
        }
    }

    // Create a FreeRTOS task to handle MQTT connection and publishing
    extern void mqttTask(void *param);
    xTaskCreatePinnedToCore(
        mqttTask,
        "MQTTTask",
        4096,
        NULL,
        1,
        NULL,
        1);

    Serial.println("MQTT inicializado (task assíncrona criada)");
}

// Tarefa FreeRTOS que gerencia conexão MQTT e publica mensagens da fila
void mqttTask(void *param)
{
    TelemetryMessage msg;
    for (;;)
    {
        // Ensure WiFi and MQTT connection
        if (!ensureWiFiConnected())
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        if (!mqttClient.connected())
        {
            unsigned long now = millis();
            if (now - lastMQTTReconnectAttempt >= MQTT_RECONNECT_INTERVAL_MS)
            {
                lastMQTTReconnectAttempt = now;
                Serial.println("Tentando reconectar ao ThingsBoard (task MQTT)...");
                conectarMQTT();
            }
        }

        mqttClient.loop();

        // Aguarda por uma mensagem para enviar (timeout para permitir loop e reconexão)
        if (mqttQueue != NULL)
        {
            if (xQueueReceive(mqttQueue, &msg, pdMS_TO_TICKS(MQTT_SEND_INTERVAL_MS)) == pdTRUE)
            {
                if (mqttClient.connected())
                {
                    if (mqttClient.publish("v1/devices/me/telemetry", msg.payload))
                    {
                        Serial.printf("[MQTT-Task] Telemetria enviada: %s\n", msg.payload);
                    }
                    else
                    {
                        Serial.println("[MQTT-Task] Erro ao publicar telemetria");
                    }
                }
                else
                {
                    Serial.println("[MQTT-Task] MQTT desconectado, descartando mensagem");
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Reconecta ao ThingsBoard se desconectado
void reconectarMQTT()
{
    // Mantido por compatibilidade, reconexão é gerenciada pela task MQTT
    if (!mqttClient.connected())
    {
        conectarMQTT();
    }
}

// Retorna se o cliente MQTT está conectado
bool isMQTTConnected()
{
    return mqttClient.connected();
}

/**
 * Publica dados de telemetria diretamente ao ThingsBoard
 * Usado pelo telemetry_scheduler para descarregar fila e enviar dados
 * Retorna true se sucesso, false caso contrário
 */
bool mqttPublishTelemetry(const String &jsonPayload)
{
    if (!mqttClient.connected())
    {
        return false;
    }

    if (mqttClient.publish("v1/devices/me/telemetry", jsonPayload.c_str()))
    {
        return true;
    }

    return false;
}

/**
 * Atualiza o estado de conexão no scheduler de telemetria
 * Chamado quando a conexão MQTT muda
 */
static void updateTelemetryConnectionState()
{
    // Estado 1: Online (conectado MQTT)
    // Estado 0: Offline (sem conexão MQTT)
    if (mqttClient.connected())
    {
        updateConnectionState(1); // CONN_ONLINE
    }
    else
    {
        updateConnectionState(0); // CONN_OFFLINE
    }
}

// Estrutura para armazenar resultados do cálculo RMS Triaxial
typedef struct
{
    float mediaX, mediaY, mediaZ; // Componentes DC (Gravidade)
    float rmsX, rmsY, rmsZ;       // RMS individual por eixo
    float valorRMS;               // RMS Triaxial Combinado
    String statusNorma;           // Classificação ISO 10816-3
} RMSTriaxialResult;

// Calcula o RMS Triaxial com remoção de componente DC (Gravidade)
// Retorna struct com médias DC, RMS individual e RMS triaxial combinado
RMSTriaxialResult calcularRMSTriaxial(float *bufferX, float *bufferY, float *bufferZ, int numAmostras)
{
    RMSTriaxialResult resultado;

    // 1. PRIMEIRA VARREDURA: Calcula as médias (DC offset / Gravidade) para cada eixo
    resultado.mediaX = 0.0f;
    resultado.mediaY = 0.0f;
    resultado.mediaZ = 0.0f;

    for (int i = 0; i < numAmostras; i++)
    {
        resultado.mediaX += bufferX[i];
        resultado.mediaY += bufferY[i];
        resultado.mediaZ += bufferZ[i];
    }
    resultado.mediaX /= numAmostras;
    resultado.mediaY /= numAmostras;
    resultado.mediaZ /= numAmostras;

    // 2. SEGUNDA VARREDURA: Calcula RMS dinâmico (AC) removendo o componente DC
    float somaX = 0.0f, somaY = 0.0f, somaZ = 0.0f;
    for (int i = 0; i < numAmostras; i++)
    {
        float acX = bufferX[i] - resultado.mediaX; // Remove componente DC do eixo X
        float acY = bufferY[i] - resultado.mediaY; // Remove componente DC do eixo Y
        float acZ = bufferZ[i] - resultado.mediaZ; // Remove componente DC do eixo Z

        somaX += acX * acX;
        somaY += acY * acY;
        somaZ += acZ * acZ;
    }
    resultado.rmsX = sqrt(somaX / numAmostras);
    resultado.rmsY = sqrt(somaY / numAmostras);
    resultado.rmsZ = sqrt(somaZ / numAmostras);

    // 3. VETOR RESULTANTE: Calcula RMS Triaxial Combinado
    // RMS_total = sqrt(RMS_X^2 + RMS_Y^2 + RMS_Z^2)
    resultado.valorRMS = sqrt(resultado.rmsX * resultado.rmsX +
                              resultado.rmsY * resultado.rmsY +
                              resultado.rmsZ * resultado.rmsZ);

    // 4. Classificação baseada na ISO 10816-3 (Classe II)
    if (resultado.valorRMS < 2.8)
    {
        resultado.statusNorma = "(Zonas A/B)";
    }
    else if (resultado.valorRMS >= 2.8 && resultado.valorRMS <= 7.1)
    {
        resultado.statusNorma = "(Zona C)";
    }
    else
    {
        resultado.statusNorma = "(Zona D)";
    }

    return resultado;
}

/**
 * Calcula apenas o valor RMS Triaxial (versão simplificada para o scheduler)
 * Retorna float com o valor RMS Triaxial combinado
 */
float calcularRMSTriaxialSimplificado(float *bufferX, float *bufferY, float *bufferZ, int numAmostras)
{
    RMSTriaxialResult resultado = calcularRMSTriaxial(bufferX, bufferY, bufferZ, numAmostras);
    return resultado.valorRMS;
}

// Compat: chamada no loop principal não precisa fazer nada
void updateMQTT()
{
    // A task MQTT executa loop() e publica mensagens; manter chamada para compatibilidade
}

// Esta função deve ser chamada logo após a inferência do TinyML
// Armazena os dados para envio periódico (não envia imediatamente)
void sendData(String predicaoIA, float confianca, float *bufferX, float *bufferY, float *bufferZ, int numAmostras)
{
    // Calcula RMS Triaxial (com remoção de componente DC)
    RMSTriaxialResult rmsResultado = calcularRMSTriaxial(bufferX, bufferY, bufferZ, numAmostras);
    // Monta payload JSON
    unsigned long currentMillis = millis();
    int rssi = getWiFiRSSI();
    int battery_level = getBatteryPercentage();

    String payloadStr = "{";
    payloadStr += "\"status_ia\":\"" + predicaoIA + "\",";
    payloadStr += "\"confianca\":" + String(confianca, 2) + ",";
    payloadStr += "\"rms_velocidade\":" + String(rmsResultado.valorRMS, 2) + ",";
    payloadStr += "\"norma_iso\":\"" + rmsResultado.statusNorma + "\",";
    payloadStr += "\"dispositivo\":\"EVA_01\",";
    payloadStr += "\"rssi\":" + String(rssi) + ",";
    payloadStr += "\"battery_level\":" + String(battery_level) + ",";
    payloadStr += "\"timestamp\":" + String(currentMillis);
    payloadStr += "}";

    // Enfileira para a task MQTT (não bloqueante)
    if (mqttQueue != NULL)
    {
        TelemetryMessage msg;
        memset(&msg, 0, sizeof(msg));
        payloadStr.toCharArray(msg.payload, sizeof(msg.payload));
        msg.timestamp = currentMillis;

        if (xQueueSend(mqttQueue, &msg, 0) != pdTRUE)
        {
            Serial.println("[MQTT] Fila cheia: mensagem descartada");
        }
    }

    // Serial.printf("[INFERÊNCIA] DC(X/Y/Z): %.2f/%.2f/%.2f | RMS(X/Y/Z): %.2f/%.2f/%.2f | RMS_Total: %.2f mm/s | Status: %s | Confiança: %.2f%%\n",
    //               rmsResultado.mediaX, rmsResultado.mediaY, rmsResultado.mediaZ,
    //               rmsResultado.rmsX, rmsResultado.rmsY, rmsResultado.rmsZ,
    //               rmsResultado.valorRMS, rmsResultado.statusNorma.c_str(), confianca);
}
