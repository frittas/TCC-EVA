#ifndef TELEMETRY_SCHEDULER_H
#define TELEMETRY_SCHEDULER_H

#include <Arduino.h>
#include "storage_queue.h"

/**
 * Estados de conexão para máquina de estados
 */
enum ConnectionState
{
    CONN_OFFLINE = 0,  // Sem internet, salvar localmente
    CONN_ONLINE = 1,   // Conectado, pode enviar
    CONN_TRANSITIONING = 2 // Mudança de estado em progresso
};

/**
 * Estrutura para dados de telemetria antes de serialização
 */
typedef struct
{
    String predicaoIA;      // Diagnóstico da IA (ex: "Normal", "Anômalo")
    float confianca;        // Confiança 0.0-1.0
    float rms_triaxial;     // RMS Triaxial
    float *bufferX;         // Ponteiro para dados X (não armazenado)
    float *bufferY;         // Ponteiro para dados Y (não armazenado)
    float *bufferZ;         // Ponteiro para dados Z (não armazenado)
    int numAmostras;        // Número de amostras nos buffers
} TelemetryData;

// ============================================================================
// INTERFACE PÚBLICA
// ============================================================================

/**
 * Inicializa o scheduler de telemetria
 * Deve ser chamado no setup() após initStorageQueue()
 */
void initTelemetryScheduler();

/**
 * Processa ciclo de telemetria (chamar no loop principal)
 * Retorna true se alguma ação foi executada
 * Não bloqueia, usa millis() para temporização
 */
bool processTelemetrySchedule();

/**
 * Enfileira dados de telemetria para processamento
 * A decisão de enviar via MQTT ou salvar localmente é feita em processTelemetrySchedule()
 */
void queueTelemetryData(const TelemetryData &data);

/**
 * Atualiza o estado de conexão (chamado quando WiFi/MQTT muda)
 * state: 0=Offline, 1=Online, 2=Transitioning
 */
void updateConnectionState(uint8_t state);

/**
 * Retorna o estado de conexão atual
 */
ConnectionState getConnectionState();

/**
 * Retorna o número de telemetrias pendentes de processamento
 */
uint32_t getPendingTelemetryCount();

/**
 * Retorna o número de registros armazenados localmente
 */
uint32_t getStoredRecordsCount();

/**
 * Força processamento imediato (ignora intervalo 10s)
 * Útil para testes e situações críticas
 */
void forceTelemetryProcess();

/**
 * Obtém estatísticas de operação
 */
typedef struct
{
    uint32_t recordsProcessed;
    uint32_t recordsSentViaMQTT;
    uint32_t recordsStoredLocally;
    uint32_t queueFlushedCount;
    uint32_t lastProcessTime_ms;
} TelemetryStats;

TelemetryStats getTelemetryStats();

#endif // TELEMETRY_SCHEDULER_H
