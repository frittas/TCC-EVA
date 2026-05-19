#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "storage_queue.h"

// ============================================================================
// CONFIGURAÇÕES INTERNAS
// ============================================================================

static const char *LOG_FILE = "/telemetry_log.bin";
static const uint32_t MAX_QUEUE_SIZE = 5000;
static const uint32_t RECORD_SIZE = sizeof(TelemetryRecord);

// Estrutura de índice para otimizar acesso FIFO
typedef struct
{
    uint32_t headIndex;    // Índice do primeiro registro
    uint32_t tailIndex;    // Índice do próximo espaço livre
    uint32_t recordCount;  // Número de registros válidos
    uint8_t magic[4];      // Magic number para validação: "TLOG"
} QueueMetadata;

static QueueMetadata queueMeta = {0, 0, 0, {'T', 'L', 'O', 'G'}};
static const uint32_t METADATA_SIZE = sizeof(QueueMetadata);

// ============================================================================
// FUNÇÕES PRIVADAS
// ============================================================================

/**
 * Persiste os metadados no início do arquivo
 */
static bool saveMetadata()
{
    File file = LittleFS.open(LOG_FILE, "r+b");
    if (!file)
    {
        file = LittleFS.open(LOG_FILE, "wb");
        if (!file)
        {
            Serial.println("[StorageQueue] Erro: não conseguiu criar arquivo de log");
            return false;
        }
    }

    if (file.seek(0))
    {
        size_t written = file.write((const uint8_t *)&queueMeta, METADATA_SIZE);
        file.close();
        return written == METADATA_SIZE;
    }

    file.close();
    return false;
}

/**
 * Carrega metadados do arquivo
 */
static bool loadMetadata()
{
    File file = LittleFS.open(LOG_FILE, "rb");
    if (!file)
    {
        // Arquivo não existe, criar novo com metadados zerados
        queueMeta.headIndex = 0;
        queueMeta.tailIndex = 0;
        queueMeta.recordCount = 0;
        return saveMetadata();
    }

    if (file.seek(0))
    {
        size_t readBytes = file.readBytes((char *)&queueMeta, METADATA_SIZE);
        file.close();
        return readBytes == METADATA_SIZE;
    }

    file.close();
    return false;
}

/**
 * Obtém offset em bytes para um índice de registro
 */
static uint32_t getRecordOffset(uint32_t index)
{
    return METADATA_SIZE + (index % MAX_QUEUE_SIZE) * RECORD_SIZE;
}

/**
 * Lê um registro em posição específica
 */
static bool readRecordAt(uint32_t index, TelemetryRecord &outRecord)
{
    File file = LittleFS.open(LOG_FILE, "rb");
    if (!file)
        return false;

    uint32_t offset = getRecordOffset(index);
    if (!file.seek(offset))
    {
        file.close();
        return false;
    }

    size_t readBytes = file.readBytes((char *)&outRecord, RECORD_SIZE);
    file.close();
    return readBytes == RECORD_SIZE;
}

/**
 * Escreve um registro em posição específica
 */
static bool writeRecordAt(uint32_t index, const TelemetryRecord &record)
{
    File file = LittleFS.open(LOG_FILE, "r+b");
    if (!file)
    {
        file = LittleFS.open(LOG_FILE, "wb");
        if (!file)
            return false;
    }

    uint32_t offset = getRecordOffset(index);
    if (!file.seek(offset))
    {
        file.close();
        return false;
    }

    size_t written = file.write((const uint8_t *)&record, RECORD_SIZE);
    file.close();
    return written == RECORD_SIZE;
}

// ============================================================================
// INTERFACE PÚBLICA
// ============================================================================

bool initStorageQueue()
{
    if (!LittleFS.begin(true)) // true = format se necessário
    {
        Serial.println("[StorageQueue] Erro: não conseguiu inicializar LittleFS");
        return false;
    }

    Serial.println("[StorageQueue] LittleFS inicializado");

    // Carregar ou criar metadados
    if (!loadMetadata())
    {
        Serial.println("[StorageQueue] Erro ao carregar metadados");
        return false;
    }

    Serial.printf("[StorageQueue] Fila carregada: %u registros\n", queueMeta.recordCount);
    return true;
}

void shutdownStorageQueue()
{
    // Salvar metadados uma última vez
    saveMetadata();

    // Desmontar LittleFS
    LittleFS.end();
    Serial.println("[StorageQueue] Sistema de armazenamento desmontado");
}

bool enqueueRecord(const TelemetryRecord &record)
{
    // Se fila está cheia, remover o registro mais antigo (FIFO)
    if (queueMeta.recordCount >= MAX_QUEUE_SIZE)
    {
        queueMeta.headIndex = (queueMeta.headIndex + 1) % MAX_QUEUE_SIZE;
        queueMeta.recordCount--;
    }

    // Escrever novo registro na posição tail
    if (!writeRecordAt(queueMeta.tailIndex, record))
    {
        Serial.println("[StorageQueue] Erro ao escrever registro");
        return false;
    }

    // Atualizar índices
    queueMeta.tailIndex = (queueMeta.tailIndex + 1) % MAX_QUEUE_SIZE;
    queueMeta.recordCount++;

    // Persistir metadados
    if (!saveMetadata())
    {
        Serial.println("[StorageQueue] Erro ao salvar metadados");
        return false;
    }

    return true;
}

bool dequeueRecord(TelemetryRecord &outRecord)
{
    if (queueMeta.recordCount == 0)
        return false;

    // Ler registro na posição head
    if (!readRecordAt(queueMeta.headIndex, outRecord))
    {
        Serial.println("[StorageQueue] Erro ao ler registro");
        return false;
    }

    // Atualizar índices
    queueMeta.headIndex = (queueMeta.headIndex + 1) % MAX_QUEUE_SIZE;
    queueMeta.recordCount--;

    // Persistir metadados
    if (!saveMetadata())
    {
        Serial.println("[StorageQueue] Erro ao salvar metadados");
        return false;
    }

    return true;
}

uint32_t getQueueSize()
{
    return queueMeta.recordCount;
}

uint32_t getQueueCapacity()
{
    return MAX_QUEUE_SIZE;
}

bool isQueueEmpty()
{
    return queueMeta.recordCount == 0;
}

bool isQueueFull()
{
    return queueMeta.recordCount >= MAX_QUEUE_SIZE;
}

uint32_t getStorageUsageBytes()
{
    return METADATA_SIZE + (queueMeta.recordCount * RECORD_SIZE);
}

void clearQueue()
{
    queueMeta.headIndex = 0;
    queueMeta.tailIndex = 0;
    queueMeta.recordCount = 0;
    saveMetadata();

    LittleFS.remove(LOG_FILE);
    Serial.println("[StorageQueue] Fila limpa");
}

String recordToJSON(const TelemetryRecord &record)
{
    StaticJsonDocument<256> doc;

    doc["ts"] = record.timestamp;
    doc["rms"] = record.rms_triaxial;
    doc["conf"] = record.confianca;
    doc["diag"] = (int)record.diagnostico;
    doc["bat"] = (int)record.bateria_percent;
    doc["conn"] = (int)record.status_conexao;
    doc["rssi"] = (int)record.rssi;

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}
