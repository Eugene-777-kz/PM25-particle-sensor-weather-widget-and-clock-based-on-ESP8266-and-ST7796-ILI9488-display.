#include "pms.h"

// Основной UART 
// SoftwareSerual не заработал как нужно, поэтому при заливке скетча нужно отключать датчик.
#define PMS_SERIAL Serial

uint16_t pm1 = 0;
uint16_t pm25 = 0;
uint16_t pm10 = 0;

// Порог изменения (антидребезг)
#define PM_DELTA 2   // мкг/м3

void pmsInit() {
    PMS_SERIAL.begin(9600);
}

// Чтение данных с PMS5003
void readPMS() {
    while (PMS_SERIAL.available() >= 32) {

        // Ищем заголовок кадра
        if (PMS_SERIAL.read() != 0x42) continue;
        if (PMS_SERIAL.read() != 0x4D) continue;

        uint8_t buf[30];
        PMS_SERIAL.readBytes(buf, 30);

        // Проверка длины пакета
        uint16_t frameLen = (buf[0] << 8) | buf[1];
        if (frameLen != 28) continue;

        // Значения PM 
        uint16_t tpm1  = (buf[8]  << 8) | buf[9];
        uint16_t tpm25 = (buf[10] << 8) | buf[11];
        uint16_t tpm10 = (buf[12] << 8) | buf[13];

        // Защита от мусора
        if (tpm1 >= 1000 || tpm25 >= 1000 || tpm10 >= 1000) continue;

        // --- Антидребезг ---
        if (abs((int)tpm1  - (int)pm1)  >= PM_DELTA) pm1  = tpm1;
        if (abs((int)tpm25 - (int)pm25) >= PM_DELTA) pm25 = tpm25;
        if (abs((int)tpm10 - (int)pm10) >= PM_DELTA) pm10 = tpm10;

        break; // пакет успешно обработан
    }
}
