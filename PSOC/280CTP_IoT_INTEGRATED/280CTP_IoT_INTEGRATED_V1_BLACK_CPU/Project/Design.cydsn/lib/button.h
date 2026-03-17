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
#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "UI.h"
    
#define MAX_BUTTON_TEXT_SIZE 32
    
typedef struct {
    uint16 *image;
    uint16 width;
    uint16 height;
} IMAGE;

typedef struct {
    IMAGE *image;
    RECT rect;               // button area
    char text[MAX_BUTTON_TEXT_SIZE];
    fontdatatype *font;    
    COLOR foregroundColor;
    COLOR backgroundColor;
    COLOR outlineColor;
    uint16 marginLeftRight;
    uint16 marginTopBottom;
    uint8 align;             // text align, Left Center Right
    uint8 disable;           // no pick
    uint8 show;              // no show : no text, fill black
    uint8 ghost;             // no drawing
} BUTTON;

typedef struct {
    BUTTON btn[10];
    uint16 size;
} BUTTON_ARRAY;


enum BUTTON_SIZE {
    // for x direction    
    BUTTON_X_TITLE_BAR = 0, 
    BUTTON_X_FULL,
    BUTTON_X_1_2,
    BUTTON_X_2_2,
    BUTTON_X_1_3,
    BUTTON_X_2_3,
    BUTTON_X_3_3,
    BUTTON_X_1_4,    
    BUTTON_X_2_4,
    BUTTON_X_3_4,
    BUTTON_X_4_4,    
    // for y direction
    BUTTON_Y,      // User must input y postion
    BUTTON_Y_TOP,
    BUTTON_Y_FIRST,
    BUTTON_Y_HALF,
    BUTTON_Y_2_3,
    BUTTON_Y_3_3,
    BUTTON_Y_2_4,
    BUTTON_Y_3_4,
    BUTTON_Y_4_4,
    BUTTON_Y_BOTTOM,     
};

enum BUTTON_STYLE_COLOR {    // Foreground, Backgound,    Outline
     BUTTON_STYLE_LIST = 0   , //     WHITE,           BLUE,   DARKGREY
     BUTTON_STYLE_TITLE      , //     WHITE,            RED,   DARKGREY
     BUTTON_STYLE_EXIT       , //     WHITE,           BLUE,   DARKGREY
     BUTTON_STYLE_SAVE       , //     WHITE,          GREEN,   DARKGREY
     BUTTON_STYLE_TEXT       , //     WHITE,          BLACK,      BLACK
     BUTTON_STYLE_WHITE      , //     WHITE,          BLACK,      BLACK
     BUTTON_STYLE_RED        , //       RED,          BLACK,      BLACK
     BUTTON_STYLE_ORANGE     , //    ORANGE,          BLACK,      BLACK
     BUTTON_STYLE_YELLOW     , //    YELLOW,          BLACK,      BLACK
     BUTTON_STYLE_GREEN      , //     GREEN,          BLACK,      BLACK
     BUTTON_STYLE_BLUE       , //      BLUE,          BLACK,      BLACK
     BUTTON_STYLE_LIGHTGREY  , // LIGHTGREY,          BLACK,      BLACK
     BUTTON_STYLE_DARKGREY   , //  DARKGREY,          BLACK,      BLACK    
     BUTTON_STYLE_R_WHITE    , //     BLACK,          WHITE,   DARKGREY
     BUTTON_STYLE_R_RED      , //     BLACK,            RED,   DARKGREY
     BUTTON_STYLE_R_ORANGE   , //     BLACK,         ORANGE,      BLACK
     BUTTON_STYLE_R_YELLOW   , //     BLACK,         YELLOW,   DARKGREY
     BUTTON_STYLE_R_GREEN    , //     BLACK,          GREEN,   DARKGREY
     BUTTON_STYLE_R_BLUE     , //     BLACK,           BLUE,   DARKGREY
     BUTTON_STYLE_R_LIGHTGREY, //     BLACK,      LIGHTGREY,   DARKGREY
     BUTTON_STYLE_R_DARKGREY,  //     BLACK,       DARKGREY,   DARKGREY
     BUTTON_STYLE_NONE,
};

void SetDefaultButtonStyle(BUTTON *btn);
void SetButtonStyleColor(BUTTON *btn, uint16 styleColor);

void SetButtonSize(BUTTON *btn, uint16 y, uint16 height, uint8 x_size_type, uint8 y_size_type, uint8 withScroll);
void SetButtonText(BUTTON *btn, const char *fmt, ...);
//void SetIconButton(ICON_BUTTON *btn, uint16 x, uint16 y, uint16 width, uint16 height, uint16 *image);


void SetDefaultButtonStyle(BUTTON *btn);
void SetDefaultButtonStyleArray(BUTTON *btn, uint16 size);
void SetHiddenButtonStyle(BUTTON *btn);

void DrawButtonText(BUTTON *btn);
void DrawButton(BUTTON *btn);
void DrawButtons(BUTTON *btn, uint16 size);
void DrawButtonArray(BUTTON_ARRAY *array);

void DrawImageRect(RECT *rect, IMAGE *image, uint16 replaceBlackColor); // rect의 중간에 image를 그린다
//void DrawIconButton(ICON_BUTTON *btn);

uint16 getIndexOfClickedButtonArray(TOUCH *tc, BUTTON_ARRAY *array);
uint16 getIndexOfClickedButton     (TOUCH *tc, BUTTON *btns, uint16 size);
uint16 getClickedButton            (TOUCH *tc, BUTTON *btn);
#endif    