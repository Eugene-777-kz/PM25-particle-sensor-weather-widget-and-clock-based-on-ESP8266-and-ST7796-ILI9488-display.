#pragma once
#include <TFT_eSPI.h>

// ===== Глобальные переменные погоды =====
extern float temperature;
extern String description;
extern String icon;
extern bool iconLoaded;

// ===== Функции =====
void weatherInit();
void updateWeather();
void loadIconToBuffer();
void drawWeatherBlock(TFT_eSPI &tft);
void drawWeatherDescription(TFT_eSPI &tft);
