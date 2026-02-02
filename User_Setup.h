//==========Выбрать нужный дисплей===============
#define ST7796_DRIVER
//#define ILI9488_DRIVER     // WARNING: Do not connect ILI9488 display SDO to MISO if other devices share the //SPI bus (TFT SDO does NOT tristate when CS is high)

#define TFT_WIDTH  320
#define TFT_HEIGHT 480
//#define TFT_INVERSION_ON // для дисплея  ILI9488 отлключить для ST7796
//#define TFT_MISO D6  // Automatically assigned with ESP8266 if not defined
//#define TFT_MOSI D7  // Automatically assigned with ESP8266 if not defined
//#define TFT_SCLK D5  // Automatically assigned with ESP8266 if not defined
#define TFT_CS   D8  // Chip select control pin D8
#define TFT_DC   D4  // Data Command control pin
#define TFT_RST  D3  // Reset pin (could connect to NodeMCU RST, see next line)
//#define TFT_RST  -1     // Set TFT_RST to -1 if the display RESET is connected to NodeMCU RST or 3.3V

// #define SPI_FREQUENCY   1000000
// #define SPI_FREQUENCY   5000000
// #define SPI_FREQUENCY  10000000
// #define SPI_FREQUENCY  20000000
#define SPI_FREQUENCY  27000000
// #define SPI_FREQUENCY  40000000
// #define SPI_FREQUENCY  55000000 // STM32 SPI1 only (SPI2 maximum is 27MHz)
// #define SPI_FREQUENCY  80000000

// ===== FONTS =====
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
