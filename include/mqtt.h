#ifndef MQTT_H
#define MQTT_H

// Inicializa o módulo MQTT (WiFi e conexão)
void initMQTT();

// Mantém MQTT ativo e envia dados a cada 5 segundos
// Deve ser chamada no loop principal
void updateMQTT();

// Envia dados para o ThingsBoard (chamada internamente a cada 5s)
void enviarDadosParaNuvem(String predicaoIA, float confianca, float *bufferX, float *bufferY, float *bufferZ, int numAmostras);

// Reconecta ao ThingsBoard se desconectado
void reconectarMQTT();

// Retorna se o cliente MQTT está conectado
bool isMQTTConnected();

#endif // MQTT_H
