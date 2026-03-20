
#include "widget.h"
#include "menuDesign.h"
#include "image.h"
#include "externalFlash.h"
#include "UI.h"
#include "WIFI.h"
#include "server.h"
#include "LEDControl.h"
#include "../otaMenu.h"   /* OTA 업데이트 배지 */

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

IMAGE g_imageWifi =
{
    .image = image_wifi_4,
    .width = 32,
    .height=32,
    //.width = 0,
    //.height=0,
};

RECT             g_rectWifi;
BUTTON           g_wifi_strength;
BUTTON           g_TitleBar;
LIST_MENU        g_ListMenu;
YES_NO_MENU      g_YesNoMenu;
UP_DOWN_ONE_MENU g_IncreseNumberMenu;
IMAGE            g_DownArrowImage;

void initWidget()
{
    /* wifi Strength */
    g_wifi_strength.rect.top      = 0;
    g_wifi_strength.rect.bottom   =  DEFAULT_TOP_TITLE_HEIGHT-2;
    g_wifi_strength.rect.right = g_SCREEN_WIDTH-1;
    g_wifi_strength.rect.left  = g_wifi_strength.rect.right - 30;
    SetDefaultButtonStyle(&g_wifi_strength);    
    SetButtonStyleColor(&g_wifi_strength,BUTTON_STYLE_TEXT);
        
    /* wifi Icon */
    g_rectWifi.right  = g_wifi_strength.rect.left-1;
    g_rectWifi.left   = g_rectWifi.right - g_imageWifi.width;
    g_rectWifi.top    = g_wifi_strength.rect.top;
    g_rectWifi.bottom = g_wifi_strength.rect.bottom;
    
    /* title Bar */
    g_TitleBar.rect.right  = g_rectWifi.left;
    g_TitleBar.rect.left   = 0;
    g_TitleBar.rect.top    = 0;
    g_TitleBar.rect.bottom = DEFAULT_TOP_TITLE_HEIGHT-2;
    
    SetButtonStyleColor(&g_TitleBar,BUTTON_STYLE_WHITE);
    SetButtonText(&g_TitleBar,"%s",g_ptrServer->deviceName);
    //SetButtonText(&g_TitleBar,"KVC-3 COUNTER");
        
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
void SetDrawListButtons(LIST_MENU *menu, char *title, char *textArray[], uint16 sizeOfList, uint8 buttonColor)
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

    uint8 withScroll = isExistScroll(menu);
    
    if((yPosTitle = DrawTitleButton(title,withScroll)) > 0) yPos = yPosTitle;
      
    if(textArray != NULL)
        for(int i=0; i < menu->noOfDisplayButton; i++)
        {                        
            BUTTON *btn = &menu->btns[i];
            SetDefaultButtonStyle(btn);     
            if(buttonColor != BUTTON_STYLE_NONE)
                 SetButtonStyleColor(btn, buttonColor);  
            
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
void UpdateDrawListButtons(LIST_MENU *menu, uint16 indexSelect)
{
    RECT bodyRect = GetBodyArea();
    uint16 yPos = bodyRect.top;
        
    for(int i=0; i < menu->noOfDisplayButton; i++)
    {                        
        int j = i+menu->curPage*menu->noOfDisplayButton;
        if(j >= menu->sizeOfList) break;

        BUTTON *btn = &menu->btns[i];

        if(indexSelect != NO_SELECT)
        {
            char buff[MAX_BUTTON_TEXT_SIZE];
            sprintf(buff,"%c%s",(indexSelect==j) ? '>' : ' ',menu->listText[j]);
            strcpy(btn->text,buff);
        } else {
           strcpy(btn->text,menu->listText[j]); 
        }
        
        DrawButton(btn);
        yPos = btn->rect.bottom + 1;
    }
    
    EraseBlankArea(yPos, isExistScroll(menu));
}

uint8 isFirtstPage(LIST_MENU *menu) { return (menu->curPage == 0 ) ? TRUE : FALSE; }
uint8 isLastPage(LIST_MENU *menu)    { return (menu->curPage == (menu->sizeOfList-1) / menu->noOfDisplayButton) ? TRUE : FALSE; }

uint16 getIndexOfClickedListMenu(TOUCH *tc, LIST_MENU *menu)
{
    int8 bIsFirstPage = isFirtstPage(menu); // (menu->curPage == 0 ) ? TRUE : FALSE;
    int8 bIsLastPage  = isLastPage(menu);   //(menu->curPage == (menu->sizeOfList-1) / menu->noOfDisplayButton) ? TRUE : FALSE;
        
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
            //    printf("Start Page : %d\r\n",page);
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
                                case BOTTOM_LEFT:
                           //         page = g_ListMenu.curPage;
                                    return MENU_RETURN_PARENT;                         
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

int doListMenuPage(void *this, uint8 reflash, uint16 *ptrPage) //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;     
    static uint16 noOfList;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                noOfList = getNoOfChild(thisMenu);
                g_ListMenu.curPage = *ptrPage;
                SetDrawNodeListButtons(thisMenu, &g_ListMenu);
                UpdateDrawListButtons(&g_ListMenu,NO_SELECT);
                SetDrawBottomButtons("QUIT", NULL, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE); 
            //    printf("Start Page : %d\r\n",page);
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
                                case BOTTOM_LEFT:
                                    *ptrPage = g_ListMenu.curPage;
                                    return MENU_RETURN_PARENT;                         
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

/*
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
            //    printf("Start Page : %d\r\n",page);
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
                                case BOTTOM_LEFT:
                           //         page = g_ListMenu.curPage;
                                    return MENU_RETURN_PARENT;                         
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
*/

/////////////////////////////////////////////////////////////////////////////////////////////////
int doSelectList(void *this, uint8 reflash, char *listText[], uint16 listSize, uint16 *ptr_uSelect, uint16 *ptrPage)
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;    
   
    uint8 bUpdate = FALSE;
    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                g_ListMenu.curPage = *ptrPage;
                bUpdate = TRUE;
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                
                uint16 idx = getIndexOfClickedListMenu(&tc, &g_ListMenu);

                switch(idx)
                {
                    case NO_CLICK :
                        {
                            switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                            {
                                case BOTTOM_LEFT:
                                {
                                    *ptrPage = g_ListMenu.curPage;
                                    return MENU_BOTTOM_LEFT;    
                                }
                                case BOTTOM_RIGHT: 
                                {
                                    *ptrPage = g_ListMenu.curPage;
                                    return MENU_BOTTOM_RIGHT;                                
                                }                            
                            }
                            break;
                        }
                    default:
                        *ptr_uSelect = idx;
                        return idx;                   
                }                
            }
            break;
    }
    if(bUpdate)
    {
        SetDrawListButtons(&g_ListMenu,thisMenu->nodeName,listText,listSize, BUTTON_STYLE_LIST);                    
        UpdateDrawListButtons(&g_ListMenu,*ptr_uSelect);
        SetDrawBottomButtons("QUIT", "SAVE", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
        bUpdate = FALSE;
    }
    return MENU_RETURN_THIS;    
}


void DisplayIncreseNumberMenuValueChange(UP_DOWN_ONE_MENU *menu, uint16 value)
{
    SetButtonText(&menu->btn[2],"%d",value);
    DrawButton(&menu->btn[2]);
}

void DisplayIncreseNumberMenuSelectValue(UP_DOWN_ONE_MENU *menu, uint8 selectIncre, uint16 value)
{
    menu->btn[5].foregroundColor = ((selectIncre==0) ? ORANGE : DARKGREY);  // 1 incre
    menu->btn[6].foregroundColor = ((selectIncre==1) ? ORANGE : DARKGREY);  // 2 incre  
    menu->btn[7].foregroundColor = ((selectIncre==2) ? ORANGE : DARKGREY);  // 3 incre  

    DrawButton(&menu->btn[5]);
    DrawButton(&menu->btn[6]);
    DrawButton(&menu->btn[7]);
    
    DisplayIncreseNumberMenuValueChange(menu,value);    
}

void DisplayIncreseNumberMenu(UP_DOWN_ONE_MENU *menu, int32 *incr, char *strTitle, char *strValue)
{
    for(int i=0; i < 3; i++)
    {
        menu->inc[i] = incr[i];
        menu->btn[i].disable = TRUE; // for no click title
    }
    
    SetDrawUpDownOneMenu(menu, strValue, strTitle);
    
    SetDrawBottomButtons("QUIT", "SAVE", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
}


int doIncreseNumberMenu(uint8 reflash, char *strTitle, char *strValue, int32 *incr, int32 *value,  int32 minValue, int32 maxValue, uint16 *selectIncre)
{    
    static uint16 tmp;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {           
                DisplayIncreseNumberMenu(&g_IncreseNumberMenu, incr, strTitle, strValue);
                DisplayIncreseNumberMenuSelectValue(&g_IncreseNumberMenu,  *selectIncre, *value);
            }
            break;  
        
        case FALSE: // Cliking Check
            {

                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                uint16 indexClick = getIndexOfClickedButton(&tc, g_IncreseNumberMenu.btn,8);
                switch(indexClick)
                {                  
                    case 3: // for up
                        if      ((*value+g_IncreseNumberMenu.inc[*selectIncre]) < maxValue)
                            *value+=g_IncreseNumberMenu.inc[*selectIncre];
                        else
                            *value = maxValue;
                        
                        DisplayIncreseNumberMenuValueChange(&g_IncreseNumberMenu,*value);
                        break;                       
                    case 4: // for down
                        if      ((*value-g_IncreseNumberMenu.inc[*selectIncre]) > minValue)
                            *value-=g_IncreseNumberMenu.inc[*selectIncre];
                        else
                            *value = minValue;
                        DisplayIncreseNumberMenuValueChange(&g_IncreseNumberMenu,*value);                        
                        break;                        
                    case 5:
                        *selectIncre = 0;
                        DisplayIncreseNumberMenuSelectValue(&g_IncreseNumberMenu,*selectIncre, *value);
                        break;
                    case 6:
                        *selectIncre = 1;
                        DisplayIncreseNumberMenuSelectValue(&g_IncreseNumberMenu,*selectIncre, *value);
                        break;
                    case 7:
                        *selectIncre = 2;
                        DisplayIncreseNumberMenuSelectValue(&g_IncreseNumberMenu, *selectIncre, *value);
                        break;                        
                    default:
                        switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                        {
                            case BOTTOM_LEFT:  return MENU_BOTTOM_LEFT;
                            case BOTTOM_RIGHT: return MENU_BOTTOM_RIGHT;                  
                        }
                    break;
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
    menu->downScroll.rect.bottom   = r.bottom - 5;
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

void ShowThreeMessage(THREE_MESSAGE *menu, char *msg1, char *msg2, char *msg3)
{
    uint16 ypos = DrawTitleButton(menu->title,FALSE);      
    EraseBlankArea(ypos, FALSE);
    
    BUTTON button1;
    SetButtonStyleColor(&button1,BUTTON_STYLE_R_ORANGE);   
    SetButtonSize(&button1,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_2_4,FALSE);
    SetButtonText(&button1,msg1);
    DrawButton(&button1);    
    
    BUTTON button2;    
    SetButtonStyleColor(&button2,BUTTON_STYLE_TEXT);       
    SetButtonSize(&button2,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_3_4,FALSE);
    SetButtonText(&button2,msg2);
    DrawButton(&button2);

    BUTTON button3;      
    SetButtonStyleColor(&button3,BUTTON_STYLE_TEXT);       
    SetButtonSize(&button3,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_4_4,FALSE);
    SetButtonText(&button3,msg3);
    DrawButton(&button3);   
}

void ShowMessage(const char *msg, ...)
{
    va_list ap;
    char buff[20];
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

void DrawWifi()
{     
    uint8 uLevel = 0;

    if     (g_network.RSSI == 0)   uLevel=0; 
    else if(-30 <= g_network.RSSI) uLevel=5;
    else if(-67 <= g_network.RSSI) uLevel=4;
    else if(-70 <= g_network.RSSI) uLevel=3;
    else if(-80 <= g_network.RSSI) uLevel=2;
    else if(-90 <= g_network.RSSI) uLevel=1;

    switch(uLevel)
    {
        case 0:
            SetButtonText(&g_wifi_strength,"");
            g_wifi_strength.foregroundColor = WHITE;
            DrawButton(&g_wifi_strength);
            g_imageWifi.image = image_wifi_0;
        
            break;
        default:
            SetButtonText(&g_wifi_strength,"%d",uLevel);
            switch(uLevel)
            {
                case 0:
                case 1: g_wifi_strength.foregroundColor = RED;   
                        g_uLED1_Color = LED_RED;
                        break;
                case 2: g_wifi_strength.foregroundColor = ORANGE;
                        g_uLED1_Color = LED_PINK;                
                        break;
                case 3: g_wifi_strength.foregroundColor = YELLOW;
                         g_uLED1_Color = LED_YELLOW;    
                        break;
                case 4: g_wifi_strength.foregroundColor = GREEN;
                         g_uLED1_Color = LED_GREEN;   
                        break;
                case 5: g_wifi_strength.foregroundColor = BLUE;
                         g_uLED1_Color = LED_BLUE;   
                        break;             
            }
            DrawButton(&g_wifi_strength);
            g_imageWifi.image = image_wifi_4;  
            break;            
    }
 
    DrawImageRect(&g_rectWifi,&g_imageWifi,BLACK);

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
    DrawWifi();

    DrawTitle();

    otaDrawUpdateBadge(); /* OTA 업데이트 있을 때 헤더에 배지 표시 */

    // split line
    DrawHorizontalLine(0,g_SCREEN_WIDTH-1,DEFAULT_TOP_TITLE_HEIGHT-1,WHITE);
}

/* [] END OF FILE */