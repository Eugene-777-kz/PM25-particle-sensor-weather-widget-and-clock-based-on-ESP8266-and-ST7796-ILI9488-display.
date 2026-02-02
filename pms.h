#pragma once
#include <Arduino.h>

// ===== Глобальные переменные =====
extern uint16_t pm1;
extern uint16_t pm25;
extern uint16_t pm10;

// ===== Функции =====
void pmsInit();
void readPMS();
