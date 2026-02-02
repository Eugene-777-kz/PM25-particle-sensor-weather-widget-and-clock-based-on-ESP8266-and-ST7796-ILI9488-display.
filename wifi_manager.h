#pragma once
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// Структура для хранения WiFi данных
struct WiFiConfig {
    String ssid;
    String pass;
};

// Глобальный веб-сервер для режима AP
extern ESP8266WebServer server;

// Прототипы функций
bool loadWiFiConfig(WiFiConfig &cfg);
void saveWiFiConfig(const WiFiConfig &cfg);

void startWiFi();
void ensureWiFiConnected();
void startAP();
