#include "ui.h"
#include "pms.h"
#include "weather.h"
#include <ESP8266WiFi.h>
#include <TFT_eSPI.h>

// смещение блоков
const int UI_SHIFT_Y = 5;

// ---------------------------------------------------------------------
// Цвет PM в зависимости от значения
// ---------------------------------------------------------------------
uint16_t pmColor(uint16_t v) {
    if (v <= 12) return TFT_GREEN;
    if (v <= 35) return TFT_YELLOW;
    if (v <= 55) return TFT_ORANGE;
    if (v <= 150) return TFT_RED;
    return TFT_PURPLE;
}

// ---------------------------------------------------------------------
// Фоновые блоки
// ---------------------------------------------------------------------
void drawLayout(TFT_eSPI &tft) {
    tft.fillRect(0, 0, 320, 60, TFT_DARKGREY);     // Часы
    tft.fillRect(0, WX_Y, 320, WX_H, TFT_BLACK);   // Температура + иконка
    tft.fillRect(0, DESC_Y, 320, DESC_H, TFT_BLACK);// Описание погоды
    tft.fillRect(0, DATE_Y, 320, DATE_H, TFT_BLACK);// Дата
    tft.fillRect(0, 230, 320, 230, TFT_BLACK);     // PM
    tft.fillRect(0, 460, 320, 20, TFT_NAVY);       // Нижняя панель WiFi
}

// ---------------------------------------------------------------------
// Часы
// ---------------------------------------------------------------------
void drawTime(TFT_eSPI &tft, NTPClient &timeClient) {
    String now = timeClient.getFormattedTime();
    tft.setTextFont(6);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    int charWidth = 26;
    int x = (320 - now.length() * charWidth) / 2;
    tft.setCursor(x, 12);
    tft.print(now);
}

// ---------------------------------------------------------------------
// Дата
// ---------------------------------------------------------------------
void drawDate(TFT_eSPI &tft, NTPClient &timeClient) {
    time_t rawTime = timeClient.getEpochTime();
    struct tm *timeInfo = localtime(&rawTime);

    static const char* WEEKDAYS[] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };

    char dateBuf[32];
    snprintf(dateBuf, sizeof(dateBuf),
             "%s  %02d.%02d.%04d",
             WEEKDAYS[timeInfo->tm_wday],
             timeInfo->tm_mday,
             timeInfo->tm_mon + 1,
             timeInfo->tm_year + 1900);

    String dateStr = dateBuf;

    static String lastDate = "";

    if (dateStr == lastDate) return;

    lastDate = dateStr;

    int blockY = DATE_Y + UI_SHIFT_Y;

    tft.fillRect(0, blockY, 320, DATE_H, TFT_BLACK);

    tft.setTextFont(4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    int textW = tft.textWidth(dateStr.c_str(), 4);
    int x = (320 - textW) / 2;
    int y = blockY + (DATE_H - tft.fontHeight()) / 2;

    tft.setCursor(x, y);
    tft.print(dateStr);
}

// ---------------------------------------------------------------------
// Блок PM с динамическим цветом и адаптивной заливкой числа
// ---------------------------------------------------------------------
void drawPM(TFT_eSPI &tft) {
    static int lastPM1  = -1;
    static int lastPM25 = -1;
    static int lastPM10 = -1;

    static uint16_t lastColorPM1  = 0xFFFF;
    static uint16_t lastColorPM25 = 0xFFFF;
    static uint16_t lastColorPM10 = 0xFFFF;

    static bool firstDraw = true;

    tft.setFreeFont(&FreeSansBoldOblique18pt7b);

    const int pmY   = 290;
    const int pmGap = 60;
    const int lineH = tft.fontHeight() + 8;

    const int xText = 10;
    const int numX  = 120;
    const int unitX = 210;

    const int clearW = 320;   // очищаем всю строку

    char buf[5];

    // ===================== PM1.0 =====================
    uint16_t colorPM1 = pmColor(pm1);

    if (firstDraw || colorPM1 != lastColorPM1) {
        tft.fillRect(0, pmY - lineH + 5, clearW, lineH, TFT_BLACK);

        tft.setTextColor(colorPM1, TFT_BLACK);
        tft.setCursor(xText, pmY); tft.print("PM1.0:");
        tft.setCursor(unitX, pmY); tft.print("ug/m3");

        lastColorPM1 = colorPM1;
        lastPM1 = -1; // принудительно обновим число
    }

    if (pm1 != lastPM1) {
        int numW = tft.textWidth("9999");
        tft.fillRect(numX, pmY - lineH + 5, numW, lineH, TFT_BLACK);

        snprintf(buf, sizeof(buf), "%3d", pm1);
        tft.setTextColor(colorPM1, TFT_BLACK);
        tft.setCursor(numX, pmY);
        tft.print(buf);

        lastPM1 = pm1;
    }

    // ===================== PM2.5 =====================
    uint16_t colorPM25 = pmColor(pm25);

    if (firstDraw || colorPM25 != lastColorPM25) {
        tft.fillRect(0, pmY + pmGap - lineH + 5, clearW, lineH, TFT_BLACK);

        tft.setTextColor(colorPM25, TFT_BLACK);
        tft.setCursor(xText, pmY + pmGap); tft.print("PM2.5:");
        tft.setCursor(unitX, pmY + pmGap); tft.print("ug/m3");

        lastColorPM25 = colorPM25;
        lastPM25 = -1;
    }

    if (pm25 != lastPM25) {
        int numW = tft.textWidth("9999");
        tft.fillRect(numX, pmY + pmGap - lineH + 5, numW, lineH, TFT_BLACK);

        snprintf(buf, sizeof(buf), "%3d", pm25);
        tft.setTextColor(colorPM25, TFT_BLACK);
        tft.setCursor(numX, pmY + pmGap);
        tft.print(buf);

        lastPM25 = pm25;
    }

    // ===================== PM10 =====================
    uint16_t colorPM10 = pmColor(pm10);

    if (firstDraw || colorPM10 != lastColorPM10) {
        tft.fillRect(0, pmY + pmGap * 2 - lineH + 5, clearW, lineH, TFT_BLACK);

        tft.setTextColor(colorPM10, TFT_BLACK);
        tft.setCursor(xText, pmY + pmGap * 2); tft.print("PM10:");
        tft.setCursor(unitX, pmY + pmGap * 2); tft.print("ug/m3");

        lastColorPM10 = colorPM10;
        lastPM10 = -1;
    }

    if (pm10 != lastPM10) {
        int numW = tft.textWidth("9999");
        tft.fillRect(numX, pmY + pmGap * 2 - lineH + 5, numW, lineH, TFT_BLACK);

        snprintf(buf, sizeof(buf), "%3d", pm10);
        tft.setTextColor(colorPM10, TFT_BLACK);
        tft.setCursor(numX, pmY + pmGap * 2);
        tft.print(buf);

        lastPM10 = pm10;
    }

    firstDraw = false;
}

// ---------------------------------------------------------------------
// Нижняя панель WiFi 
// ---------------------------------------------------------------------
void drawStatus(TFT_eSPI &tft) {
    static wl_status_t lastStatus = WL_IDLE_STATUS;
    static String lastIP = "";

    wl_status_t status = WiFi.status();
    String ip = (status == WL_CONNECTED) ? WiFi.localIP().toString() : "";

    if (status == lastStatus && ip == lastIP) return;

    tft.fillRect(0, 460, 320, 20, TFT_NAVY);
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setCursor(5, 460);

    if (status == WL_CONNECTED) {
        tft.print("WiFi: OK  ");
        tft.print(ip);
    } else {
        tft.print("WiFi: OFF Connect AP WeatherStation/192.168.4.1");
    }

    lastStatus = status;
    lastIP = ip;
}
