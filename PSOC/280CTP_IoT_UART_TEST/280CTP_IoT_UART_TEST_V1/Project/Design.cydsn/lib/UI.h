/* ========================================
 *
 * Copyright Suntech, 2023.02.01
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#ifndef _UI_H_
#define _UI_H_
    
#include "main.h"
#include "fonts.h"
    
#include "ST7789V.h"

enum DISPLAY_DIRECTION {
    DISPLAY_DIRECTION_PORTRAIT=0,
    DISPLAY_DIRECTION_LANDSCAPE,
};
    
#define DEFAULT_BUTTON_HEIGHT_PORTRAIT  46
#define DEFAULT_BUTTON_HEIGHT_LANDSCAPE 40
#define DEFAULT_TOP_TITLE_HEIGHT        40
#define DEFAULT_BOTTOM_BUTTON_HEIGHT    40

#define LCD_WIDTH  240
#define LCD_HEIGHT 320
    
#define NO_CLICK       0xFF
#define NO_SELECT      0xFFFF

extern uint8 g_displayDirection; // display 방향
extern uint16 g_DEFAULT_BUTTON_HEIGHT;
extern uint16 g_SCREEN_WIDTH;    // 화면 폭
extern uint16 g_SCREEN_HEIGHT;   // 화면 높이

typedef struct {
    uint16 left;
    uint16 right;
    uint16 top;    
    uint16 bottom;
} RECT;

typedef uint16 COLOR;

typedef struct {
    uint16 x;
    uint16 y;
} POINT;

typedef struct {
    uint16 isClick;
    POINT point;
} TOUCH;

//#define BODY_HEIGHT  (SCREEN_HEIGHT - (HEAD_HEIGHT+TAIL_HEIGHT))

#define CONVERT565(r,g,b) ( ((r>>3)<<11) | ((g>>3)<<5) | (b>>3) )
#define COLOR_CONVERT555(r,g,b) ( ((r>>3)<<10) | ((g>>3)<<5) | (b>>3) )

void setDisplayDirection(uint8 dir);

void FillAllArea(COLOR color); // 화면의 모든 영역을 주어진 색상으로 칠한다.
void FillRectangle(uint16 x1, uint16 y1, uint16 x2, uint16 y2, COLOR color);      // Rectangle을 색으로 채운다
void FillRect(RECT *r, COLOR color);                                              // Rectangle을 색으로 채운다
void DrawHorizontalLine(uint16 x1, uint16 x2, uint16 y, COLOR color);             // 수평선을 그린다
void DrawVerticalLine  (uint16 x, uint16 y1, uint16 y2, COLOR color);             // 수직선을 그린다.
void DrawRectangle     (uint16 x1, uint16 y1, uint16 x2, uint16 y2, COLOR color); // Rectangle외곽을 그린다.
void DrawRect          (RECT rect, COLOR color);                                  // Rectangle외곽을 그린다.

void DrawImage(uint16 xPos, uint16 yPos, uint16 width, uint16 height, uint16 *image); // Image를 그린다.
// 색상이 black이면(0x0000)면 blackReplaceColor로 색상을 변경한다.
void DrawImageBackColorReplace(uint16 xPos, uint16 yPos, uint16 width, uint16 height, uint16 *image, uint16 blackReplaceColor);
void LCD_DrawFont(uint16 xPos, uint16 yPos, uint16 foregroundColor, uint16 backgroundColor, fontdatatype *font, const char st); // 문자를 그린다.
void LCD_printf(uint16 xPos, uint16 yPos, uint16 foregroundColor, uint16 backgroundColor,   fontdatatype *font, const char *fmt, ...);
POINT CalculationOfLocationForString(RECT boundingBox, uint16 font_width, uint16 font_height, uint16 marginLeftRight, uint16 marginTopBottom, uint16 strlength, uint8 horizontalAlign, uint8 verticallAlign);

uint8 isPointInRect(RECT *r, POINT pt);
void RectangleSizeShrink(RECT *r, int16 left, int16 right, int16 up, int16 down); // width, height은 증분을 의미 한다.

TOUCH GetTouch();

enum BUZZER_TYPE {
    BUZZER_STOP = 0,
    BUZZER_CLICK,
    BUZZER_WARNING,
    BUZZER_WARNING_CONTINUOUS, 
    BUZZER_SHORT_WARNING,
    BUZZER_SHORT_WARNING_CONTINUOUS,     
    BUZZER_CONTINUOUS_BEEP,
};

enum TEXT_ALIGN {
    TEXT_ALIGN_CENTER = 0,    
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_RIGHT,
    TEXT_TWO_LINE,
    TEXT_ALIGN_TOP,
    TEXT_ALIGN_MIDDLE,
    TEXT_ALIGN_BOTTOM,
};

typedef struct {
    uint16 uDownCount;
    uint16 uUpCount;
    uint16 uInterate;
    uint16 type;
} BUZZER_CONTROL;

void Buzzer(uint8 type, uint16 noOfInterate);
void BuzzerTimer();

//uint16 getBottomAreaTopPosition(); // Bottom Button의 최상단 위치
RECT GetBodyArea();

void initUI();

#endif    
/* [] END OF FILE */
