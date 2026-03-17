/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "currentSensor.h"
#include "userProjectPatternSewing.h"
#include "lib/internalFlash.h"
#include "lib/externalFlash.h"
#include "count.h"
#include "andonApi.h"
#include "andonJson.h"

static uint8  g_bUpdateRuntime = FALSE;  /* currentSensor.c 내부 전용 */
static uint32 g_updateTimeTime = 0;      /* currentSensor.c 내부 전용 */

int16 getCurrentSensor()
{
    if(0u != ADC_SAR_Seq_IsEndConversion(ADC_SAR_Seq_RETURN_STATUS))
    {
        return abs(ADC_SAR_Seq_GetResult16(0));
    }
    return INT16_MIN;
}

void currentSensorRoutine()
{
    static uint8 bStart = FALSE;
    int16 result = 0u;
    static uint32 old_time, currTime;
    static int count=0;
    static uint32 value;
    
    if(g_ptrMachineParameter->current_enable==FALSE) return;
    
    if(bStart==FALSE) // for initialize..
    {
        bStart = TRUE;
        ADC_SAR_Seq_Start();
        ADC_SAR_Seq_StartConvert();  
        old_time = RTC_GetUnixTime();
    } else {
        
        if((result = getCurrentSensor()) != INT16_MIN)
        {
            //result = ADC_SAR_Seq_GetResult16(0);
           
            currTime=RTC_GetUnixTime();
        
            if(result> g_ptrMachineParameter->current_sensor_threshold && currTime != old_time)  // 3020(6-X), C2(9-X). 19. 15. 13. 11(X). 12(x)
            {      
             //       printf("->Sensor %d\r\n", abs(result));
                setActual(getActual()+1);
                old_time = currTime;
                
                if(isInTopMenu()) g_TopMenuNode->func(NULL,REFLASH);

                g_updateTimeTime = currTime + 2; // update 할 시간은 현재 시간에 더해 준다. (기존값 1)
                g_bUpdateRuntime =TRUE;
            }
            else if(g_bUpdateRuntime == TRUE && currTime >= g_updateTimeTime)
            {
              //  printf("Send..%lu\r\n", getActual()); 
                if(g_bTargetReceived==TRUE)
                {
                    updateRuntimeSum(getActual(),RTC_GetUnixTime()-g_lastSendTime); // target값을 서버로 부터 받았다면, 그때 부터 데이터를 보낼 수 있다
                    setActual(0); // <-- 데이타 보낸후에 0로 초기화한다.
                    g_lastSendTime = RTC_GetUnixTime();
                }
                SaveInternalFlash();
                g_bUpdateRuntime = FALSE;                
            }
        }   
    }
}


int doCurrentSensorSet(void *this, uint8 reflash) ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;    

    static char *listText[] = {"DISABLE", "ENABLE"};
    static uint16 indexSelect = 0;
    static uint16 page = 0;
    
    if(reflash)
    {

        if(g_ptrMachineParameter->current_enable)
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
                g_ptrMachineParameter->current_enable = FALSE;
            else
                 g_ptrMachineParameter->current_enable = TRUE;
            ShowWaitMessage();                                    
            SaveExternalFlashConfig();
            doSelectList(this, TRUE, listText, 2, &indexSelect, &page);
        }            
    }
    return MENU_RETURN_THIS;    
}

int doListMenuCurrent(void *this, uint8 reflash)
{
    static uint16 page = 0;
    return doListMenuPage(this, reflash, &page) ;
}

int doCurThreshold(void *this, uint8 reflash)
{    
    MENUNODE *thisMenu = (MENUNODE *) this;  
    static char *strValue = "Value";
    static int32 minValue = 0, maxValue = 100;
    static int32 incr[] ={1, 5, 10};
    
    static uint16 selectIncre = 0;
    static int32 value = 0;
    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                value = g_ptrMachineParameter->current_sensor_threshold;          
            }
            break;  
    }
  
    switch(doIncreseNumberMenu(reflash, thisMenu->nodeName, strValue, incr, &value, minValue, maxValue, &selectIncre))
    {
    case MENU_BOTTOM_LEFT:
        return MENU_RETURN_PARENT;
        break;
    case MENU_BOTTOM_RIGHT:
        {
            g_ptrMachineParameter->current_sensor_threshold = value;   
            ShowWaitMessage();
            SaveExternalFlashConfig();         
            return MENU_RETURN_PARENT;
        }
        break;
    }
    return MENU_RETURN_THIS;    
}

void DisplayMeasureMenu(int16 min, int16 max, int16 aver);
int doCurMeasure(void *this, uint8 reflash) /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;
    static uint16 page = 0;
    static int16 min, max, average;
    static uint32 oldTime, currTime;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                g_ListMenu.curPage = page;
                SetDrawListButtons(&g_ListMenu,thisMenu->nodeName,NULL,0, BUTTON_STYLE_LIST);

                SetDrawBottomButtons("QUIT", "SET", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE); 
                
                ADC_SAR_Seq_Start();
                ADC_SAR_Seq_StartConvert(); 
                                
                min = INT16_MAX;
                max = INT16_MIN;    
                average = 0;
                
                oldTime = RTC_GetUnixTime();
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();

                if(tc.isClick == FALSE) break;
                
                int ret = getIndexOfClickedButton(&tc,g_btnBottom,2);             
                switch(ret)
                {
                    case BOTTOM_LEFT:
                    case BOTTOM_RIGHT:                     
                    {
                        page = g_ListMenu.curPage;
                        
                        if(g_ptrMachineParameter->current_enable==FALSE)
                        {
                            ADC_SAR_Seq_Stop();
                        }
                        
                        if(ret==BOTTOM_RIGHT)
                        {
                            g_ptrMachineParameter->current_sensor_threshold = average;   
                            ShowWaitMessage();
                            SaveExternalFlashConfig();                                
                        }
                        return MENU_RETURN_PARENT;
                    }
                }      
            }
            break;
    }
    
    currTime = RTC_GetUnixTime();
    if(currTime != oldTime)
    {
        int16 cur = getCurrentSensor();

        min = MIN(min,cur);
        max = MAX(max,cur);
        average = (min+max) / 2;
        DisplayMeasureMenu(min, max, average);
    }
    
    oldTime = currTime;    
    return MENU_RETURN_THIS;    
}
void DisplayMeasureValue(int idx, int16 value)
{
    uint16 xPos, yPos;  
    BUTTON button, *btn;
    btn = &button;
    
    switch(idx)
    {
        case 0: SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_2_4,FALSE); break;
        case 1: SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_3_4,FALSE); break; 
        case 2: SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_4_4,FALSE); break;          
        break;
    }
    SetDefaultButtonStyle(btn);
    SetButtonStyleColor(btn,BUTTON_STYLE_TEXT);

    switch(idx)
    {
        case 0: SetButtonText(btn,"Minimum"); break;
        case 1: SetButtonText(btn,"Maximum"); break;
        case 2: SetButtonText(btn,"Average"); break;          
        break;
    }
    
    btn->font = Arial_round_16x24;
    btn->foregroundColor = YELLOW;
    btn->align = TEXT_ALIGN_LEFT;
    DrawButton(btn);
    xPos = btn->rect.left+ 20;
    yPos = btn->rect.bottom-15;       
     
    LCD_printf(xPos+5,yPos+3 ,WHITE,BLACK,Arial_round_16x24,"%d", value);       
}

void DisplayMeasureMenu(int16 min, int16 max, int16 average)
{
    DisplayMeasureValue(0,min);
    DisplayMeasureValue(1,max);
    DisplayMeasureValue(2,average);
}

MENUNODE *currentSensorMenuCreate(MENUNODE *root)
{
      MENUNODE        *curSet               = createMENUNODE  (root,  "CURRENT",      &doListMenuCurrent);  
      MENUNODE            *curEnable        = createMENUNODE  (curSet, "CUR.SENSOR",  &doCurrentSensorSet);
      MENUNODE            *curSetValue      = createMENUNODE  (curSet, "THRESHOLD",   &doCurThreshold);
      MENUNODE            *curTest          = createMENUNODE  (curSet, "MEASURE",     &doCurMeasure);
}
/* [] END OF FILE */
