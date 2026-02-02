#include "weather.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include "ui.h"

// Шрифт кастомный
#include "DIG15_AR36.h"
#define AR36 &ArialRoundedMTBold_36

// ===== Погода =====
float temperature = 0;
String description = "";
String icon = "/icon_app.bin";

// ===== Кэш UI =====
static String lastDescription = "";
static String lastIconPath = "";
static float lastTemp = -999;
bool iconLoaded = false;
uint16_t iconBuffer[ICON_SIZE * ICON_SIZE];

// Инициализация
void weatherInit() {
    LittleFS.begin();
}

// Получение пути иконки
String getWeatherIconPath(const String &iconUrl, const String &desc) {
    bool isNight = iconUrl.indexOf("/night/") >= 0;
    String d = desc; d.toLowerCase();

    if (d.indexOf("sunny") >= 0 || d.indexOf("clear") >= 0) return isNight ? "/icon_moon.bin" : "/icon_sun.bin";
    if (d.indexOf("partly cloudy") >= 0) return isNight ? "/icon_cloud_moon.bin" : "/icon_cloud_sun.bin";
    if (d.indexOf("cloud") >= 0) return "/icon_cloud.bin";
    if (d.indexOf("overcast") >= 0) return "/icon_cloud_s.bin";
    if (d.indexOf("rain") >= 0) return "/icon_rain_small.bin";
    if (d.indexOf("thunder") >= 0) return "/icon_thunder.bin";
    if (d.indexOf("snow") >= 0) return "/icon_snow.bin";
    if (d.indexOf("sleet") >= 0) return "/icon_sleet.bin";
    if (d.indexOf("mist") >= 0 || d.indexOf("fog") >= 0) {
        if (d.indexOf("freezing") >= 0) return "/icon_freezing_fog.bin";
        return "/icon_mist.bin";
    }
    if (d.indexOf("wind") >= 0) return "/icon_wind.bin";
    return "/icon_app.bin";
}

// Обновление данных погоды
void updateWeather() {
    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    String url = "http://api.weatherapi.com/v1/current.json?key=" + String(WEATHERAPI_KEY) +
                 "&q=" + String(OWM_CITY) + "&lang=en";
    http.begin(url);

    int code = http.GET();
    if (code == 200) {
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, http.getString());

        temperature = doc["current"]["temp_c"];
        description = doc["current"]["condition"]["text"].as<String>();
        String iconUrl = doc["current"]["condition"]["icon"].as<String>();
        icon = getWeatherIconPath(iconUrl, description);

        iconLoaded = false;
        lastDescription = "";
    }

    http.end();
}

// Загрузка иконки
void loadIconToBuffer() {
    if (icon == lastIconPath && iconLoaded) return;

    File f = LittleFS.open(icon, "r");
    if (!f) return;

    for (int y = 0; y < ICON_SIZE; y++) {
        f.read((uint8_t*)(iconBuffer + y * ICON_SIZE), ICON_SIZE * 2);
    }
    f.close();

    lastIconPath = icon;
    iconLoaded = true;
}

// Блок температуры + иконки 
void drawWeatherBlock(TFT_eSPI &tft) {
    static String lastIcon = "";
    bool needRedraw = false;

    if (temperature != lastTemp) { lastTemp = temperature; needRedraw = true; }
    if (icon != lastIcon) { lastIcon = icon; iconLoaded = false; needRedraw = true; }
    if (!needRedraw) return;

    tft.fillRect(UI_MARGIN_X, WX_Y + UI_SHIFT_Y, 320 - UI_MARGIN_X*2, WX_H, TFT_BLACK);

    tft.setFreeFont(AR36);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    String tempStr = String(temperature, 1) + "@C";
    tft.setCursor(UI_MARGIN_X+4, WX_Y + UI_SHIFT_Y + (WX_H + tft.fontHeight())/2 - UI_MARGIN_Y+10);
    tft.print(tempStr);

    loadIconToBuffer();
    tft.setSwapBytes(true);
    int iconX = 320 - UI_MARGIN_X - ICON_SIZE;
    int iconY = WX_Y + UI_SHIFT_Y + (WX_H - ICON_SIZE)/2+5;
    if (iconLoaded) tft.pushImage(iconX, iconY, ICON_SIZE, ICON_SIZE, iconBuffer);
}

// Описание погоды 
void drawWeatherDescription(TFT_eSPI &tft) {
    if (description == lastDescription) return;

    int descY = DESC_Y + UI_SHIFT_Y;
    tft.fillRect(UI_MARGIN_X, descY, 320 - UI_MARGIN_X*2, DESC_H, TFT_BLACK);

    tft.setTextFont(4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    int textW = tft.textWidth(description.c_str(), 4);
    tft.setCursor((320 - textW)/2, descY + (DESC_H - tft.fontHeight())/2+5);
    tft.print(description);

    lastDescription = description;
}

//  Функция обновления UI из loop()
void updateWeatherUI(TFT_eSPI &tft) {
    drawWeatherBlock(tft);
    drawWeatherDescription(tft);
}
