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
#include "userMenu.h"
#include "lib/widget.h"
#include "lib/image.h"
#include "lib/externalFlash.h"
#include "lib/internalFlash.h"
#include "menuDesign.h"
#include "package.h"
#include "WIFI.h"
#include "count.h"
    
#ifdef USER_PROJECT_TRIM_COUNT
    
#include "lib/manageMenu.h"
#include "userProjectCounter.h"
#include "count.h"
    
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doTopMenu   ////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayDoTopMenu(MONITORING_MENU *menu);
void TopMenuPageUpdate(MONITORING_MENU *menu, uint8 page);
void TopMenuPageUpdateValue(MONITORING_MENU *menu, uint8 page);

int doTopMenu(void *this, uint8 reflash)
{    
    MENUNODE *thisMenu = (MENUNODE *) this;    
    static MONITORING_MENU menu;
    static uint8 patternPage = 0;
    static uint8 sewingPage = 0;
    static uint8 page = 0, oldPage = 0xFF;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                if(g_ptrMachineParameter->setTrimCount < g_ptrCount->count)
                {
                    g_ptrCount->count = g_ptrMachineParameter->setTrimCount;
                }
                                
                g_updateCountMenu = TRUE;                
                DisplayDoTopMenu(&menu);                                        
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                switch(getIndexOfClickedButton(&tc, menu.btn,7))
                {
                    case 0: 
                    
                    if(KEY_LOCK_Read() == FALSE)
                    {
                        return 0;
                    } else {
                        return 1;
                    }
                    
                    break;
                    
                }
 
            }
            break;
    }
    
    if(g_updateCountMenu)
    {
        SetButtonText(&menu.btn[1],"%d",g_ptrMachineParameter->setTrimCount);    
        SetButtonText(&menu.btn[2],"%d",g_ptrMachineParameter->setTrimCount-g_ptrCount->count);         
        DrawButton(&menu.btn[1]);
        DrawButton(&menu.btn[2]);
        g_updateCountMenu = FALSE;
    }
  //  TopMenuPageUpdate(&menu, page);
    
/*    
   if(oldPage != page || g_updateCountMenu == TRUE) TopMenuPageUpdate(&menu, page);
    
    oldPage = page;
    g_updateCountMenu = FALSE;
*/
    return MENU_RETURN_THIS;    
}

void TopMenuPageUpdate(MONITORING_MENU *menu, uint8 page)
{/*
    switch(g_ptrMachineParameter->machineType)
    {
        case PATTERN_MACHINE:
        {
            switch(page)
            {
                case 0:
                    SetButtonText(&menu->btn[1],g_patternString[0]);
                    SetButtonText(&menu->btn[3],g_patternString[1]);
                    SetButtonText(&menu->btn[5],g_patternString[2]);
                    break;
                case 1:
                    SetButtonText(&menu->btn[1],g_patternString[3]);
                    SetButtonText(&menu->btn[3],g_patternString[4]);
                    SetButtonText(&menu->btn[5],g_patternString[2]);                    
                    break;
                case 2:
                    SetButtonText(&menu->btn[1],g_patternString[5]);
                    SetButtonText(&menu->btn[3],g_patternString[6]);
                    SetButtonText(&menu->btn[5],g_patternString[7]);                      
                    break;
            }
        }
        break;
        case SEWING_MACHINE:
        {
            switch(page)
            {
                case 0:
                    SetButtonText(&menu->btn[1],g_patternString[0]);
                    SetButtonText(&menu->btn[3],g_patternString[1]);
                    SetButtonText(&menu->btn[5],g_patternString[2]);
                    break;
                case 1:
                    SetButtonText(&menu->btn[1],g_patternString[3]);
                    SetButtonText(&menu->btn[3],g_patternString[4]);
                    SetButtonText(&menu->btn[5],g_patternString[5]);                    
                    break;     
            }
        }
        break;        
    }
*/
  //  menu->btn[3].disable = TRUE; // for no click second line
  //  menu->btn[4].disable = TRUE; // for no click second line 
        
    DrawButton(&menu->btn[1]);
    DrawButton(&menu->btn[3]);
    DrawButton(&menu->btn[5]); 

    TopMenuPageUpdateValue(menu,page);
    
}

int setCount = 2;
int curCount = 1;

void DisplayDoTopMenu(MONITORING_MENU *menu)
{    
    RECT r = GetBodyArea();
    
    EraseBlankAreaWithoutHeader();
    
    BUTTON *btn = &menu->btn[0];    
    // SET //////////////////////////////////
    btn->image           = NULL;    
    btn->font            = Grotesk16x32;
    btn->backgroundColor = RED;
    btn->foregroundColor = WHITE;
    btn->outlineColor    = BLACK;        
    btn->align           = TEXT_ALIGN_CENTER;
    btn->marginLeftRight = 0;
    btn->marginTopBottom =  0;
    btn->show            = TRUE;
    btn->disable         = FALSE;
    btn->ghost           = FALSE;

    btn->rect.left = r.left;
    btn->rect.right =(r.right-r.left) / 3;
    btn->rect.top = r.top;
    btn->rect.bottom = r.bottom + g_DEFAULT_BUTTON_HEIGHT;    

    // Setting Trim //////////////////////////////////  
    BUTTON *setTrim = &menu->btn[1];    
    setTrim->image           = NULL;    
    setTrim->font            = ArialNumFontPlus32x50;
    setTrim->backgroundColor = BLACK;
    setTrim->foregroundColor = YELLOW;
    setTrim->outlineColor    = BLACK;        
    setTrim->align           = TEXT_ALIGN_RIGHT;
    setTrim->marginLeftRight = 10;
    setTrim->marginTopBottom =  0;
    setTrim->show            = TRUE;
    setTrim->disable         = FALSE;
    setTrim->ghost           = FALSE;
 
    setTrim->rect.left = btn->rect.right + 1;
    setTrim->rect.right = r.right - 10;
    setTrim->rect.top = r.top+ 12;
    setTrim->rect.bottom = (r.bottom + g_DEFAULT_BUTTON_HEIGHT)/2;    
    
    // Setting Trim //////////////////////////////////  
    BUTTON *curTrim = &menu->btn[2];       
    memcpy(curTrim,setTrim,sizeof(BUTTON));
    curTrim->foregroundColor = GREEN;
    curTrim->rect.top = setTrim->rect.bottom+1;
    curTrim->rect.bottom = r.bottom + g_DEFAULT_BUTTON_HEIGHT;          
       
    SetButtonText(btn,"SET");    
//    SetButtonText(setTrim,"%d",g_ptrMachineParameter->setTrimCount);    
//    SetButtonText(curTrim,"%d",g_ptrMachineParameter->curTrimCount); 
    
    DrawButton(&menu->btn[0]);
//    DrawButton(&menu->btn[1]);    
//    DrawButton(&menu->btn[2]);     
        
   //    SetDrawBottomButtons("MENU", "ANDON", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
}

void TopMenuPageUpdateValue(MONITORING_MENU *menu, uint8 page)
{
    BUTTON *btn;
    uint8 persent = FALSE;
    
    DrawButton(&menu->btn[2]);
    DrawButton(&menu->btn[4]);
    DrawButton(&menu->btn[6]);
    
    if( persent)
    {
        BUTTON *b = &menu->btn[6];
        LCD_DrawFont(b->rect.right-16,(b->rect.top+b->rect.bottom)*.5-10,b->foregroundColor,b->backgroundColor,b->font,'%');
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doUnlockWithKey   //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int doUnlockWithKey(void *this, uint8 reflash)
{    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                RECT r = GetBodyArea();                
                
                BUTTON button;
                BUTTON *btn = &button;
                
                btn->image           = NULL;    
                btn->font            = Grotesk16x32;
                btn->backgroundColor = BLACK;
                btn->foregroundColor = YELLOW;
                btn->outlineColor    = WHITE;        
                btn->align           = TEXT_ALIGN_CENTER;
                btn->marginLeftRight = 0;
                btn->marginTopBottom =  0;
                btn->show            = TRUE;
                btn->disable         = FALSE;
                btn->ghost           = FALSE;

                btn->rect.left =  r.left + 10;
                btn->rect.right = r.right-10;
                btn->rect.top = r.top + 10;
                btn->rect.bottom = r.bottom + g_DEFAULT_BUTTON_HEIGHT - 10;
                
                SetButtonText(btn, "Unlock with Key.");
                DrawButton(btn);
                
                Buzzer(BUZZER_WARNING, 3);
            }
            break;  

        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;   
                return MENU_RETURN_PARENT;
            }
            break;
    }
    return MENU_RETURN_THIS;    
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doTargetInfoMenu   //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayDoTargetInfoMenu(UP_DOWN_ONE_MENU *menu);
void DisplayDoTargetInfoMenuSelectValue(UP_DOWN_ONE_MENU *menu, uint8 selectIncre, uint16 value);
void DisplayDoTargetInfoMenuValueChange(UP_DOWN_ONE_MENU *menu, uint16 value);

int doTargetInfoMenu(void *this, uint8 reflash)
{    
    MENUNODE *thisMenu = (MENUNODE *) this;    
    static UP_DOWN_ONE_MENU menu;    
    static uint16 selectIncre = 0;
    static uint16 value = 9999;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {/*
                switch(g_ptrMachineParameter->machineType)
                {
                    case PATTERN_MACHINE: 
                        value  = g_ptrMachineParameter->patternTarget;
                        break;
                    case SEWING_MACHINE:
                        value  = g_ptrMachineParameter->sewingTarget;
                        break;
                    break;
                }    */     
                value = g_ptrMachineParameter->setTrimCount;
                DisplayDoTargetInfoMenu(&menu);
                DisplayDoTargetInfoMenuSelectValue(&menu, selectIncre, value);
            }
            break;  
        
        case FALSE: // Cliking Check
            {

                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                uint16 indexClick = getIndexOfClickedButton(&tc, menu.btn,8);
                switch(indexClick)
                {                  
                    case 3: // for up
                        if      (value+menu.inc[selectIncre] < 100)
                        {
                            value+=menu.inc[selectIncre];
                            DisplayDoTargetInfoMenuValueChange(&menu,value);
                        }
                        break;                       
                    case 4: // for down
                        if      (value-menu.inc[selectIncre] > 0)
                        {
                            value-=menu.inc[selectIncre];
                            DisplayDoTargetInfoMenuValueChange(&menu,value);
                        }
                        break;               
                        break;                        
                    case 5:
                        selectIncre = 0;
                        DisplayDoTargetInfoMenuSelectValue(&menu,selectIncre, value);
                        break;
                    case 6:
                        selectIncre = 1;
                        DisplayDoTargetInfoMenuSelectValue(&menu,selectIncre, value);
                        break;
                    case 7:
                        selectIncre = 2;
                        DisplayDoTargetInfoMenuSelectValue(&menu,selectIncre, value);
                        break;                        
                    default:
                        switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                        {
                            case BOTTOM_LEFT:  return MENU_RETURN_PARENT;
                   
                            case BOTTOM_RIGHT: 
                            {
                                g_ptrMachineParameter->setTrimCount = value;

                                ShowWaitMessage();
                                
                                if(g_ptrMachineParameter->setTrimCount < g_ptrCount->count)
                                {
                                    g_ptrCount->count = g_ptrMachineParameter->setTrimCount;
                                    SaveInternalFlash();
                                }
                                       
                                SaveExternalFlashConfig();
                           
                                return MENU_RETURN_PARENT;   
                            
                            }                     
                        }
                    break;
                }                                  
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

void DisplayDoTargetInfoMenu(UP_DOWN_ONE_MENU *menu)
{
    menu->inc[0] = 1;
    menu->inc[1] = 10;
    menu->inc[2] = 0;
    
    SetDrawUpDownOneMenu(menu, "TRIM", "Set Target");
    
    menu->btn[0].disable = TRUE; // for no click title
    menu->btn[1].disable = TRUE; // for no click title
    menu->btn[2].disable = TRUE; // for no click title
    
    SetDrawBottomButtons("EXIT", "SAVE", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
}

void DisplayDoTargetInfoMenuSelectValue(UP_DOWN_ONE_MENU *menu, uint8 selectIncre, uint16 value)
{
    menu->btn[5].foregroundColor = ((selectIncre==0) ? ORANGE : DARKGREY);  // 1 incre
    menu->btn[6].foregroundColor = ((selectIncre==1) ? ORANGE : DARKGREY);  // 2 incre  
//    menu->btn[7].foregroundColor = ((selectIncre==2) ? ORANGE : DARKGREY);  // 3 incre  

    DrawButton(&menu->btn[5]);
    DrawButton(&menu->btn[6]);
 //   DrawButton(&menu->btn[7]);
    
    DisplayDoTargetInfoMenuValueChange(menu,value);    
}
void DisplayDoTargetInfoMenuValueChange(UP_DOWN_ONE_MENU *menu, uint16 value)
{
    SetButtonText(&menu->btn[2],"%d",value);
    DrawButton(&menu->btn[2]);
}

MENUNODE * menuCreate()
{
    setDisplayDirection(DISPLAY_DIRECTION_LANDSCAPE);

    SetButtonText(&g_TitleBar,"Trim Reset");
    DrawButton(&g_TitleBar);
    
    // Main Menu  ............................................
    MENUNODE *root                     = createMENUNODE(NULL,    "TOP MENU",       &doTopMenu);
    MENUNODE *unlockWithKeyNode        = createMENUNODE(root,    "Unlock with Key",&doUnlockWithKey);
    MENUNODE *trimCountSet             = createMENUNODE(root,    "Trim Count",     &doTargetInfoMenu);    
        
    return root;
}

#endif
/* [] END OF FILE */
