#include "wifi_manager.h"

// ===== Настройки AP =====
ESP8266WebServer server(80);
const char* AP_NAME = "WeatherStation";

// ===== Состояние WiFi =====
bool wifiConnected = false;   // STA подключена
bool connecting = false;      // идёт попытка подключения STA
bool apModeActive = false;    // AP включён

unsigned long apStartTime = 0;
unsigned long connectStart = 0;

const unsigned long AP_TIMEOUT = 60000;      // AP работает максимум 1 мин
const unsigned long CONNECT_TIMEOUT = 30000; // Попытка подключения STA 30 сек

WiFiConfig wifiCfg;

// ---------------------------------------------------------------------
// Загрузка конфигурации WiFi
// ---------------------------------------------------------------------
bool loadWiFiConfig(WiFiConfig &cfg) {
    if (!LittleFS.begin()) return false;
    if (!LittleFS.exists("/wifi.json")) return false;

    File f = LittleFS.open("/wifi.json", "r");
    if (!f) return false;

    DynamicJsonDocument doc(256);
    if (deserializeJson(doc, f) != DeserializationError::Ok) {
        f.close();
        return false;
    }
    f.close();

    cfg.ssid = doc["ssid"].as<String>();
    cfg.pass = doc["pass"].as<String>();
    return true;
}

// ---------------------------------------------------------------------
// Сохранение конфигурации WiFi
// ---------------------------------------------------------------------
void saveWiFiConfig(const WiFiConfig &cfg) {
    DynamicJsonDocument doc(256);
    doc["ssid"] = cfg.ssid;
    doc["pass"] = cfg.pass;

    File f = LittleFS.open("/wifi.json", "w");
    if (!f) return;
    serializeJson(doc, f);
    f.close();
}

// ---------------------------------------------------------------------
// Веб-страница для настройки WiFi
// ---------------------------------------------------------------------
void handleRoot() {
    int n = WiFi.scanNetworks();
    String page = "<!DOCTYPE html><html><body>";
    page += "<h2>WiFi Setup</h2>";
    page += "<form method='POST' action='/save'>";
    page += "Select WiFi: <select name='ssid'>";
    for (int i = 0; i < n; i++) {
        page += "<option>" + WiFi.SSID(i) + "</option>";
    }
    page += "</select><br>";
    page += "Password: <input name='pass' type='password'><br>";
    page += "<input type='submit' value='Connect'></form></body></html>";
    server.send(200, "text/html", page);
}

// ---------------------------------------------------------------------
// Сохранение WiFi после ручного подключения
// ---------------------------------------------------------------------
void handleSave() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
        wifiCfg.ssid = server.arg("ssid");
        wifiCfg.pass = server.arg("pass");

        Serial.printf("Trying to connect to WiFi: %s\n", wifiCfg.ssid.c_str());

        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiCfg.ssid.c_str(), wifiCfg.pass.c_str());

        connecting = true;
        connectStart = millis();

        apModeActive = false; // сразу отключаем AP, если был
        server.send(200, "text/html", "<h3>Trying to connect. Wait...</h3>");
    } else {
        server.send(400, "text/plain", "Missing parameters");
    }
}

// ---------------------------------------------------------------------
// Запуск AP
// ---------------------------------------------------------------------
void startAP() {
    if (!apModeActive) {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(AP_NAME);

        static bool handlersRegistered = false;
        if (!handlersRegistered) {
            server.on("/", handleRoot);
            server.on("/save", HTTP_POST, handleSave);
            handlersRegistered = true;
        }
        server.begin();
        apStartTime = millis();
        apModeActive = true;
        Serial.println("AP started at 192.168.4.1 for configuration");
    }
}

// ---------------------------------------------------------------------
// Инициализация WiFi при старте
// ---------------------------------------------------------------------
void startWiFi() {
    if (loadWiFiConfig(wifiCfg)) {
        Serial.printf("Trying last saved WiFi: %s\n", wifiCfg.ssid.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiCfg.ssid.c_str(), wifiCfg.pass.c_str());
    } else {
        Serial.println("No saved WiFi, starting AP...");
        startAP();
    }
}

// ---------------------------------------------------------------------
// Основная неблокирующая проверка WiFi и AP
// ---------------------------------------------------------------------
void ensureWiFiConnected() {
    server.handleClient(); // обрабатываем веб-запросы
    unsigned long now = millis();

    // ===== Подключение STA через веб =====
    if (connecting) {
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            connecting = false;
            apModeActive = false;
            Serial.println("WiFi connected (manual)!");
            saveWiFiConfig(wifiCfg);
        } else if (now - connectStart > CONNECT_TIMEOUT) {
            connecting = false;
            Serial.println("WiFi connect failed, reverting to AP");
            startAP();
        }
        return;
    }

    // ===== Автоподключение после перезагрузки =====
    if (WiFi.status() == WL_CONNECTED) {
        if (!wifiConnected) {
            wifiConnected = true;
            Serial.println("WiFi connected (auto)!");
        }
        return;
    } else {
        wifiConnected = false;
    }

    // ===== Включаем AP, если нет сети =====
    if (!apModeActive) startAP();

    // ===== Таймаут AP =====
    if (apModeActive && now - apStartTime > AP_TIMEOUT) {
        Serial.println("AP timeout reached, retrying last WiFi...");
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        apModeActive = false;
        if (wifiCfg.ssid.length() > 0) {
            WiFi.begin(wifiCfg.ssid.c_str(), wifiCfg.pass.c_str());
            connecting = true;
            connectStart = millis();
        }
    }
}
