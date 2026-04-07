#include "ssd1306.h"
#include "main.h"
#include "i2c.h"
#include <string.h>
#include <stdlib.h>


// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Screen object
static SSD1306_t SSD1306;
static uint16_t ssd1306Addr = SSD1306_I2C_ADDR;

//
//  Send a byte to the command register
//
static uint8_t ssd1306_WriteCommand(uint8_t command)
{
    return SSD1306_WriteCommand(ssd1306Addr, command);
}


//
//  Initialize the oled screen
//
uint8_t ssd1306_Init(void)
{
    // Wait for the screen to boot
    int status = 0;
	SSD1306.Inverted = 0;

    // Init LCD
    status += ssd1306_WriteCommand(0xAE);   // Display off
    status += ssd1306_WriteCommand(0x20);   // Set Memory Addressing Mode
    status += ssd1306_WriteCommand(0x10);   // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    status += ssd1306_WriteCommand(0xB0);   // Set Page Start Address for Page Addressing Mode,0-7
    status += ssd1306_WriteCommand(0xC8);   // Set COM Output Scan Direction
    status += ssd1306_WriteCommand(0x00);   // Set low column address
    status += ssd1306_WriteCommand(0x10);   // Set high column address
    status += ssd1306_WriteCommand(0x40);   // Set start line address
    status += ssd1306_WriteCommand(0x81);   // set contrast control register
    status += ssd1306_WriteCommand(0xFF);
    status += ssd1306_WriteCommand(0xA1);   // Set segment re-map 0 to 127
    status += ssd1306_WriteCommand(0xA6);   // Set normal display

    status += ssd1306_WriteCommand(0xA8);   // Set multiplex ratio(1 to 64)
    status += ssd1306_WriteCommand(SSD1306_HEIGHT - 1);

    status += ssd1306_WriteCommand(0xA4);   // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    status += ssd1306_WriteCommand(0xD3);   // Set display offset
    status += ssd1306_WriteCommand(0x00);   // No offset
    status += ssd1306_WriteCommand(0xD5);   // Set display clock divide ratio/oscillator frequency
    status += ssd1306_WriteCommand(0xF0);   // Set divide ratio
    status += ssd1306_WriteCommand(0xD9);   // Set pre-charge period
    status += ssd1306_WriteCommand(0x22);

    status += ssd1306_WriteCommand(0xDA);   // Set com pins hardware configuration
    status += ssd1306_WriteCommand(SSD1306_COM_LR_REMAP << 5 | SSD1306_COM_ALTERNATIVE_PIN_CONFIG << 4 | 0x02);

    status += ssd1306_WriteCommand(0xDB);   // Set vcomh
    status += ssd1306_WriteCommand(0x20);   // 0x20,0.77xVcc
    status += ssd1306_WriteCommand(0x8D);   // Set DC-DC enable
    status += ssd1306_WriteCommand(0x14);   //
    status += ssd1306_WriteCommand(0xAF);   // Turn on SSD1306 panel

    if (status != 0) {
        return 1;
    }

    // Clear screen
    if(SSD1306.Inverted) ssd1306_Fill(White);
    else 				 ssd1306_Fill(Black);

    // Flush buffer to screen
    ssd1306_UpdateScreen();

    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    SSD1306.Initialized = 1;

    return 0;
}

void ssd1306_SetDevAddr(uint16_t addr)
{
	ssd1306Addr = addr;
}

//
//  Fill the whole screen with the given color
//
void ssd1306_Fill(SSD1306_COLOR color)
{
    // Fill screenbuffer with a constant value (color)
    uint32_t i;

    for(i = 0; i < sizeof(SSD1306_Buffer); i++)
    {
        SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

void ssd1306_Clr(void){
    if(SSD1306.Inverted) ssd1306_Fill(White);
    else 				 ssd1306_Fill(Black);
}

//
//  Write the screenbuffer with changed to the screen
//
void ssd1306_UpdateScreen(void)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        ssd1306_WriteCommand(0xB0 + i);
        ssd1306_WriteCommand(0x00);
        ssd1306_WriteCommand(0x10);

        SSD1306_WriteData(ssd1306Addr, &SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH);
    }
}

//
//  Draw one pixel in the screenbuffer
//  X => X Coordinate
//  Y => Y Coordinate
//  color => Pixel color
//
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        // Don't write outside the buffer
        return;
    }

    // Check if pixel should be inverted
    if (SSD1306.Inverted)
    {
        color = (SSD1306_COLOR)!color;
    }

    // Draw in the correct color
    if (color == White)
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    }
    else
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

void ssd1306_Pixel(uint8_t x, uint8_t y){
    ssd1306_DrawPixel(x,y,White);
}


//
//  Draw 1 char to the screen buffer
//  ch      => Character to write
//  Font    => Font to use
//  color   => Black or White
//
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
    uint32_t i, b, j;

    // Check remaining space on current line
    if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font.FontWidth) ||
        SSD1306_HEIGHT <= (SSD1306.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }

    // Translate font to screenbuffer
    if(SSD1306.CurrentY > 1)
    {
        for (j = 0; j < Font.FontWidth; j++)
        {
        	ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY - 2), (SSD1306_COLOR) !color);
        	ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY - 1), (SSD1306_COLOR) !color);
        }
    }
    else if(SSD1306.CurrentY > 0)
    {
        for (j = 0; j < Font.FontWidth; j++)
        {
        	ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY - 1), (SSD1306_COLOR) !color);
        }
    }


    for (i = 0; i < Font.FontHeight; i++)
    {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++)
        {
            if ((b << j) & 0x8000)
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            }
            else
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }

    if((SSD1306.CurrentY + Font.FontHeight) < (SSD1306_HEIGHT-1))
    {
        for (j = 0; j < Font.FontWidth; j++)
        {
        	ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + Font.FontHeight + 0), (SSD1306_COLOR) !color);
        	ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + Font.FontHeight + 1), (SSD1306_COLOR) !color);
        }
    }
    else if((SSD1306.CurrentY + Font.FontHeight) < (SSD1306_HEIGHT-0))
    {
        for (j = 0; j < Font.FontWidth; j++)
        {
        	ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + Font.FontHeight + 0), (SSD1306_COLOR) !color);
        }
    }

    // The current space is now taken
    SSD1306.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

//
//  Write full string to screenbuffer
//
char ssd1306_WriteString(const char* str, FontDef Font, SSD1306_COLOR color)
{
    // Write until null-byte
    while (*str)
    {
        if (ssd1306_WriteChar(*str, Font, color) != *str)
        {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

//
//  Invert background/foreground colors
//
void ssd1306_InvertColors(void)
{
    SSD1306.Inverted = !SSD1306.Inverted;
}

//
//  Set cursor position
//
void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

void SSD1306_Txt(uint8_t x, uint8_t y, const char* str, FontDef Font,SSD1306_INVERT inv){
	ssd1306_SetCursor(x,y);
	if(Invert==inv) ssd1306_InvertColors();
	ssd1306_WriteString(str, Font, White);
	if(Invert==inv) ssd1306_InvertColors();
}
void SSD1306_TxtMidd(uint8_t x,uint8_t y,  const char* str,FontDef Font,SSD1306_INVERT inv){
	uint8_t width  = Font.FontWidth*strlen(str);
	uint8_t height = Font.FontHeight;
	ssd1306_SetCursor( MIDDLE(x,SSD1306_WIDTH,width), MIDDLE(y,SSD1306_HEIGHT,height) );
	if(Invert==inv) ssd1306_InvertColors();
	ssd1306_WriteString(str, Font, White);
	if(Invert==inv) ssd1306_InvertColors();
}
void SSD1306_TxtMiddX(uint8_t x,uint8_t y,  const char* str,FontDef Font,SSD1306_INVERT inv){
	uint8_t width  = Font.FontWidth*strlen(str);
	ssd1306_SetCursor( MIDDLE(x,SSD1306_WIDTH,width), y );
	if(Invert==inv) ssd1306_InvertColors();
	ssd1306_WriteString(str, Font, White);
	if(Invert==inv) ssd1306_InvertColors();
}
void SSD1306_TxtMiddY(uint8_t x,uint8_t y,  const char* str,FontDef Font,SSD1306_INVERT inv){
	uint8_t height = Font.FontHeight;
	ssd1306_SetCursor( x, MIDDLE(y,SSD1306_HEIGHT,height) );
	if(Invert==inv) ssd1306_InvertColors();
	ssd1306_WriteString(str, Font, White);
	if(Invert==inv) ssd1306_InvertColors();
}
void SSD1306_TxtMiddBK(uint8_t x,uint8_t y,  uint8_t widthBk,uint8_t heightBk,  const char* str,FontDef Font,SSD1306_INVERT inv){
	uint8_t width  = Font.FontWidth*strlen(str);
	uint8_t height = Font.FontHeight;
	ssd1306_SetCursor( MIDDLE(x,widthBk,width), MIDDLE(y,heightBk,height) );
	if(Invert==inv) ssd1306_InvertColors();
	ssd1306_WriteString(str, Font, White);
	if(Invert==inv) ssd1306_InvertColors();
}
void SSD1306_DispBK(uint16_t addrDev,uint8_t invert){
	  if(invert) ssd1306_InvertColors();
	  ssd1306_Clr();
	  ssd1306_SetDevAddr(addrDev);
}
uint8_t SSD1306_diffY(FontDef Font1,FontDef Font2){  return abs(Font1.FontHeight-Font2.FontHeight);  }
uint8_t SSD1306_diffX(FontDef Font1,FontDef Font2){  return abs(Font1.FontWidth -Font2.FontWidth );  }

uint8_t SSD1306_posX(void){  return SSD1306.CurrentX;  }
uint8_t SSD1306_posY(void){  return SSD1306.CurrentY;  }

void SSD1306_rect(uint8_t x,uint8_t y, uint8_t width,uint8_t height){
	for (uint8_t i=0; i<width; i++) {
		for (uint8_t j=0; j<height; j++) {
			ssd1306_Pixel(x+i,y+j);
	}}
}
void SSD1306_frame(uint8_t x,uint8_t y, uint8_t width,uint8_t height){
	for (uint8_t i=0; i<width; i++) {
		for (uint8_t j=0; j<height; j++) {
			if(i==0 || i==width-1 || j==0 || j==height-1)  ssd1306_Pixel(x+i,y+j);
	}}
}
void SSD1306_roundRect(uint8_t x,uint8_t y, uint8_t width,uint8_t height){
	uint8_t i,j;
	for (i=0; i<width; i++) {
		for (j=0; j<height; j++) {
			if( (i==0&&j==0) || (i==width-1&&j==0) || (i==0&&j==height-1) || (i==width-1&&j==height-1) );
			else ssd1306_Pixel(x+i,y+j);
	}}
}
void SSD1306_roundFrame(uint8_t x,uint8_t y, uint8_t width,uint8_t height){
	uint8_t i,j;
	for (i=0; i<width; i++) {
		for (j=0; j<height; j++) {
			if(i==0 || i==width-1 || j==0 || j==height-1){
				if( (i==0&&j==0) || (i==width-1&&j==0) || (i==0&&j==height-1) || (i==width-1&&j==height-1) );
				else ssd1306_Pixel(x+i,y+j);
			}
	}}
}

int SD1306_Inits(uint8_t* addrDispTab){
	int ret=0;
	ssd1306_SetDevAddr(addrDispTab[0]);	if(ssd1306_Init()!=0) ret|=1;
	ssd1306_SetDevAddr(addrDispTab[1]); if(ssd1306_Init()!=0) ret|=2;
	return ret;
}

void EXAMPLE_DrawTxt(uint8_t* addrDispTab)
{
	  ssd1306_SetDevAddr(addrDispTab[0]);
	  ssd1306_Clr();
	  SSD1306_Txt(0,2,"Radio krakow",Font_7x10,Invert);
	  SSD1306_Txt(0,25,"*!a@c&",Font_7x10,NoInvert);
	  SSD1306_Txt(0,45,"?ay|W",Font_7x10,Invert);
	  SSD1306_Txt(70,45,"1+2=3",Font_7x10,NoInvert);
	  SSD1306_roundRect(100,0, 10,10);
	  SSD1306_rect(100,20, 10,10);
	  ssd1306_UpdateScreen();

	  ssd1306_SetDevAddr(addrDispTab[1]);
	  ssd1306_Clr();
	  SSD1306_Txt(  0, 15, "1234+5!", Font_7x10,NoInvert);
	  SSD1306_Txt(  0, 35, "1234+5!", Font_7x10,Invert);
	  SSD1306_roundRect(100,0, 10,10);
	  SSD1306_rect(100,20, 10,10);
	  ssd1306_UpdateScreen();

	  HAL_Delay(2300);

	  ssd1306_SetDevAddr(addrDispTab[0]);
	  ssd1306_Clr(); ssd1306_InvertColors();
	  SSD1306_Txt(  0, 15, "Rafa", Font_7x10,Invert);
	  SSD1306_Txt(  0, 35, "Rafa", Font_7x10,NoInvert);
	  ssd1306_InvertColors();
	  SSD1306_roundRect(100,20, 10,10);
	  ssd1306_UpdateScreen();

	  ssd1306_SetDevAddr(addrDispTab[1]);
	  ssd1306_InvertColors();
	  ssd1306_Clr();
	  SSD1306_Txt(  0, 15, "!@#I|$%^&*()_+yX", Font_7x10,NoInvert);
	  SSD1306_Txt(  0, 35, "!@#I|$%^&*()_+yX", Font_7x10,Invert);
	  SSD1306_roundRect(100,0, 10,10);
	  ssd1306_UpdateScreen();
}
