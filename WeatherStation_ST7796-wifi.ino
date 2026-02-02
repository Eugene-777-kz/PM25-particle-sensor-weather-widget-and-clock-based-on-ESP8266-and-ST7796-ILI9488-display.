#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TFT_eSPI.h>

#include "config.h"
#include "wifi_manager.h"
#include "pms.h"
#include "weather.h"
#include "ui.h"

static unsigned long lastMillis = 0;
static unsigned long localSeconds = 0;

TFT_eSPI tft;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", TIME_OFFSET);

void setup() {
    Serial.begin(115200);

    // Инициализация экрана
    tft.init();
    //tft.invertDisplay(true); // ВКЛЮЧАЕМ инверсию цветов для ILI9488!!!!
    tft.setRotation(2); // 2 для ST7796 0 для ILI9488 
    tft.fillScreen(TFT_BLACK);

    // WiFi
    startWiFi();  

    // PMS
    pmsInit();

    // Погода
    weatherInit();

    // NTP
    timeClient.begin();
    if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    localSeconds = timeClient.getEpochTime();
} else {
    localSeconds = 0; // стартуем с 0, или можно установить вручную
}
    lastMillis = millis(); 

    // Рисуем статические блоки UI
    drawLayout(tft);

    // Если WiFi уже подключён, сразу обновляем погоду
    if (WiFi.status() == WL_CONNECTED) {
        updateWeather();
    }
}

void loop() {
    unsigned long now = millis();

    // Проверка WiFi и обработка AP каждый проход loop()
    ensureWiFiConnected();

    // ---------- Отслеживание появления WiFi ----------
    static bool wasWiFiConnected = false;
    if (WiFi.status() == WL_CONNECTED) {
        if (!wasWiFiConnected) {
            // WiFi ТОЛЬКО ЧТО появился
            updateWeather();    // сразу обновляем погоду
            timeClient.update(); // и время
        }
        wasWiFiConnected = true;
    } else {
        wasWiFiConnected = false;
    }

    // ---------- Обновление времени ТОЛЬКО при наличии WiFi раз в час ----------
    static unsigned long lastNtpSync = 0;
    if (WiFi.status() == WL_CONNECTED && now - lastNtpSync > 3600000) {
        timeClient.update();
        lastNtpSync = now;
    }

    // ---------- Обновление погоды каждые 10 минут ----------
    static unsigned long lastWeatherUpdate = 0;
    if (WiFi.status() == WL_CONNECTED && now - lastWeatherUpdate > 600000) {
        updateWeather();
        lastWeatherUpdate = now;
    }

    // ---------- Обновление блоков раз в секунду ----------
    static unsigned long lastTick = 0;
    if (now - lastTick > 1000) {
        lastTick = now;

        drawTime(tft, timeClient);
        drawDate(tft, timeClient);
        updateWeatherUI(tft);
        readPMS();
        drawPM(tft);
        drawStatus(tft);
    }
}
