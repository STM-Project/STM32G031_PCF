/**
 * This Library is written and optimized by Olivier Van den Eede(4ilo) in 2016
 * for Stm32 Uc and HAL-i2c lib's.
 *
 * To use this library with ssd1306 oled display you will need to customize the defines below.
 *
 * This library uses 2 extra files (fonts.c/h).
 * In this files are 3 different fonts you can use:
 *     - Font_7x10
 *     - Font_11x18
 *     - Font_16x26
 *
 */

#ifndef _SSD1306_H
#define _SSD1306_H

#include "fonts.h"
#include "main.h"

// I2c address
#ifndef SSD1306_I2C_ADDR
#define SSD1306_I2C_ADDR        0x7A
#endif // SSD1306_I2C_ADDR

// SSD1306 width in pixels
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH           128
#endif // SSD1306_WIDTH

// SSD1306 LCD height in pixels
#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT          64
#endif // SSD1306_HEIGHT

#ifndef SSD1306_COM_LR_REMAP
#define SSD1306_COM_LR_REMAP    0
#endif // SSD1306_COM_LR_REMAP

#ifndef SSD1306_COM_ALTERNATIVE_PIN_CONFIG
#define SSD1306_COM_ALTERNATIVE_PIN_CONFIG    1
#endif // SSD1306_COM_ALTERNATIVE_PIN_CONFIG


//
//  Enumeration for screen colors
//
typedef enum {
    Black = 0x00,   // Black color, no pixel
    White = 0x01,   // Pixel is set. Color depends on LCD
} SSD1306_COLOR;

typedef enum {
    NoInvert = 0,
    Invert = 1,
} SSD1306_INVERT;

//
//  Struct to store transformations
//
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
} SSD1306_t;

//
//  Function definitions
//

uint8_t ssd1306_Init(void);
void ssd1306_UpdateScreen(void);
void ssd1306_Fill(SSD1306_COLOR color);
void ssd1306_Clr(void);
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color);
char ssd1306_WriteString(const char* str, FontDef Font, SSD1306_COLOR color);
void ssd1306_SetCursor(uint8_t x, uint8_t y);
void ssd1306_InvertColors(void);

void ssd1306_SetDevAddr(uint16_t addr);
void SSD1306_Txt(uint8_t x, uint8_t y, const char* str, FontDef Font, SSD1306_INVERT inv);
void ssd1306_Pixel(uint8_t x, uint8_t y);
void SSD1306_TxtMiddBK(uint8_t x,uint8_t y,  uint8_t widthBk,uint8_t heightBk,  const char* str,FontDef Font);
void SSD1306_TxtMidd(uint8_t x,uint8_t y,  const char* str,FontDef Font);
void SSD1306_DispBK(uint16_t addrDev,uint8_t invert);

uint8_t SSD1306_diffY(FontDef Font1,FontDef Font2);
uint8_t SSD1306_diffX(FontDef Font1,FontDef Font2);

uint8_t SSD1306_posX(void);
uint8_t SSD1306_posY(void);

void SSD1306_rect(uint8_t x,uint8_t y, uint8_t width,uint8_t height);
void SSD1306_roundRect(uint8_t x,uint8_t y, uint8_t width,uint8_t height);

int EXAMPLE_DrawTxt(uint8_t* addrDispTab);

#endif  // _SSD1306_H
