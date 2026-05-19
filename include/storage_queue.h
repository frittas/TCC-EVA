#ifndef STORAGE_QUEUE_H
#define STORAGE_QUEUE_H

#include <Arduino.h>

/**
 * Estrutura compacta para armazenar telemetria em Flash
 * Tamanho otimizado: ~64 bytes por registro
 */
typedef struct
{
    uint32_t timestamp;        // 4 bytes - timestamp em ms
    float rms_triaxial;        // 4 bytes - RMS Triaxial
    float confianca;           // 4 bytes - Confiança da IA
    int8_t diagnostico;        // 1 byte - Índice de diagnóstico (0-127)
    uint8_t bateria_percent;   // 1 byte - Percentual de bateria (0-100)
    uint8_t status_conexao;    // 1 byte - Status: 0=Offline, 1=Online, 2=Enviado
    int8_t rssi;               // 1 byte - WiFi RSSI (-127 a 0)
    uint8_t reserved[6];       // 6 bytes - Reservado para futuras expansões
    // Total: 22 bytes por registro
} TelemetryRecord;

// ============================================================================
// INTERFACE PÚBLICA
// ============================================================================

/**
 * Inicializa o sistema de armazenamento em Flash (LittleFS)
 * Deve ser chamado uma única vez no setup()
 * Retorna true se inicialização bem-sucedida
 */
bool initStorageQueue();

/**
 * Desmonta o sistema de arquivos LittleFS (segurança ao desligar)
 */
void shutdownStorageQueue();

/**
 * Enfileira um novo registro de telemetria na memória Flash
 * Se a fila atingir 5.000 registros, descarta o mais antigo (FIFO)
 * Retorna true se sucesso, false se erro de I/O
 */
bool enqueueRecord(const TelemetryRecord &record);

/**
 * Desenfileira o registro mais antigo (FIFO)
 * Retorna true se havia registro, false se fila vazia
 */
bool dequeueRecord(TelemetryRecord &outRecord);

/**
 * Retorna o número de registros armazenados atualmente
 */
uint32_t getQueueSize();

/**
 * Retorna a capacidade máxima da fila
 */
uint32_t getQueueCapacity();

/**
 * Retorna true se a fila está vazia
 */
bool isQueueEmpty();

/**
 * Retorna true se a fila está cheia (5.000 registros)
 */
bool isQueueFull();

/**
 * Retorna o uso de memória Flash em bytes
 */
uint32_t getStorageUsageBytes();

/**
 * Limpa completamente a fila (deleta arquivo de log)
 * Use com cuidado!
 */
void clearQueue();

/**
 * Retorna serialização JSON de um registro para envio MQTT
 */
String recordToJSON(const TelemetryRecord &record);

#endif // STORAGE_QUEUE_H
