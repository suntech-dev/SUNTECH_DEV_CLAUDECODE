/* ========================================
 *
 * Copyright Suntech, 2023.04.01
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "andonMenu.h"
#include "userMenu.h"
#include "lib/widget.h"
#include "lib/image.h"
#include "lib/internalFlash.h"
#include "lib/externalFlash.h"
#include "menuDesign.h"
#include "andonApi.h"
#include "package.h"
#include "WIFI.h"
#include "count.h"
#include "userProjectPatternSewing.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetAndonListButtons(LIST_MENU *menu, char *title, ANDON_LISTS *list)
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
   
    menu->sizeOfList = list->noOfList;
    strcpy(menu->title,title);

    uint8 withScroll = isExistScroll(menu);
    
    if((yPosTitle = DrawTitleButton(title,withScroll)) > 0) yPos = yPosTitle;
      
    for(int i=0; i < menu->noOfDisplayButton; i++)
    {                        
        BUTTON *btn = &menu->btns[i];
        SetDefaultButtonStyle(btn);
        
        SetButtonSize      (btn, yPos, g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y,withScroll);
        
        uint16 textIdx = menu->noOfDisplayButton * menu->curPage + i;
        if(textIdx >= menu->sizeOfList) break;
     
        ANDON_LIST_ITEM *item = &list->item[textIdx];
        int16 not_completed_qty = item->not_completed_qty;
        
        if(not_completed_qty == 0)
            SetButtonStyleColor(btn, BUTTON_STYLE_LIST); 
        else
            ////SetButtonStyleColor(btn, BUTTON_STYLE_R_ORANGE); 
            SetButtonStyleColor(btn, BUTTON_STYLE_R_RED);
            
        btn->align = TEXT_ALIGN_LEFT;
            
        if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
        {
            sprintf(menu->listText[i],"%-10s %2d",item->text, item->not_completed_qty);
        }
        else
        {
            sprintf(menu->listText[i],"%-15s %2d",item->text, item->not_completed_qty);
        }
                
        yPos += g_DEFAULT_BUTTON_HEIGHT;
    }

    if(withScroll) SetDrawScrollButton(menu);
}

char * updateDateTime(uint8 reflash)
{
    static uint32 uOldtSeconds = 0, bToggle=TRUE;
    static char strTime[10];
    uint32 time = RTC_GetTime();
    
    uint32 uSeconds = RTC_GetSecond(time);
        
    if(uOldtSeconds != uSeconds || reflash == TRUE)
    {    
        uOldtSeconds = uSeconds;        
  
        if(bToggle) 
        {
            bToggle = FALSE;
            sprintf(strTime, "%02lu:%02lu", RTC_GetHours(time), RTC_GetMinutes(time)); 
        }
        else
        {
            bToggle = TRUE;
            sprintf(strTime, "%02lu %02lu", RTC_GetHours(time), RTC_GetMinutes(time)); 
        }
        return strTime;
    }
    return NULL;
}

int doAndonMenu(void *this, uint8 reflash)
{   
    static uint16 page = 0;
    char *strTime = updateDateTime(reflash);
    
   // static LIST_MENU menu;
    
    if(g_AndonLists.bUpdated || reflash == TRUE)
    {
        SetAndonListButtons(&g_ListMenu, NULL, &g_AndonLists);
        UpdateDrawListButtons(&g_ListMenu,NO_SELECT);
        g_AndonLists.bUpdated = FALSE;
    }
    
    if(strTime != NULL)
    {
        BUTTON *rightButton = &g_btnBottom[1];
        SetButtonText(rightButton,strTime);
        DrawButton(rightButton);
    }
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
        {
           SetDrawBottomButtons("QUIT", strTime, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_WHITE); 
           getCount()->andonEntry = TRUE;
           SaveInternalFlash();
           g_ListMenu.curPage = page;
         //  SaveInternalFlash();
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
                                   getCount()->andonEntry = FALSE;
                                   SaveInternalFlash();
                                page = g_ListMenu.curPage;
                                return MENU_RETURN_PARENT;
                            //  case BOTTOM_RIGHT: return 1;                            
                            }
                            break;
                        }
                    default: 
                        if(idx < g_ListMenu.sizeOfList) 
                        {
                            g_AndonLists.uSelectIndex = idx;
                            return 0;
                        }
                }        
            }
            break;
    }
        
    return MENU_RETURN_THIS;    
}

int doAndonRequest(void *this, uint8 reflash)
{
    MENUNODE *thisMenu = (MENUNODE *) this;    
    static LIST_MENU menu;
    static char *listText[] = {"WARNING", "COMPLETE"};
    static uint16 indexSelect;
    uint8 bUpdate = FALSE;
   
    ANDON_LIST_ITEM *item = &g_AndonLists.item[g_AndonLists.uSelectIndex];    
    int16 not_completed_qty = item->not_completed_qty;
    if(not_completed_qty == 0) indexSelect = 0;
                
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                menu.curPage = 0;
                bUpdate = TRUE;
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                
                uint16 idx = getIndexOfClickedListMenu(&tc, &menu);

                switch(idx)
                {
                    case NO_CLICK :
                        {
                            switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                            {
                                case BOTTOM_LEFT:                        
                                    return MENU_RETURN_PARENT;
                                case BOTTOM_RIGHT: 
                                {
                                    switch(indexSelect)
                                    {
                                        case 0: makeAndonWarningRequest(item->idx);
                                        break;
                                        case 1: makeAndonCompleteRequest(item->idx);
                                        break;
                                    }
                                    return MENU_RETURN_PARENT;                                
                                }                            
                            }
                            break;
                        }
                    case 0:
                        indexSelect = 0;
                        bUpdate = TRUE;
                        break;
                    case 1:
                        indexSelect = 1;                        
                        bUpdate = TRUE;                     
                        break;
                }                
            }
            break;
    }
    if(bUpdate)
    {
        char msg[MAX_BUTTON_TEXT_SIZE];
        sprintf(msg,"%s:#%d",item->text,not_completed_qty);
        SetDrawListButtons(&menu,msg,listText,not_completed_qty ==0 ? 1: 2 , BUTTON_STYLE_LIST);                    
        UpdateDrawListButtons(&menu,indexSelect);
        SetDrawBottomButtons("QUIT", "SEND", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
        bUpdate = FALSE;
    }
    return MENU_RETURN_THIS;    
}

MENUNODE * andonMenuCreate(MENUNODE *parent)
{
    // Main Menu  ............................................
   MENUNODE *menuMain                  = createMENUNODE(parent,    "ANDON",     &doAndonMenu);
             MENUNODE *factoryReset    = createMENUNODE(menuMain,  "REQUEST",   &doAndonRequest);
   return menuMain;
}


int doAndonSet(void *this, uint8 reflash) ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;    

    static char *listText[] = {"DISABLE", "ENABLE"};
    static uint16 indexSelect = 0;
    static uint16 page = 0;
    
    if(reflash)
    {
        if(g_ptrMachineParameter->andon_enable)
            indexSelect = 1;
        else                       
            indexSelect = 0;
    }
    
    switch(doSelectList(this, reflash, listText, 2, &indexSelect, &page))
    {
        case 0:
        case 1:
            doSelectList(this, TRUE, listText, 2, &indexSelect, &page);
            break; 
        case MENU_BOTTOM_LEFT: // QUIT
        {
            return MENU_RETURN_PARENT;
        }
        case MENU_BOTTOM_RIGHT: // SAVE
        {
            if(indexSelect == 0)
                g_ptrMachineParameter->andon_enable = FALSE;
            else
                 g_ptrMachineParameter->andon_enable = TRUE;
            ShowWaitMessage();                                    
            SaveExternalFlashConfig();
            CySoftwareReset();
        }            
    }
    return MENU_RETURN_THIS;    
}
/* [] END OF FILE */
