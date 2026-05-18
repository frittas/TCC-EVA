#ifndef WIFI_MANAGEMENT_H
#define WIFI_MANAGEMENT_H

#include <WiFi.h>

enum AppWifiMode { APP_WIFI_MODE_STA, APP_WIFI_MODE_AP };

void initWiFi();
bool ensureWiFiConnected();
bool isWiFiConnected();
AppWifiMode getCurrentWiFiMode();
int getWiFiRSSI();
String getWiFiIP();

#endif
#ifndef WIFI_MANAGEMENT_H
#define WIFI_MANAGEMENT_H

#include <WiFi.h>

enum AppWifiMode { APP_WIFI_MODE_STA, APP_WIFI_MODE_AP };

void initWiFi();
bool ensureWiFiConnected();
bool isWiFiConnected();
AppWifiMode getCurrentWiFiMode();
int getWiFiRSSI();
String getWiFiIP();

#endif