#pragma once
#include <TFT_eSPI.h>
#include <NTPClient.h>

// ===== Глобальные переменные погоды =====
extern float temperature;
extern String description;
extern String icon;
extern bool iconLoaded;

// ---------------------------------------------------------------------
// Смещение для блоков погоды и даты
// ---------------------------------------------------------------------
extern const int UI_SHIFT_Y;

// ---------------------------------------------------------------------
// Размеры блоков
// ---------------------------------------------------------------------
#define WX_Y 60                   // верх блока температуры
#define WX_H 80                   // высота блока температуры
#define DESC_Y (WX_Y + WX_H)      // верх блока описания погоды
#define DESC_H 40                 // высота блока описания погоды
#define DATE_Y (DESC_Y + DESC_H)  // верх блока даты
#define DATE_H 40                 // высота блока даты

// ---------------------------------------------------------------------
// UI параметры
// ---------------------------------------------------------------------
#define UI_MARGIN_X 24
#define UI_MARGIN_Y 8
#define ICON_SIZE 60

// ===== UI функции =====
void drawLayout(TFT_eSPI &tft);
void drawTime(TFT_eSPI &tft, NTPClient &timeClient);
void drawDate(TFT_eSPI &tft, NTPClient &timeClient);
void drawPM(TFT_eSPI &tft);
void drawStatus(TFT_eSPI &tft);
void updateWeatherUI(TFT_eSPI &tft);
