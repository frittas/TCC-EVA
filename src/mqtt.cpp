#include <PubSubClient.h>
#include <cmath>
#include "battery_management.h"
#include "wifi_management.h"

const char *TB_SERVER = "192.168.3.2"; // Ou o IP do seu servidor local

#ifndef MQTT_TOKEN
#error "MQTT_TOKEN não definido. Certifique-se de que o arquivo mqtt_token.txt existe na raiz do projeto."
#endif
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)
static const char *TOKEN = STRINGIFY(MQTT_TOKEN);

static WiFiClient espClient;
static PubSubClient mqttClient(espClient);

// Variáveis de controle de timing - Envio a cada 1 segundo
static unsigned long lastMQTTSendTime = 0;
static const unsigned long MQTT_SEND_INTERVAL_MS = 1000; // 1 segundos

// Estrutura para armazenar resultados do cálculo RMS Triaxial
struct RMSTriaxialResult {
    float mediaX, mediaY, mediaZ;      // Componentes DC (Gravidade)
    float rmsX, rmsY, rmsZ;            // RMS individual por eixo
    float valorRMS;                     // RMS Triaxial Combinado
    String statusNorma;                // Classificação ISO 10816-3
};

// Buffer para armazenar últimos dados - Persistência entre ciclos
static String lastPredictedStatus = "MONITORANDO";
static float lastConfidence = 0.0f;
static float lastRMSValue = 0.0f;
static String lastNormaStatus = "Saudável (Zonas A/B)";
static bool hasPendingData = false;

// Variáveis de controle de timing para reconexão MQTT
static unsigned long lastMQTTReconnectAttempt = 0;
static const unsigned long MQTT_RECONNECT_INTERVAL_MS = 5000; // Tentar reconectar a cada 5 segundos

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
    // Tenta conectar ao MQTT, mas não bloqueia o setup se falhar
    bool connected = conectarMQTT();
    lastMQTTSendTime = millis();
    if (connected)
    {
        Serial.println("MQTT inicializado com sucesso!");
    }
    else
    {
        Serial.println("MQTT inicializado sem conexao. A reconexao sera tentada no loop.");
    }
}

// Reconecta ao ThingsBoard se desconectado
void reconectarMQTT()
{
    if (!mqttClient.connected()) // Se não estiver conectado
    {
        unsigned long currentMillis = millis();
        // Tenta reconectar apenas após o intervalo definido
        if (currentMillis - lastMQTTReconnectAttempt >= MQTT_RECONNECT_INTERVAL_MS)
        {
            lastMQTTReconnectAttempt = currentMillis; // Atualiza o tempo da última tentativa
            Serial.println("Tentando reconectar ao ThingsBoard...");
            if (!conectarMQTT()) // Tenta conectar uma vez
            {
                Serial.println("Reconexão MQTT falhou; tentaremos novamente mais tarde.");
            }
        }
    }
}

// Retorna se o cliente MQTT está conectado
bool isMQTTConnected()
{
    return mqttClient.connected();
}

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
        float acX = bufferX[i] - resultado.mediaX;  // Remove componente DC do eixo X
        float acY = bufferY[i] - resultado.mediaY;  // Remove componente DC do eixo Y
        float acZ = bufferZ[i] - resultado.mediaZ;  // Remove componente DC do eixo Z
        
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

// Atualiza estado MQTT e envia dados a cada 5 segundos
void updateMQTT()
{
    unsigned long currentMillis = millis();
    
    // Mantém a conexão MQTT ativa
    if (!mqttClient.connected())
    {
        reconectarMQTT();
    }
    
    mqttClient.loop(); // Processa mensagens MQTT

    // Envia dados a cada 5 segundos
    if (currentMillis - lastMQTTSendTime >= MQTT_SEND_INTERVAL_MS)
    {
        lastMQTTSendTime = currentMillis;
        
        if (hasPendingData)
        {
            // Calcula RMS a partir dos dados armazenados
            int rssi = getWiFiRSSI();
            int battery_level = getBatteryPercentage();

            String payloadStr = "{";
            payloadStr += "\"status_ia\":\"" + lastPredictedStatus + "\",";
            payloadStr += "\"confianca\":" + String(lastConfidence, 2) + ",";
            payloadStr += "\"rms_velocidade\":" + String(lastRMSValue, 2) + ",";
            payloadStr += "\"norma_iso\":\"" + lastNormaStatus + "\",";
            payloadStr += "\"dispositivo\":\"EVA_01\",";
            payloadStr += "\"rssi\":" + String(rssi) + ",";
            payloadStr += "\"battery_level\":" + String(battery_level) + ",";
            payloadStr += "\"timestamp\":" + String(currentMillis);
            payloadStr += "}";

            // Converte String para char* para publicação MQTT
            char payload[512];
            payloadStr.toCharArray(payload, sizeof(payload));

            // Publica no tópico padrão de telemetria do ThingsBoard
            if (mqttClient.publish("v1/devices/me/telemetry", payload))
            {
                Serial.printf("[MQTT] Telemetria enviada: %s\n", lastPredictedStatus.c_str());
            }
            else
            {
                Serial.println("[MQTT] Erro ao enviar telemetria!");
            }
            
            hasPendingData = false; // Limpa flag após envio
        }
    }
}

// Esta função deve ser chamada logo após a inferência do TinyML
// Armazena os dados para envio periódico (não envia imediatamente)
void sendData(String predicaoIA, float confianca, float *bufferX, float *bufferY, float *bufferZ, int numAmostras)
{
    // Calcula RMS Triaxial (com remoção de componente DC)
    RMSTriaxialResult rmsResultado = calcularRMSTriaxial(bufferX, bufferY, bufferZ, numAmostras);

    // Armazena dados para envio periódico (SEPARAÇÃO CLARA: inferência ≠ envio)
    lastPredictedStatus = predicaoIA;
    lastConfidence = confianca;
    lastRMSValue = rmsResultado.valorRMS;
    lastNormaStatus = rmsResultado.statusNorma;
    hasPendingData = true; // Marca como pendente para envio no próximo ciclo

    Serial.printf("[INFERÊNCIA] DC(X/Y/Z): %.2f/%.2f/%.2f | RMS(X/Y/Z): %.2f/%.2f/%.2f | RMS_Total: %.2f mm/s | Status: %s | Confiança: %.2f%%\n",
                  rmsResultado.mediaX, rmsResultado.mediaY, rmsResultado.mediaZ, 
                  rmsResultado.rmsX, rmsResultado.rmsY, rmsResultado.rmsZ, 
                  rmsResultado.valorRMS, rmsResultado.statusNorma.c_str(), confianca);
}
