#include <Arduino.h>
#include <WiFi.h>
#include "wifi_management.h"

// Configurações da Rede
const char *WIFI_SSID = "CASA-515";
const char *WIFI_PASSWORD = "Lola@beringela1234";
const char *AP_SSID = "EVA_AP";
const char *AP_PASSWORD = "evabeta2026";

static const bool WIFI_FALLBACK_TO_AP = true;
static const unsigned long WIFI_STA_CONNECT_TIMEOUT_MS = 15000;

static AppWifiMode currentWifiMode = APP_WIFI_MODE_STA;

static bool conectarWiFiStation()
{
    if (WiFi.status() == WL_CONNECTED)
        return true;

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_STA_CONNECT_TIMEOUT_MS)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.printf("\nWiFi STA conectado: %s (IP: %s)\n", WIFI_SSID, WiFi.localIP().toString().c_str());
        return true;
    }

    Serial.println("\nFalha ao conectar em modo STA.");
    return false;
}

static void iniciarWiFiAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.printf("WiFi AP iniciado: %s (IP: %s)\n", AP_SSID, WiFi.softAPIP().toString().c_str());
}

void initWiFi()
{
    if (!conectarWiFiStation())
    {
        if (WIFI_FALLBACK_TO_AP)
        {
            currentWifiMode = APP_WIFI_MODE_AP;
            iniciarWiFiAP();
        }
    }
    else
    {
        currentWifiMode = APP_WIFI_MODE_STA;
    }
}

bool ensureWiFiConnected()
{
    if (currentWifiMode == APP_WIFI_MODE_STA)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            return conectarWiFiStation();
        }
        return true;
    }
    return true; // No modo AP está sempre disponível para rede local
}

bool isWiFiConnected()
{
    return (WiFi.status() == WL_CONNECTED) || (WiFi.getMode() & WIFI_MODE_AP);
}

AppWifiMode getCurrentWiFiMode()
{
    return currentWifiMode;
}

int getWiFiRSSI()
{
    return WiFi.RSSI();
}

String getWiFiIP()
{
    return (currentWifiMode == APP_WIFI_MODE_STA) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
}