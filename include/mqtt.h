#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>

// Inicializa o módulo MQTT (cria tarefa e fila)
void initMQTT();

// Compatibilidade: chamada no loop principal é agora não bloqueante/sem efeito
void updateMQTT();

// Enfileira dados de telemetria para envio assíncrono (não bloqueante)
void sendData(String predicaoIA, float confianca, float *bufferX, float *bufferY, float *bufferZ, int numAmostras);

// Retorna se o cliente MQTT está conectado
bool isMQTTConnected();

/**
 * Publica payload JSON diretamente ao ThingsBoard
 * Usado por telemetry_scheduler para envio de dados
 * Retorna true se sucesso
 */
bool mqttPublishTelemetry(const String &jsonPayload);

/**
 * Calcula RMS Triaxial simplificado para armazenamento em Flash
 * Retorna apenas o valor RMS combinado (não struct completa)
 */
float calcularRMSTriaxialSimplificado(float *bufferX, float *bufferY, float *bufferZ, int numAmostras);

#endif // MQTT_H
