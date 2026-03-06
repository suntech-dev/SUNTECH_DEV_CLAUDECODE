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


#ifdef USER_PROJECT_PATTERN_SEWING_MACHINE
    
#include "userProjectPatternSewing.h"
#include "lib/manageMenu.h"
#include "lib/andonMenu.h"
#include "count.h"
    
char *g_patternString[] = {"Target", "Actual", "Rate", "CT", "RT", "SQ", "SL", "SPI"};
uint8 g_noPatternString = 8;

char *g_sewingString[]  = {"Target", "Actual", "Rate", "Trim", "Stitch", "RT"};
uint8 g_nosewingString = 6;

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
                switch(g_ptrMachineParameter->machineType)
                {
                    case PATTERN_MACHINE:  page = patternPage; break;
                    case SEWING_MACHINE :  page = sewingPage;  break;
                }
                
                oldPage = 0xFF;
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
                    case 0: return 2; break;
                    
                    case 1:
                    case 2:
                        switch(g_ptrMachineParameter->machineType)
                        {
                            case PATTERN_MACHINE:
                                if     (page == 0) return 3;
                                else if(page > 0 ) page--;
                                break;
                            case SEWING_MACHINE :
                                if     (page == 0) return 3;
                                else if(page > 0 ) page--;
                                break;                                
                        }
                        break;
                        
                    case 3:                        
                    case 4:
                        switch(g_ptrMachineParameter->machineType)
                        {
                            case PATTERN_MACHINE:
                                if     (page == 0) return 4;
                             //   else if(page > 0 ) page--;
                                break;
                            case SEWING_MACHINE :
                                if     (page == 0) return 4;
                            //    else if(page > 0 ) page--;
                                break;                                
                        }                       
                        break;
                        
                    case 5:
                    case 6:
                        switch(g_ptrMachineParameter->machineType)
                        {
                            case PATTERN_MACHINE:
                                if     (page < 2)  page++;
                                break;
                            case SEWING_MACHINE :
                                if     (page < 1)  page++;
                                break;                            
                        }
                        break;             
                    default:
                        switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                        {
                            case BOTTOM_LEFT:  return 0;
                            case BOTTOM_RIGHT: return 1;                            
                        }
                    break;
                }
                
            }
            break;
    }
    
   if(oldPage != page || g_updateCountMenu == TRUE) TopMenuPageUpdate(&menu, page);
    
    oldPage = page;
    g_updateCountMenu = FALSE;
    return MENU_RETURN_THIS;    
}

void TopMenuPageUpdate(MONITORING_MENU *menu, uint8 page)
{
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

  //  menu->btn[3].disable = TRUE; // for no click second line
  //  menu->btn[4].disable = TRUE; // for no click second line 
        
    DrawButton(&menu->btn[1]);
    DrawButton(&menu->btn[3]);
    DrawButton(&menu->btn[5]); 

    TopMenuPageUpdateValue(menu,page);
    
}

void DisplayDoTopMenu(MONITORING_MENU *menu)
{
    switch(g_ptrMachineParameter->machineType)
    {
        case PATTERN_MACHINE: SetDrawMonitoringMenu(menu, "%d : %3.1f-Pair",
            g_ptrMachineParameter->patternPairCount, g_ptrMachineParameter->patternPair/10.);
            break;
        case SEWING_MACHINE:  SetDrawMonitoringMenu(menu, "%d : %3.1f-Pair",
            g_ptrMachineParameter->sewingPairTrim, g_ptrMachineParameter->sewingPair/10.);
        break;
    }    
    SetDrawBottomButtons("MENU", "ANDON", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
}

void TopMenuPageUpdateValue(MONITORING_MENU *menu, uint8 page)
{
    BUTTON *btn;
    uint8 persent = FALSE;
    
    float spi = 0.0;
    if((uint32) CONVERT_TO_4BYTE(g_ptrCount->patternNoStitchSumH,g_ptrCount->patternNoStitchSumL) > 0 &&
       (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternStitchLengthSumH,g_ptrCount->patternStitchLengthSumL) > 0)
    {
        spi = 25.4 / (((float) ((uint32) CONVERT_TO_4BYTE(g_ptrCount->patternStitchLengthSumH,g_ptrCount->patternStitchLengthSumL))) /
                      ((float) ((uint32) CONVERT_TO_4BYTE(g_ptrCount->patternNoStitchSumH,g_ptrCount->patternNoStitchSumL))));   
        
    }
//    printf("%lu, %lu\r\n", (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternStitchLengthSumH,g_ptrCount->patternStitchLengthSumL),
//    ((uint32) CONVERT_TO_4BYTE(g_ptrCount->patternNoStitchSumH,g_ptrCount->patternNoStitchSumL)));
//    
    switch(g_ptrMachineParameter->machineType)
    {
        case PATTERN_MACHINE:
        {
            switch(page)
            {
                case 0:
                    SetButtonText(&menu->btn[2],"%u",g_ptrMachineParameter->patternTarget);
                    SetButtonText(&menu->btn[4],"%lu",   ((uint32) CONVERT_TO_4BYTE(g_ptrCount->patternActualH,g_ptrCount->patternActualL))/10);
                    SetButtonText(&menu->btn[6],"%4.1f",((uint32) CONVERT_TO_4BYTE(g_ptrCount->patternActualH,g_ptrCount->patternActualL))*10./g_ptrMachineParameter->patternTarget);
                    persent = TRUE;
                    break;
                case 1:
                    SetButtonText(&menu->btn[2],"%0.1f",(uint32) CONVERT_TO_4BYTE(g_ptrCount->patternCycleTimeSumH,g_ptrCount->patternCycleTimeSumL) / 1000.);
                    SetButtonText(&menu->btn[4],"%0.1f",(uint32) CONVERT_TO_4BYTE(g_ptrCount->patternMotorRunTimeSumH,g_ptrCount->patternMotorRunTimeSumL) / 1000.);
                    SetButtonText(&menu->btn[6],"%4.1f",((uint32) CONVERT_TO_4BYTE(g_ptrCount->patternMotorRunTimeSumH,g_ptrCount->patternMotorRunTimeSumL)) * 100. / ((uint32) CONVERT_TO_4BYTE(g_ptrCount->patternCycleTimeSumH,g_ptrCount->patternCycleTimeSumL)));
                    persent = TRUE;
                    break;
                case 2:
                    SetButtonText(&menu->btn[2],"%lu",(uint32) CONVERT_TO_4BYTE(g_ptrCount->patternNoStitchSumH,g_ptrCount->patternNoStitchSumL));
                    SetButtonText(&menu->btn[4],"%lu",(uint32) CONVERT_TO_4BYTE(g_ptrCount->patternStitchLengthSumH,g_ptrCount->patternStitchLengthSumL));
                    SetButtonText(&menu->btn[6],"%4.1f",spi);    
                    break;
            }
        }
        break; 
        case SEWING_MACHINE:
        {
            switch(page)
            {
                case 0:
                    SetButtonText(&menu->btn[2],"%u",g_ptrMachineParameter->sewingTarget);
                    SetButtonText(&menu->btn[4],"%lu",   ((uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingActualH,g_ptrCount->sewingActualL))/10);
                    SetButtonText(&menu->btn[6],"%4.1f",((uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingActualH,g_ptrCount->sewingActualL))*10./g_ptrMachineParameter->sewingTarget);
                    persent = TRUE;                    
                    break;
                case 1:
                    SetButtonText(&menu->btn[2],"%lu",((uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingTrimCountSumH,g_ptrCount->sewingTrimCountSumL)));
                    SetButtonText(&menu->btn[4],"%lu",((uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingNoStitchSumH,g_ptrCount->sewingNoStitchSumL)));
                    SetButtonText(&menu->btn[6],"%lu",((uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingMotorRunTimeSumH,g_ptrCount->sewingMotorRunTimeSumL)));              
                    break;     
            }
        }
        break;        
    }    
    
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
/// doPairsInfoMenu   //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayDoPairsInfoMenu(UP_DOWN_TWO_MENU *menu);
void DisplayDoPairsInfoMenuSelectValue(UP_DOWN_TWO_MENU *menu, uint8 selectValue, uint16 leftValue, float rightValue);
void DisplayDoPairsInfoMenuLeftValueChange(UP_DOWN_TWO_MENU *menu, uint16 leftValue);
void DisplayDoPairsInfoMenuRightValueChange(UP_DOWN_TWO_MENU *menu, uint16 rightValue); // rightValue is 10 times

int doPairsInfoMenu(void *this, uint8 reflash)
{    
    MENUNODE *thisMenu = (MENUNODE *) this;    
    static UP_DOWN_TWO_MENU menu;    
    static uint16 selectValue = 0;
    static uint16 leftValue = 2;
    static uint16  rightValue = 25;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                DisplayDoPairsInfoMenu(&menu);
                
                switch(g_ptrMachineParameter->machineType)
                {
                    case PATTERN_MACHINE: 
                        leftValue  = g_ptrMachineParameter->patternPairCount;
                        rightValue = g_ptrMachineParameter->patternPair;
                        break;
                    case SEWING_MACHINE:
                        leftValue  = g_ptrMachineParameter->sewingPairTrim;
                        rightValue = g_ptrMachineParameter->sewingPair;
                        break;
                    break;
                }  
    
                DisplayDoPairsInfoMenuSelectValue(&menu, selectValue, leftValue, rightValue);
            }
            break;  

        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                uint16 indexClick = getIndexOfClickedButton(&tc, menu.btn,7);
                switch(indexClick)
                {
                    case UPDOWNTWO_FIRST_TEXT:  // left
                    case UPDOWNTWO_FIRST_VALUE:
                        selectValue = 0;
                        DisplayDoPairsInfoMenuSelectValue(&menu,selectValue, leftValue, rightValue);
                        break;
                    case UPDOWNTWO_SECOND_TEXT:  // right
                    case UPDOWNTWO_SECOND_VALUE:
                        selectValue = 1;
                        DisplayDoPairsInfoMenuSelectValue(&menu,selectValue, leftValue, rightValue);                        
                        break;                    
                    case UPDOWNTWO_UP_BUTTON: // for up
                        if      (selectValue==0 && leftValue < 10000)
                        {
                            leftValue++;
                            DisplayDoPairsInfoMenuLeftValueChange(&menu,leftValue);
                        }
                        else if (selectValue==1 && rightValue < 10000)
                        {
                            rightValue++;
                            DisplayDoPairsInfoMenuRightValueChange(&menu,rightValue);
                        }
                        break;                       
                    case UPDOWNTWO_DOWN_BUTTON: // for down
                        if      (selectValue==0 && leftValue > 1)
                        {
                            leftValue--;
                            DisplayDoPairsInfoMenuLeftValueChange(&menu,leftValue);
                        }                        
                        else if (selectValue==1 && rightValue > 1) 
                        {
                            rightValue--;
                            DisplayDoPairsInfoMenuRightValueChange(&menu,rightValue);
                        }                 
                        break;                    
                    default:
                        switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                        {
                            case BOTTOM_LEFT:  return MENU_RETURN_PARENT;
                            case BOTTOM_RIGHT: 
                            {
                                switch(g_ptrMachineParameter->machineType)
                                {
                                    case PATTERN_MACHINE: 
                                        g_ptrMachineParameter->patternPairCount = leftValue; 
                                        g_ptrMachineParameter->patternPair      = rightValue;
                                        break;
                                    case SEWING_MACHINE:
                                        g_ptrMachineParameter->sewingPairTrim = leftValue ; 
                                        g_ptrMachineParameter->sewingPair     = rightValue; 
                                        break;
                                    break;
                                }
                                ShowWaitMessage();
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

void DisplayDoPairsInfoMenu(UP_DOWN_TWO_MENU *menu)
{
    switch(g_ptrMachineParameter->machineType)
    {
        case PATTERN_MACHINE: SetDrawUpDownTwoMenu(menu, "Count", "Pair(s)", "Pairs infos.");   break;
        case SEWING_MACHINE:  SetDrawUpDownTwoMenu(menu, "Trim",  "Pair(s)", "Pairs infos.");   break;
        break;
    }    
    
    menu->btn[0].disable = TRUE; // for no click title
   
    SetDrawBottomButtons("QUIT", "SAVE", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
}

void DisplayDoPairsInfoMenuSelectValue(UP_DOWN_TWO_MENU *menu, uint8 selectValue, uint16 firstValue, float secondValue)
{
    SetButtonStyleColor(&menu->btn[UPDOWNTWO_FIRST_TEXT],  (selectValue==0) ? BUTTON_STYLE_WHITE : BUTTON_STYLE_DARKGREY);  // firtst text  
    SetButtonStyleColor(&menu->btn[UPDOWNTWO_FIRST_VALUE], (selectValue==0) ? BUTTON_STYLE_YELLOW: BUTTON_STYLE_DARKGREY);  // firtst Value  
    SetButtonStyleColor(&menu->btn[UPDOWNTWO_SECOND_TEXT], (selectValue==1) ? BUTTON_STYLE_WHITE : BUTTON_STYLE_DARKGREY);  // second text  
    SetButtonStyleColor(&menu->btn[UPDOWNTWO_SECOND_VALUE],(selectValue==1) ? BUTTON_STYLE_YELLOW: BUTTON_STYLE_DARKGREY);  // second Value    
    
    DrawButton(&menu->btn[UPDOWNTWO_FIRST_TEXT]);
    DrawButton(&menu->btn[UPDOWNTWO_SECOND_TEXT]);
    
    DisplayDoPairsInfoMenuLeftValueChange(menu,firstValue);
    DisplayDoPairsInfoMenuRightValueChange(menu,secondValue);    
}
void DisplayDoPairsInfoMenuLeftValueChange(UP_DOWN_TWO_MENU *menu, uint16 value)
{  
    SetButtonText(&menu->btn[UPDOWNTWO_FIRST_VALUE],"%d",value);    // left value   
    DrawButton(&menu->btn[UPDOWNTWO_FIRST_VALUE]);
}
void DisplayDoPairsInfoMenuRightValueChange(UP_DOWN_TWO_MENU *menu, uint16 value)
{
    SetButtonText(&menu->btn[UPDOWNTWO_SECOND_VALUE],"%3.1f",value/10.); // right value   
    DrawButton(&menu->btn[UPDOWNTWO_SECOND_VALUE]);     
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
            {
                switch(g_ptrMachineParameter->machineType)
                {
                    case PATTERN_MACHINE: 
                        value  = g_ptrMachineParameter->patternTarget;
                        break;
                    case SEWING_MACHINE:
                        value  = g_ptrMachineParameter->sewingTarget;
                        break;
                    break;
                }                  
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
                        if      (value+menu.inc[selectIncre] < 10000)
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
                                switch(g_ptrMachineParameter->machineType)
                                {
                                    case PATTERN_MACHINE: 
                                        g_ptrMachineParameter->patternTarget = value;
                                        break;
                                    case SEWING_MACHINE:
                                        g_ptrMachineParameter->sewingTarget = value;
                                        break;
                                    break;
                                }
                                ShowWaitMessage();
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
    menu->inc[2] = 100;
    
    SetDrawUpDownOneMenu(menu, "Target", "Target info.");
    
    menu->btn[0].disable = TRUE; // for no click title
    menu->btn[1].disable = TRUE; // for no click title
    menu->btn[2].disable = TRUE; // for no click title
    
    SetDrawBottomButtons("QUIT", "SAVE", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
}

void DisplayDoTargetInfoMenuSelectValue(UP_DOWN_ONE_MENU *menu, uint8 selectIncre, uint16 value)
{
    menu->btn[5].foregroundColor = ((selectIncre==0) ? ORANGE : DARKGREY);  // 1 incre
    menu->btn[6].foregroundColor = ((selectIncre==1) ? ORANGE : DARKGREY);  // 2 incre  
    menu->btn[7].foregroundColor = ((selectIncre==2) ? ORANGE : DARKGREY);  // 3 incre  

    DrawButton(&menu->btn[5]);
    DrawButton(&menu->btn[6]);
    DrawButton(&menu->btn[7]);
    
    DisplayDoTargetInfoMenuValueChange(menu,value);    
}
void DisplayDoTargetInfoMenuValueChange(UP_DOWN_ONE_MENU *menu, uint16 value)
{
    SetButtonText(&menu->btn[2],"%d",value);
    DrawButton(&menu->btn[2]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doActualInfoMenu   //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayDoActualInfoMenu(UP_DOWN_ONE_MENU *menu);
void DisplayDoActualInfoMenuSelectValue(UP_DOWN_ONE_MENU *menu, uint8 selectIncre, uint16 value);
void DisplayDoActualInfoMenuValueChange(UP_DOWN_ONE_MENU *menu, uint16 value);

int doActualInfoMenu(void *this, uint8 reflash)
{    
    MENUNODE *thisMenu = (MENUNODE *) this;    
    static UP_DOWN_ONE_MENU menu;    
    static uint16 selectIncre = 0;
    static uint32 value = 0, tmp;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                switch(g_ptrMachineParameter->machineType)
                {
                    case PATTERN_MACHINE: 
                        value  = (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternActualH, g_ptrCount->patternActualL)/10;
                        break;
                    case SEWING_MACHINE:
                        value  = (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingActualH, g_ptrCount->sewingActualL)/10;
                        break;
                    break;
                }                  
                DisplayDoActualInfoMenu(&menu);
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
                        if      (value+menu.inc[selectIncre] < 10000)
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
                                switch(g_ptrMachineParameter->machineType)
                                {
                                    case PATTERN_MACHINE: 
                                        tmp = (uint32) CONVERT_TO_4BYTE(g_ptrCount->patternActualH, g_ptrCount->patternActualL);
                                        g_ptrCount->patternActualH = CONVERT_TO_HIGH(value*10);
                                        g_ptrCount->patternActualL = CONVERT_TO_LOW(value*10);
                                        break;
                                    case SEWING_MACHINE:
                                        tmp = (uint32) CONVERT_TO_4BYTE(g_ptrCount->sewingActualH, g_ptrCount->sewingActualL);
                                        g_ptrCount->sewingActualH = CONVERT_TO_HIGH(value*10);
                                        g_ptrCount->sewingActualL = CONVERT_TO_LOW(value*10);
                                        break;
                                    break;
                                }
                            //    printf("%lu",(uint32) CONVERT_TO_4BYTE(g_ptrCount->patternActualH, g_ptrCount->patternActualL));
                                SaveInternalFlash();
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

void DisplayDoActualInfoMenu(UP_DOWN_ONE_MENU *menu)
{
    menu->inc[0] = 1;
    menu->inc[1] = 10;
    menu->inc[2] = 100;
    
    SetDrawUpDownOneMenu(menu, "Actual", "Actual info.");
    
    menu->btn[0].disable = TRUE; // for no click title
    menu->btn[1].disable = TRUE; // for no click title
    menu->btn[2].disable = TRUE; // for no click title
    
    SetDrawBottomButtons("QUIT", "SAVE", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int doMachine(void *this, uint8 reflash) ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;    
    static LIST_MENU menu;
    static char *listText[] = {"PATTERN", "SEWING"};
    static uint16 indexSelect;
    static uint16 displayDir;
    uint8 bUpdate = FALSE;
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                indexSelect = g_ptrMachineParameter->machineType;
                displayDir  = getMiscConfig()->bDisplayDir;  
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
                                    switch(g_ptrMachineParameter->machineType)
                                    {
                                        case 0: setDisplayDirection(DISPLAY_DIRECTION_PORTRAIT);  break;
                                        case 1: setDisplayDirection(DISPLAY_DIRECTION_LANDSCAPE); break;
                                    }                                                                  
                                    return MENU_RETURN_PARENT;
                                case BOTTOM_RIGHT: 
                                {
                                    g_ptrMachineParameter->machineType = indexSelect;
                                    switch(g_ptrMachineParameter->machineType)
                                    {
                                        case 0: setDisplayDirection(DISPLAY_DIRECTION_PORTRAIT);  break;
                                        case 1: setDisplayDirection(DISPLAY_DIRECTION_LANDSCAPE); break;
                                    }  
                                    
                                    SetCountLoop();
                                    ShowWaitMessage();                                    
                                    SaveExternalFlashConfig();
                                    return MENU_RETURN_PARENT;                                
                                }                            
                            }
                            break;
                        }
                    case 0:
                        indexSelect = 0;
                        setDisplayDirection(DISPLAY_DIRECTION_PORTRAIT);
                        bUpdate = TRUE;
                        break;
                    case 1:
                        indexSelect = 1;
                        setDisplayDirection(DISPLAY_DIRECTION_LANDSCAPE);                        
                        bUpdate = TRUE;                     
                        break;
                }                
            }
            break;
    }
    if(bUpdate)
    {
        SetDrawListButtons(&menu,thisMenu->nodeName,listText,2);                    
        UpdateDrawListButtons(&menu,indexSelect);
        SetDrawBottomButtons("QUIT", "SAVE", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
        bUpdate = FALSE;
    }
    return MENU_RETURN_THIS;    
}

MENUNODE * menuCreate()
{
    setDisplayDirection(getMiscConfig()->bDisplayDir);

    // Main Menu  ............................................
    MENUNODE *root                        = createMENUNODE(NULL,      "TOP MENU",     &doTopMenu);
    MENUNODE    *manage                   = manageMenuCreate(root);
    for(int i=0; i < getNoOfChild(manage); i++) 
    {
        MENUNODE *node = getNthChild(manage,i);
        if(strcmp(node->nodeName,"SETTINGS") == 0)
        {
            MENUNODE        *machine              = createMENUNODE  (node,  "MACHINE",      &doMachine);  
            break;
        }
    }
                                            
    MENUNODE    *andon                    = andonMenuCreate (root);                                               
    MENUNODE    *pairsInfoMenuNode        = createMENUNODE  (root,    "PAIR(S) INFO", &doPairsInfoMenu);
    MENUNODE    *targetInfoMenuNode       = createMENUNODE  (root,    "TARGET",       &doTargetInfoMenu);
    MENUNODE    *actualtInfoMenuNode      = createMENUNODE  (root,    "ACTUAL",       &doActualInfoMenu);
   return root;
}

#endif

/* [] END OF FILE */
