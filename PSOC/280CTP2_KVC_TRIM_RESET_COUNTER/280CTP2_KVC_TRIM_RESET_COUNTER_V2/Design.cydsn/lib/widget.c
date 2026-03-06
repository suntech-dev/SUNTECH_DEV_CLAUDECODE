/* ========================================
 *
 * Copyright Suntech, 2023.02.01
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "widget.h"
#include "menuDesign.h"
#include "image.h"
#include "externalFlash.h"
#include "UI.h"
#include "LEDControl.h"

BUTTON g_btnBottom[2];

IMAGE g_imageUpArrow =
{
    .image = image_arrow_up,
    .width = 32,
    .height=32,
};

IMAGE g_imageDownArrow =
{
    .image = image_arrow_down,
    .width = 32,
    .height=32,
};

BUTTON      g_TitleBar;
LIST_MENU   g_ListMenu;

IMAGE g_DownArrowImage;
void initWidget()
{
    // title Bar - 전체 화면 너비 사용 /////////////////////////////////////
    g_TitleBar.rect.right  = g_SCREEN_WIDTH - 1;  // WiFi 영역 제거, 전체 너비 사용
    g_TitleBar.rect.left   = 0;
    g_TitleBar.rect.top    = 0;
    g_TitleBar.rect.bottom = DEFAULT_TOP_TITLE_HEIGHT-2;

    SetButtonStyleColor(&g_TitleBar,BUTTON_STYLE_WHITE);
    SetButtonText(&g_TitleBar,"Trim Reset");

    initMenuDesign();
}

void SetDrawBottomButtons(char *leftText, char *rightText, uint16 styleLeft, uint16 styleRight)
{
    BUTTON *leftButton  = &g_btnBottom[0];
    BUTTON *rightButton = &g_btnBottom[1];
    
    SetDefaultButtonStyle(leftButton);
    SetButtonStyleColor(leftButton,  styleLeft);    
    SetButtonSize(leftButton, 0,g_DEFAULT_BUTTON_HEIGHT,BUTTON_X_1_2,BUTTON_Y_BOTTOM,FALSE);

    SetDefaultButtonStyle(rightButton);
    SetButtonStyleColor(rightButton, styleRight);
    SetButtonSize(rightButton,0,g_DEFAULT_BUTTON_HEIGHT,BUTTON_X_2_2,BUTTON_Y_BOTTOM,FALSE);        
 
    // Bottom left Button       
    if(leftText != NULL)
    {
        SetButtonText(leftButton, leftText);
    } else {
        SetHiddenButtonStyle(leftButton);
    }
    
    // Bottom Right Button        
    if(rightText != NULL)
    {
        SetButtonText(rightButton, rightText);
    } else {
        SetHiddenButtonStyle(rightButton);
    }
    
    DrawButtons(g_btnBottom,2);    
}

uint16 DrawTitleButton(char *title, uint8 withScroll)
{    
    // Title Button  
    if(title != NULL)
    {
        BUTTON btn;
        SetDefaultButtonStyle(&btn);        
        SetButtonSize(&btn,0,g_DEFAULT_BUTTON_HEIGHT-1,BUTTON_X_FULL,BUTTON_Y_FIRST,withScroll);
        SetButtonStyleColor(&btn,BUTTON_STYLE_R_RED);        
        SetButtonText(&btn, title);        
        DrawButton(&btn);
        
        return btn.rect.bottom + 1;
    }
    return 0;
}

// 리스트 버턴에 넣을 수 있는 버턴의 개수
uint16 getMaxNoOfItemInListButton()
{
    return (getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT) ? 5 : 4; 
}

uint8 isExistScroll(LIST_MENU *menu)
{
    return (menu->noOfDisplayButton < menu->sizeOfList) ? TRUE : FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetDrawListButtons(LIST_MENU *menu, char *title, char *textArray[], uint16 sizeOfList)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    indexSelect: selected index, if no used 0xFFFF
*/
{
    RECT bodyRect = GetBodyArea();
    uint16 yPos = bodyRect.top;
    uint16 yPosTitle;
 
    menu->noOfDisplayButton = getMaxNoOfItemInListButton();

    if(title != NULL) menu->noOfDisplayButton--;
   
    menu->sizeOfList = sizeOfList;
    strcpy(menu->title,title);
  //  menu->listText = textArray;
    
    uint8 withScroll = isExistScroll(menu);
    
    if((yPosTitle = DrawTitleButton(title,withScroll)) > 0) yPos = yPosTitle;
      
    if(textArray != NULL)
        for(int i=0; i < menu->noOfDisplayButton; i++)
        {                        
            BUTTON *btn = &menu->btns[i];
            SetDefaultButtonStyle(btn);     
            SetButtonStyleColor(btn, BUTTON_STYLE_LIST);        
            SetButtonSize      (btn, yPos, g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y,withScroll);
            btn->align = TEXT_ALIGN_LEFT;
            
            uint16 textIdx = menu->noOfDisplayButton * menu->curPage + i;
            if(textIdx >= menu->sizeOfList) break;
         
            strcpy(menu->listText[textIdx],textArray[textIdx]);
            strcpy(btn->text,textArray[textIdx]);        
            yPos += g_DEFAULT_BUTTON_HEIGHT;
           // DrawButton(btn);
        }
    else
        {
            EraseBlankArea(yPos, FALSE);
        }

    if(withScroll) SetDrawScrollButton(menu);     
}

void SetDrawNodeListButtons(MENUNODE *parent, LIST_MENU *menu)
{
    RECT bodyRect = GetBodyArea();
    uint16 yPos = bodyRect.top;
    uint16 yPosTitle;
 
    menu->noOfDisplayButton = getMaxNoOfItemInListButton() -1;
   
    menu->sizeOfList = getNoOfChild(parent);
    strcpy(menu->title,parent->nodeName);
    
    uint8 withScroll = isExistScroll(menu);
    
    if((yPosTitle = DrawTitleButton(menu->title,withScroll)) > 0) yPos = yPosTitle;
      
    if(parent->firstChild != NULL)
    {
        for(int i=0; i < menu->sizeOfList; i++)
        {  
            strcpy(menu->listText[i],getNthChild(parent,i)->nodeName);              
        }
        
        for(int i=0; i < menu->noOfDisplayButton; i++)
        {                        
            BUTTON *btn = &menu->btns[i];
            SetDefaultButtonStyle(btn);     
            SetButtonStyleColor(btn, BUTTON_STYLE_LIST);        
            SetButtonSize      (btn, yPos, g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y,withScroll);
            btn->align = TEXT_ALIGN_LEFT;
            
            uint16 textIdx = menu->noOfDisplayButton * menu->curPage + i;
            if(textIdx >= menu->sizeOfList) break;
            
            strcpy(btn->text,menu->listText[textIdx]);        
            yPos += g_DEFAULT_BUTTON_HEIGHT;
        }
    }
    else
        {
            EraseBlankArea(yPos, FALSE);
        }

    if(withScroll) SetDrawScrollButton(menu);       
}
//
//void SetDrawNodeListButtons2(MENUNODE *parent, LIST_MENU *menu)
//{
//    RECT bodyRect = GetBodyArea();
//    uint16 yPos = bodyRect.top;
//    uint16 yPosTitle;
// 
//    menu->noOfDisplayButton = getMaxNoOfItemInListButton() -1;
//   
//    menu->sizeOfList = getNoOfChild(parent);
//    strcpy(menu->title,parent->nodeName);
//    
//    uint8 withScroll = isExistScroll(menu);
//    
//    if((yPosTitle = DrawTitleButton(menu->title,withScroll)) > 0) yPos = yPosTitle;
//      
//    if(parent->firstChild != NULL)
//        for(int i=0; i < menu->noOfDisplayButton; i++)
//        {                        
//            BUTTON *btn = &menu->btns[i];
//            SetDefaultButtonStyle(btn);     
//            SetButtonStyleColor(btn, BUTTON_STYLE_LIST);        
//            SetButtonSize      (btn, yPos, g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y,withScroll);
//            btn->align = TEXT_ALIGN_LEFT;
//            
//            uint16 textIdx = menu->noOfDisplayButton * menu->curPage + i;
//            if(textIdx >= menu->sizeOfList) break;
//            
//            MENUNODE *child = getNthChild(parent,textIdx);
//            strcpy(menu->listText[textIdx],child->nodeName);
//            strcpy(btn->text,menu->listText[textIdx]);        
//            yPos += g_DEFAULT_BUTTON_HEIGHT;
//        }
//    else
//        {
//            EraseBlankArea(yPos, FALSE);
//        }
//
//    if(withScroll) SetDrawScrollButton(menu);       
//}
void UpdateDrawListButtons(LIST_MENU *menu, uint16 indexSelect)
{
    uint16 yPos;    
        
    for(int i=0; i < menu->noOfDisplayButton; i++)
    {                        
        int j = i+menu->curPage*menu->noOfDisplayButton;
        if(j >= menu->sizeOfList) break;
        
        BUTTON *btn = &menu->btns[i];
        
        if(indexSelect != NO_SELECT)
        {
            char buff[30];
            sprintf(buff,"%c%s",(indexSelect==j) ? '>' : ' ',menu->listText[j]);
            strcpy(btn->text,buff);              
        } else {
            SetButtonText(btn,menu->listText[j]);              
        }

        DrawButton(btn);
        yPos = btn->rect.bottom + 1;
    }
    
    EraseBlankArea(yPos, isExistScroll(menu));
}

uint16 getIndexOfClickedListMenu(TOUCH *tc, LIST_MENU *menu)
{
    int8 bIsFirstPage = (menu->curPage == 0 ) ? TRUE : FALSE;
    int8 bIsLastPage  = (menu->curPage == (menu->sizeOfList-1) / menu->noOfDisplayButton) ? TRUE : FALSE;
        
    for(int i=0; i < menu->noOfDisplayButton; i++)
    {
        int j = i + menu->curPage * menu->noOfDisplayButton;
        
        if(j >= menu->sizeOfList) break;
        
        if(getClickedButton(tc,  &menu->btns[i]) == TRUE)
        {
            return j;
        }
    }
     
    if(! bIsFirstPage) // 처음 페이지가 아니면 앞으로 이동 할 수 있음
    {
        if(getClickedButton(tc,  &menu->upScroll) == TRUE) return IDX_SCROLL_UP;
    }
    
    if(!bIsLastPage) // 마지막 페이지가 아니면 뒤로 이동 할 수 있음
    {
        if(getClickedButton(tc,  &menu->downScroll) == TRUE) return IDX_SCROLL_DOWN;        
    }
        
    return NO_CLICK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int doListMenu(void *this, uint8 reflash) ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;     
    static uint16 noOfList;
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                noOfList = getNoOfChild(thisMenu);
                g_ListMenu.curPage = 0;
                SetDrawNodeListButtons(thisMenu, &g_ListMenu);
                UpdateDrawListButtons(&g_ListMenu,NO_SELECT);
                SetDrawBottomButtons("QUIT", NULL, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE); 
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                
                uint16 idx = getIndexOfClickedListMenu(&tc, &g_ListMenu);
                
                switch(idx)
                {
                    case IDX_SCROLL_UP:   g_ListMenu.curPage--;
                          UpdateDrawListButtons(&g_ListMenu,NO_SELECT);
                        break;
                    case IDX_SCROLL_DOWN: g_ListMenu.curPage++;
                          UpdateDrawListButtons(&g_ListMenu,NO_SELECT);                   
                        break;
                    case NO_CLICK :
                        {
                            switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                            {
                                case BOTTOM_LEFT:  return MENU_RETURN_PARENT;
                            //  case BOTTOM_RIGHT: return 1;                            
                            }
                            break;
                        }
                    default: 
                        if(idx < noOfList) return idx;
                }                
            }
            break;
    }
    return MENU_RETURN_THIS;    
}


void SetDrawScrollButton(LIST_MENU *menu)
{
    RECT r = GetBodyArea();
   
    menu->upScroll.rect.left     = g_SCREEN_WIDTH - 40;
    menu->upScroll.rect.right    = g_SCREEN_WIDTH -  1;    
    menu->upScroll.rect.top      = r.top;  
    menu->upScroll.rect.bottom   = (r.top + r.bottom)*.5;
    SetDefaultButtonStyle(&menu->upScroll);
    menu->upScroll.image = &g_imageUpArrow;  

    menu->downScroll.rect.left     = g_SCREEN_WIDTH - 40;
    menu->downScroll.rect.right    = g_SCREEN_WIDTH -  1;    
    menu->downScroll.rect.top      = (r.top + r.bottom)*.5 + 1;
    menu->downScroll.rect.bottom   = r.bottom;
    SetDefaultButtonStyle(&menu->downScroll);
    menu->downScroll.image = &g_imageDownArrow;  
    
    DrawButton(&menu->upScroll);
    DrawButton(&menu->downScroll);   
    
}

void DrawUpDownMenu(UP_DOWN_MENU *menu, char *title, char *remark, const char *fmt, ...)
{
    va_list ap;
    char buff[20];
    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    va_end(ap);
            
    SetDefaultButtonStyleArray(menu->btn,2);
    SetDefaultButtonStyle(&menu->btnMessage);
    
    uint16 yPos = DrawTitleButton(title,FALSE);
    EraseBlankArea(yPos,FALSE);
    
    BUTTON *leftBtn = &menu->btn[0];
    BUTTON *rightBtn = &menu->btn[1];

    SetButtonSize(leftBtn, 0,g_DEFAULT_BUTTON_HEIGHT,BUTTON_X_1_2,BUTTON_Y_HALF,FALSE);
    SetButtonSize(rightBtn,0,g_DEFAULT_BUTTON_HEIGHT,BUTTON_X_2_2,BUTTON_Y_HALF,FALSE); 
            
    SetButtonText(&menu->btnMessage,buff);
    DrawButton(&menu->btnMessage);
    
    BUTTON remarkBtn;
    SetDefaultButtonStyle(&remarkBtn);
    SetButtonStyleColor(&remarkBtn,BUTTON_STYLE_WHITE);
    SetButtonSize(&remarkBtn,menu->btnMessage.rect.bottom+2,g_DEFAULT_BUTTON_HEIGHT,BUTTON_X_FULL,BUTTON_Y,FALSE);

    SetButtonText(&remarkBtn,remark);
    DrawButton(&remarkBtn);
}

void SetDrawMonitoringMenu(MONITORING_MENU *menu, const char *title, ...)
{
    va_list ap;
    char buff[20];
    va_start(ap, title);
    vsprintf(buff, title, ap);
    va_end(ap);
    
    SetDefaultButtonStyleArray(menu->btn,7);
    
    BUTTON *btn;
    
    // title
    btn = &menu->btn[0];
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT-3,BUTTON_X_FULL,BUTTON_Y_FIRST,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_ORANGE);        
    SetButtonText(btn, buff);
    DrawButton(btn);
    
    EraseBlankArea(btn->rect.bottom+1,FALSE);    
    
    // first 
    btn = &menu->btn[1];
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_2, BUTTON_Y_2_4,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);
    btn->align = TEXT_ALIGN_LEFT;
    //SetButtonText(btn,first);    
    //DrawButton(btn);
    
    // first value    
    btn = &menu->btn[2];
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_2, BUTTON_Y_2_4,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_YELLOW);
    btn->align = TEXT_ALIGN_RIGHT;
   // SetButtonText(btn,"9999");    
   // DrawButton(btn);
    
    // second 
    btn = &menu->btn[3];
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_2, BUTTON_Y_3_4,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);
    btn->align = TEXT_ALIGN_LEFT;
    //SetButtonText(btn,second);    
    //DrawButton(btn);
    
    // second value    
    btn = &menu->btn[4];
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_2, BUTTON_Y_3_4,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_YELLOW);
    btn->align = TEXT_ALIGN_RIGHT;
  //  SetButtonText(btn,"9999");    
  //  DrawButton(btn);
    
    // third 
    btn = &menu->btn[5];
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_2, BUTTON_Y_4_4,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);
    btn->align = TEXT_ALIGN_LEFT;
    //SetButtonText(btn,third);    
    //DrawButton(btn);
    
    // third value    
    btn = &menu->btn[6];
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_2, BUTTON_Y_4_4,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_YELLOW);
    btn->align = TEXT_ALIGN_RIGHT;
 //   SetButtonText(btn,"999.9");    
  //  DrawButton(btn);    
}

void EraseBlankArea(uint16 y,uint8 withScroll)
{
    uint16 width = g_SCREEN_WIDTH;
    
    if(withScroll == TRUE) width -= 40;
    
    RECT blank;
    blank.left = 0;
    blank.right = width-1;
    blank.top = y;
    blank.bottom = GetBodyArea().bottom;
    
    if(blank.bottom-blank.top > 1) FillRect(&blank,BLACK);        
}

void EraseBlankAreaWithoutHeader()
{
    RECT blank;
    blank.left = 0;
    blank.right = g_SCREEN_WIDTH-1;
    blank.top = GetBodyArea().top;;
    blank.bottom = g_SCREEN_HEIGHT-1;
    
    FillRect(&blank,BLACK);     
}
 
void SetDrawUpDownTwoMenu(UP_DOWN_TWO_MENU *menu, char *first, char *second, const char *title, ...)
{
    va_list ap;
    char buff[20];
    va_start(ap, title);
    vsprintf(buff, title, ap);
    va_end(ap);
    
    SetDefaultButtonStyleArray(menu->btn,7);
    
    BUTTON *btn;
    
    // title
    btn = &menu->btn[UPDOWNTWO_TITLE];
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT,BUTTON_X_FULL,BUTTON_Y_FIRST,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_ORANGE);        
    SetButtonText(btn, buff);
    DrawButton(btn);
    
    EraseBlankArea(btn->rect.bottom+1,FALSE);    
    
    // first 
    btn = &menu->btn[UPDOWNTWO_FIRST_TEXT];
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT+12, BUTTON_X_1_2, BUTTON_Y_2_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT+12, BUTTON_X_1_3, BUTTON_Y_2_3,FALSE);
        
    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);
    SetButtonText(btn,first);    
    //DrawButton(btn);
    
    // Second 
    btn = &menu->btn[UPDOWNTWO_SECOND_TEXT];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT+12, BUTTON_X_2_2, BUTTON_Y_2_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT+12, BUTTON_X_2_3, BUTTON_Y_2_3,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_DARKGREY);
    SetButtonText(btn,second);    
    //DrawButton(btn);
    
    // first value
    btn = &menu->btn[UPDOWNTWO_FIRST_VALUE];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_2, BUTTON_Y_3_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_3, BUTTON_Y_3_3,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_YELLOW);
    SetButtonText(btn,"2");    
    //DrawButton(btn);
    
    // Second value
    btn = &menu->btn[UPDOWNTWO_SECOND_VALUE];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_2, BUTTON_Y_3_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_3, BUTTON_Y_3_3,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_DARKGREY);
    SetButtonText(btn,"2.5");    
    //DrawButton(btn);
    
    // Up
    btn = &menu->btn[UPDOWNTWO_UP_BUTTON];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_2, BUTTON_Y_4_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_3_3, BUTTON_Y_2_3,FALSE);
    RectangleSizeShrink(&btn->rect,+10,-10,-2,+2);
    SetButtonStyleColor(btn,BUTTON_STYLE_R_BLUE);
    SetButtonText(btn,"");    
    btn->image = &g_imageUpArrow;  
    DrawButton(btn);
    
    // Down
    btn = &menu->btn[UPDOWNTWO_DOWN_BUTTON];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_2, BUTTON_Y_4_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_3_3, BUTTON_Y_3_3,FALSE);
    RectangleSizeShrink(&btn->rect,+10,-10,-2,+2);    
    SetButtonStyleColor(btn,BUTTON_STYLE_R_RED);
    btn->image = &g_imageDownArrow;    
    SetButtonText(btn,"");    
    DrawButton(btn);      
}
/*
void SetDrawUpDownOneMenu(UP_DOWN_ONE_MENU *menu, char *first, const char *title, ...)
{
    va_list ap;
    char buff[20];
    va_start(ap, title);
    vsprintf(buff, title, ap);
    va_end(ap);
    
    SetDefaultButtonStyleArray(menu->btn,8);
    
    BUTTON *btn;
    
    // title
    btn = &menu->btn[0];
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT,BUTTON_X_FULL,BUTTON_Y_FIRST,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_ORANGE);        
    SetButtonText(btn, buff);
    DrawButton(btn);
    
    EraseBlankArea(btn->rect.bottom+1,FALSE);  
    
    // text
    btn = &menu->btn[1];
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_2, BUTTON_Y_2_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_3, BUTTON_Y_2_3,FALSE);  

    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);        
    SetButtonText(btn, first);
    btn->align = TEXT_ALIGN_LEFT;    
    DrawButton(btn);
    
    // value
    btn = &menu->btn[2];
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_2, BUTTON_Y_2_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_3, BUTTON_Y_2_3,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_YELLOW);     
  //  SetButtonText(btn, "9999");
    btn->align = TEXT_ALIGN_RIGHT;
    //DrawButton(btn);
    
    // Up
    btn = &menu->btn[3];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_2, BUTTON_Y_3_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_4_4, BUTTON_Y_2_3,FALSE); 
    RectangleSizeShrink(&btn->rect,+10,-10,-2,+2);
    SetButtonStyleColor(btn,BUTTON_STYLE_R_BLUE);
    SetButtonText(btn,"");    
    btn->image = &g_imageUpArrow;  
    DrawButton(btn);
    
    // Down
    btn = &menu->btn[4];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_2, BUTTON_Y_3_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_4_4, BUTTON_Y_3_3,FALSE);
    RectangleSizeShrink(&btn->rect,+10,-10,-2,+2);    
    SetButtonStyleColor(btn,BUTTON_STYLE_R_RED);
    btn->image = &g_imageDownArrow;    
    SetButtonText(btn,"");    
    DrawButton(btn);
    
    // Incre 1
    btn = &menu->btn[5];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_3, BUTTON_Y_4_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_4, BUTTON_Y_3_3,FALSE);
        
    RectangleSizeShrink(&btn->rect,+10,-10,0,0);
    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);
    SetButtonText(btn,"%d", menu->inc[0]);
    btn->outlineColor = DARKGREY;
    //DrawButton(btn);
    
    // Incre 2
    btn = &menu->btn[6];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_3, BUTTON_Y_4_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_4, BUTTON_Y_3_3,FALSE);
        
    RectangleSizeShrink(&btn->rect,+10,-10,0,0);
    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);
    SetButtonText(btn,"%d", menu->inc[1]); 
    btn->outlineColor = DARKGREY;
    //DrawButton(btn);
    
    // Incre 3
    btn = &menu->btn[7];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_3_3, BUTTON_Y_4_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_3_4, BUTTON_Y_3_3,FALSE);
        
    RectangleSizeShrink(&btn->rect,+10,-10,0,0);
    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);
    SetButtonText(btn,"%d", menu->inc[2]);
    btn->outlineColor = DARKGREY;
   // DrawButton(btn);
}
*/

// increase 숫자가 2만 있는 것
void SetDrawUpDownOneMenu(UP_DOWN_ONE_MENU *menu, char *first, const char *title, ...)
{
    va_list ap;
    char buff[20];
    va_start(ap, title);
    vsprintf(buff, title, ap);
    va_end(ap);
    
    SetDefaultButtonStyleArray(menu->btn,8);
    
    BUTTON *btn;
    
    // title
    btn = &menu->btn[0];
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT,BUTTON_X_FULL,BUTTON_Y_FIRST,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_ORANGE);        
    SetButtonText(btn, buff);
    DrawButton(btn);
    
    EraseBlankArea(btn->rect.bottom+1,FALSE);  
    
    // text
    btn = &menu->btn[1];
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_2, BUTTON_Y_2_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_3, BUTTON_Y_2_3,FALSE);  

    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);        
    SetButtonText(btn, first);
    btn->align = TEXT_ALIGN_LEFT;    
    DrawButton(btn);
    
    // value
    btn = &menu->btn[2];
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_2, BUTTON_Y_2_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_3, BUTTON_Y_2_3,FALSE);
    SetButtonStyleColor(btn,BUTTON_STYLE_YELLOW);     
  //  SetButtonText(btn, "9999");
    btn->align = TEXT_ALIGN_RIGHT;
    //DrawButton(btn);
    
    // Up
    btn = &menu->btn[3];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_2, BUTTON_Y_3_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_4_4, BUTTON_Y_2_3,FALSE); 
    RectangleSizeShrink(&btn->rect,+10,-10,-2,+2);
    SetButtonStyleColor(btn,BUTTON_STYLE_R_BLUE);
    SetButtonText(btn,"");    
    btn->image = &g_imageUpArrow;  
    DrawButton(btn);
    
    // Down
    btn = &menu->btn[4];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_2, BUTTON_Y_3_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_4_4, BUTTON_Y_3_3,FALSE);
    RectangleSizeShrink(&btn->rect,+10,-10,-2,+2);    
    SetButtonStyleColor(btn,BUTTON_STYLE_R_RED);
    btn->image = &g_imageDownArrow;    
    SetButtonText(btn,"");    
    DrawButton(btn);
    
    // Incre 1
    btn = &menu->btn[5];
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_3, BUTTON_Y_4_4,FALSE);
    else
        SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_4, BUTTON_Y_3_3,FALSE);
        
    RectangleSizeShrink(&btn->rect,+10,-10,0,0);
    
    if(menu->inc[2] == 0) RectangleSizeShrink(&btn->rect,0, 20,0,0);   
    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);
    SetButtonText(btn,"%d", menu->inc[0]);
    btn->outlineColor = DARKGREY;
    //DrawButton(btn);
    
    // Incre 2
    btn = &menu->btn[6];
    
    if(menu->inc[2] != 0)
    {    
        if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
            SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_3, BUTTON_Y_4_4,FALSE);
        else
            SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_2_4, BUTTON_Y_3_3,FALSE);
            
        RectangleSizeShrink(&btn->rect,+10,-10,0,0);            
    }
    else
    {
        if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
            SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_3_3, BUTTON_Y_4_4,FALSE);
        else
            SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_3_4, BUTTON_Y_3_3,FALSE);  
            
        RectangleSizeShrink(&btn->rect,-10,-10,0,0);            
    }

    SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);
    SetButtonText(btn,"%d", menu->inc[1]); 
    btn->outlineColor = DARKGREY;
    //DrawButton(btn);
    
    // Incre 3
    btn = &menu->btn[7];
    if(menu->inc[2] != 0)
    {
        btn->ghost = FALSE;
        
        if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
            SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_3_3, BUTTON_Y_4_4,FALSE);
        else
            SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_3_4, BUTTON_Y_3_3,FALSE);
            
        RectangleSizeShrink(&btn->rect,+10,-10,0,0);
        SetButtonStyleColor(btn,BUTTON_STYLE_WHITE);
        SetButtonText(btn,"%d", menu->inc[2]);
        btn->outlineColor = DARKGREY;
    }
    else
    {    
        btn->ghost = TRUE;
    }        
    //DrawButton(btn);
}


void SetDrawYesNoButtons(YES_NO_MENU *menu, const char *message, ...)
{
    va_list ap;
    char buff[20];
    va_start(ap, message);
    vsprintf(buff, message, ap);
    va_end(ap);
    
    uint16 ypos = DrawTitleButton(menu->title,FALSE);      
    EraseBlankArea(ypos, FALSE);
    
    BUTTON button;
    SetButtonSize(&button,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_3_4,FALSE);
    SetButtonStyleColor(&button,BUTTON_STYLE_TEXT);
    SetButtonText(&button,message);
    
    DrawButton(&button);
}

void ShowMessage(const char *msg, ...)
{
    va_list ap;
    char buff[64];  // 버퍼 크기 증가: 20 → 64
    va_start(ap, msg);
    vsprintf(buff, msg, ap);
    va_end(ap);

    BUTTON btn;
    SetDefaultButtonStyle(&btn);
    SetButtonSize(&btn,0,g_DEFAULT_BUTTON_HEIGHT*2, BUTTON_X_FULL, BUTTON_Y_2_3,FALSE);
    RectangleSizeShrink(&btn.rect,+10,-10,0,0);
    SetButtonStyleColor(&btn,BUTTON_STYLE_WHITE);
    SetButtonText(&btn,buff);
    btn.outlineColor = WHITE;

    DrawButton(&btn);
}

void ShowWaitMessage()
{
   ShowMessage("Waiting...");
}

void SetTitleBarText(char *str)
{
    SetButtonText(&g_TitleBar,str);
}

void DrawTitle()
{
    DrawButton(&g_TitleBar);    
}

void DrawHeader()
{
    // DrawWifi();  // WiFi 기능 비활성화

    DrawTitle();

    // split line
    DrawHorizontalLine(0,g_SCREEN_WIDTH-1,DEFAULT_TOP_TITLE_HEIGHT-1,WHITE);
}


/* [] END OF FILE */
