#include <Arduino.h>
#include <queue>
#include "config.h"
#include "telemetry_scheduler.h"
#include "mqtt.h"
#include "battery_management.h"
#include "wifi_management.h"

// ============================================================================
// CONFIGURAÇÕES INTERNAS
// ============================================================================

static const uint8_t DIAGNOSTICO_NORMAL = 0;
static const uint8_t DIAGNOSTICO_ANOMALO = 1;

// ============================================================================
// VARIÁVEIS ESTÁTICAS
// ============================================================================

static ConnectionState currentConnectionState = CONN_OFFLINE;
static unsigned long lastTelemetryTime = 0;
static unsigned long lastQueueFlushTime = 0;
static std::queue<TelemetryData> pendingTelemetry;

// Estatísticas
static TelemetryStats stats = {0, 0, 0, 0, 0};

// Flag para forçar processamento
static bool forceProcessFlag = false;

// ============================================================================
// FUNÇÕES PRIVADAS
// ============================================================================

/**
 * Mapeia string de diagnóstico para índice (otimização de espaço)
 */
static uint8_t mapDiagnosticoToIndex(const String &predicao)
{
    if (predicao.equalsIgnoreCase("Normal") || predicao.equalsIgnoreCase("OK"))
    {
        return DIAGNOSTICO_NORMAL;
    }
    return DIAGNOSTICO_ANOMALO;
}

/**
 * Mapeia índice de diagnóstico para string
 */
static String mapDiagnosticoFromIndex(uint8_t index)
{
    return (index == DIAGNOSTICO_NORMAL) ? "Normal" : "Anomalo";
}

/**
 * Cria um registro de telemetria compacto
 */
static TelemetryRecord createRecord(const TelemetryData &data)
{
    TelemetryRecord record;
    record.timestamp = millis();
    record.rms_triaxial = data.rms_triaxial;
    record.confianca = data.confianca;
    record.diagnostico = mapDiagnosticoToIndex(data.predicaoIA);
    record.bateria_percent = getBatteryPercentage();
    record.status_conexao = (uint8_t)currentConnectionState;
    record.rssi = (isWiFiConnected()) ? getWiFiRSSI() : -127;

    return record;
}

/**
 * Processa modo ONLINE: Tenta enviar via MQTT + descarrega fila local
 */
static bool processOnlineMode()
{
    // 1. Verificar se há dados pendentes de envio via MQTT
    if (!pendingTelemetry.empty() && isMQTTConnected())
    {
        TelemetryData data = pendingTelemetry.front();
        TelemetryRecord record = createRecord(data);

        // Enviar via MQTT
        String json = recordToJSON(record);
        if (mqttPublishTelemetry(json))
        {
            pendingTelemetry.pop();
            stats.recordsSentViaMQTT++;
            Serial.printf("[TelemetryScheduler] MQTT enviado: %s\n", json.c_str());
        }
        else
        {
            Serial.println("[TelemetryScheduler] Erro ao enviar MQTT, salvando localmente");
            // Salvar localmente como backup
            if (enqueueRecord(record))
            {
                pendingTelemetry.pop();
                stats.recordsStoredLocally++;
            }
        }
    }

    // 2. Descarregar registros antigos da fila local (se houver)
    if (!isQueueEmpty() && (millis() - lastQueueFlushTime >= QUEUE_FLUSH_TIMEOUT_MS))
    {
        TelemetryRecord oldRecord;
        if (dequeueRecord(oldRecord))
        {
            if (isMQTTConnected())
            {
                String json = recordToJSON(oldRecord);
                if (mqttPublishTelemetry(json))
                {
                    stats.recordsSentViaMQTT++;
                    Serial.printf("[TelemetryScheduler] Fila descarregada via MQTT: %s\n", json.c_str());
                }
                else
                {
                    // Recoloca na fila se falhar
                    enqueueRecord(oldRecord);
                    Serial.println("[TelemetryScheduler] Falha ao enviar fila, mantendo armazenado");
                }
            }
            lastQueueFlushTime = millis();
        }
    }

    return true;
}

/**
 * Processa modo OFFLINE: Salva tudo localmente
 */
static bool processOfflineMode()
{
    if (!pendingTelemetry.empty())
    {
        TelemetryData data = pendingTelemetry.front();
        TelemetryRecord record = createRecord(data);

        if (enqueueRecord(record))
        {
            pendingTelemetry.pop();
            stats.recordsStoredLocally++;
            Serial.printf("[TelemetryScheduler] Registro armazenado localmente (Total: %u)\n",
                          getQueueSize());
        }
        else
        {
            Serial.println("[TelemetryScheduler] Erro ao armazenar registro localmente");
        }
    }

    return false;
}

// ============================================================================
// INTERFACE PÚBLICA
// ============================================================================

void initTelemetryScheduler()
{
    currentConnectionState = CONN_OFFLINE;
    lastTelemetryTime = millis();
    lastQueueFlushTime = millis();

    // Limpar queue
    while (!pendingTelemetry.empty())
    {
        pendingTelemetry.pop();
    }

    // Zerar estatísticas
    stats.recordsProcessed = 0;
    stats.recordsSentViaMQTT = 0;
    stats.recordsStoredLocally = 0;
    stats.queueFlushedCount = 0;

    Serial.println("[TelemetryScheduler] Inicializado");
}

bool processTelemetrySchedule()
{
    unsigned long now = millis();

    // Verificar se intervalo foi atingido
    bool timeoutReached = (now - lastTelemetryTime >= TELEMETRY_INTERVAL_MS);

    if (!timeoutReached && !forceProcessFlag)
        return false;

    // Resetar flag
    forceProcessFlag = false;
    lastTelemetryTime = now;
    stats.lastProcessTime_ms = now;

    // Se nada para processar, retornar
    if (pendingTelemetry.empty() && isQueueEmpty())
        return false;

    // Executar lógica baseada em estado de conexão
    bool didWork = false;

    if (currentConnectionState == CONN_ONLINE)
    {
        didWork = processOnlineMode();
    }
    else if (currentConnectionState == CONN_OFFLINE)
    {
        didWork = processOfflineMode();
    }

    stats.recordsProcessed++;
    return didWork;
}

void queueTelemetryData(const TelemetryData &data)
{
    // Criar cópia dos dados (sem buffers de ponteiro)
    TelemetryData dataCopy = data;
    dataCopy.bufferX = nullptr;
    dataCopy.bufferY = nullptr;
    dataCopy.bufferZ = nullptr;

    pendingTelemetry.push(dataCopy);

    Serial.printf("[TelemetryScheduler] Dados enfileirados (Pending: %u)\n",
                  (uint32_t)pendingTelemetry.size());
}

void updateConnectionState(uint8_t state)
{
    ConnectionState newState = (ConnectionState)state;

    if (newState != currentConnectionState)
    {
        Serial.printf("[TelemetryScheduler] Estado de conexão: %d -> %d\n",
                      currentConnectionState, newState);
        currentConnectionState = newState;

        // Se conectou, forçar descarregamento de fila
        if (newState == CONN_ONLINE && !isQueueEmpty())
        {
            Serial.printf("[TelemetryScheduler] Transição para ONLINE com %u registros em fila\n",
                          getQueueSize());
            forceTelemetryProcess();
        }
    }
}

ConnectionState getConnectionState()
{
    return currentConnectionState;
}

uint32_t getPendingTelemetryCount()
{
    return (uint32_t)pendingTelemetry.size();
}

uint32_t getStoredRecordsCount()
{
    return getQueueSize();
}

void forceTelemetryProcess()
{
    forceProcessFlag = true;
}

TelemetryStats getTelemetryStats()
{
    return stats;
}
