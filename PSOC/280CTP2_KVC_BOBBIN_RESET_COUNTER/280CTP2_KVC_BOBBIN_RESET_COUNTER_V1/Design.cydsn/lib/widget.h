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
#ifndef _WIDGET_H_
#define _WIDGET_H_

#include "menu.h"
#include "UI.h"
#include "button.h"
    
#define NO_BASIC_MENU_BUTTONS 8
enum BASIC_MENU_IDX
{
    IDX_TITLE_BTN=0,
    IDX_LIST_BTN1,
    IDX_LIST_BTN2,
    IDX_LIST_BTN3,
    IDX_LIST_BTN4,
    IDX_LIST_BTN5,
    IDX_SCROLL_UP,
    IDX_SCROLL_DOWN
};

enum BOTTOM_BUTTON {
    BOTTOM_LEFT = 0,
    BOTTOM_RIGHT = 1,
};

typedef struct {
    char title[20];
    char listText[20][MAX_NODE_NAME];
    BUTTON btns[5];
    uint16 noOfDisplayButton;
    uint16 sizeOfList;
    BUTTON upScroll;
    BUTTON downScroll;
    uint16 curPage;
} LIST_MENU;

typedef struct {
    char title[20];
    char message[20];
    char remark[20];
    BUTTON btn[2];
    BUTTON btnMessage;   
} UP_DOWN_MENU;

typedef struct {
   BUTTON btn[7];
} MONITORING_MENU;

enum UP_DOWN_TWO {
    UPDOWNTWO_TITLE = 0,
    UPDOWNTWO_FIRST_TEXT,
    UPDOWNTWO_SECOND_TEXT,
    UPDOWNTWO_FIRST_VALUE,
    UPDOWNTWO_SECOND_VALUE,
    UPDOWNTWO_UP_BUTTON,
    UPDOWNTWO_DOWN_BUTTON,
};

typedef struct {
   BUTTON btn[7];
} UP_DOWN_TWO_MENU;

typedef struct {
   BUTTON btn[8];
   uint16 inc[3];
} UP_DOWN_ONE_MENU;

typedef struct {
    char title[20];
    char message[20];
} YES_NO_MENU;

void initWidget();
void MakeButton(BUTTON *button, uint16 y, uint16 height, uint8 style, const char *fmt, ...);
void SetDrawListButtons(LIST_MENU *menu, char *title, char *textArray[], uint16 sizeOfList);
void SetDrawNodeListButtons(MENUNODE *parent, LIST_MENU *menu);
void UpdateDrawListButtons(LIST_MENU *menu, uint16 indexSelect);
int doListMenu(void *this, uint8 reflash);

void SetDrawBottomButtons(char *leftText, char *rightText, uint16 styleLeft, uint16 styleRight);
void SetDrawMonitoringMenu(MONITORING_MENU *menu, const char *title, ...);
void SetDrawUpDownTwoMenu(UP_DOWN_TWO_MENU *menu, char *first, char *second, const char *title, ...);
void SetDrawUpDownOneMenu(UP_DOWN_ONE_MENU *menu, char *first, const char *title, ...);

void DrawUpDownMenu(UP_DOWN_MENU *menu, char *title, char *remark, const char *fmt, ...);

void SetDrawYesNoButtons(YES_NO_MENU *menu, const char *message, ...);

void ShowMessage(const char *msg, ...);
void ShowWaitMessage();

void SetTitleBarText(char *str);
void DrawWifi();
void DrawTitle();
void DrawHeader();

void EraseBlankArea(uint16 y, uint8 withScroll);
void EraseBlankAreaWithoutHeader();
void SetDrawScrollButton(LIST_MENU *menu);

uint16 getIndexOfClickedListMenu(TOUCH *tc, LIST_MENU *menu);

extern BUTTON       g_TitleBar;
extern BUTTON       g_btnBottom[2];
extern LIST_MENU    g_ListMenu;
//extern BUTTON_ARRAY g_btnList;
extern IMAGE g_imageUpArrow;
extern IMAGE g_imageDownArrow;
extern IMAGE g_imageWifi;
#endif    
/* [] END OF FILE */
