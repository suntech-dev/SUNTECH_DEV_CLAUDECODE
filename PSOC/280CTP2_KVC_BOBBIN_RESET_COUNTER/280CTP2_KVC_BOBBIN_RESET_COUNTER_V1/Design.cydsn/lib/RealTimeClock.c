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
#include "RealTimeClock.h"
#include "timerCounter.h"
//#define RTC_WDT_TEST

#define LFCLK_CYCLES_PER_SECOND     (32768u)
#define WCO_STARTUP_DELAY_CYCLES    (LFCLK_CYCLES_PER_SECOND)

/* Time: 02:59:50 */
#define TIME_HOUR           (0x02u)
#define TIME_MIN            (0x59u)
#define TIME_SEC            (0x50u)
#define HOUR_OFFSET         (16u)
#define MIN_OFFSET          (8u)
#define TIME_HR_MIN_SEC     ((uint32)(TIME_HOUR << HOUR_OFFSET) | \
                            (uint32)(TIME_MIN << MIN_OFFSET)    | \
                             TIME_SEC)
/* Date: 03/22/2014 */
#define DATE_MONTH          (RTC_MARCH)
#define DATE_DAY            (0x22u)
#define DATE_YEAR           (0x2014u)
#define MONTH_OFFSET        (24u)
#define DAY_OFFSET          (16u)
#define DATE_MONTH_DAY_YEAR ((uint32)(DATE_MONTH << MONTH_OFFSET)   | \
                            (uint32)(DATE_DAY << DAY_OFFSET)        | \
                             DATE_YEAR)

uint8  g_bIsAvaliableDateTime = FALSE;


///////////////////////////////////////////////////////////////
// 1s에 관련된 것들  ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////
uint8 g_uNoSecondCounter=0;
TIMER_COUNTER g_timerCounter_1s[MAX_NO_SECOND_COUNTER];

// 1s counter를 다시 시작한다
void resetCounter_1s(uint16 index)
{
    g_timerCounter_1s[index].current = g_timerCounter_1s[index].max;
}

// 1ms count의 목표에 도달 했는지 여부, index는 ONE_MILISECOND_COUNTER enum값
// 주의 : main loop의 총 수행시간이 1s 보다 크면 목표값에 도달한 것을 놓칠 수 있다.
uint8 isFinishCounter_1s(uint16 index) 
{    
    if(g_timerCounter_1s[index].current == 0)
    {
        resetCounter_1s(index);
        return TRUE;
    }
    return FALSE;
}

// 1s timer counter 등록 
uint8 registerCounter_1s(uint16 maxCount)
{
    if(g_uNoSecondCounter < MAX_NO_SECOND_COUNTER-1)
    {
        g_timerCounter_1s[g_uNoSecondCounter].current = 
        g_timerCounter_1s[g_uNoSecondCounter].max = maxCount;
        
        return g_uNoSecondCounter++;
    } else {
        printf("Warning : resisterCounter_1s()\r\n"); 
    }
    return 0xFF;   
}

// 현재 counter 값을 알아낸다. 
// 주의 counter is Decreasing..
uint16 getCurrentTimerCounter_1s(uint8 index) { return g_timerCounter_1s[index].current; }

// 1s 마다 인터럽트에 의한 호출되는 ISR
CY_ISR(ISR_ONE_SECOND)
{
    OneSecond_ISR();
    
    for(int i=0; i < g_uNoSecondCounter; i++)
    {
        if(g_timerCounter_1s[i].current > 0) g_timerCounter_1s[i].current--;           
    }
}

#ifdef RTC_WDT_TEST

/* Interrupt prototypes */
CY_ISR_PROTO(EnableRtcOperation);
CY_ISR_PROTO(UpdateTimeIsrHandler);
CY_ISR_PROTO(AlarmIsrHandler);
#endif
    
void initRealTimeClock()
{      
#ifdef RTC_WDT_TEST    
   /* Prepare COUNTER0 to use it by CySysTimerDelay function in
     * "INTERRUPT" mode: disable "clear on match" functionality, configure
     * COUNTER0 to generate interrupts on match.
     */
    CySysWdtSetClearOnMatch(CY_SYS_WDT_COUNTER0, 0u);
    CySysWdtSetMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_INT);
    
    /* Enable WDT COUNTER0 */
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
    
    /* Disable servicing interrupts from WDT_COUNTER0 to prevent
       trigger callback before the CySysTimerDelay() function. */
    CySysWdtDisableCounterIsr(CY_SYS_WDT_COUNTER0);
    
    /* Register EnableRtcOperation() by the COUNTER0. */
    CySysWdtSetInterruptCallback(CY_SYS_WDT_COUNTER0, EnableRtcOperation);
    
    /* Initiate run the EnableRtcOperation() callback in WCO_STARTUP_DELAY_CYCLES interval. */
    CySysTimerDelay(CY_SYS_WDT_COUNTER0, CY_SYS_TIMER_INTERRUPT, WCO_STARTUP_DELAY_CYCLES);
    
#endif    
    PWM_ONE_SECOND_Start();
    isr_ONE_SECOND_StartEx(ISR_ONE_SECOND);
    
    /* Start RTC component */
    RTC_Start();
    
 //   SetDateTime(2021, 7, 14, 17, 31, 21);   
}

void setCurrentTime(char *data)
{
#define SEPERATOR "-: "
    uint16 year, month, dayOfMonth, hour, minute, second;
//printf("%s\r\n",data);
//return;
    year          = atoi(&data[0]);
    month         = atoi(&data[5]);
    dayOfMonth    = atoi(&data[8]);
    hour          = atoi(&data[11]);
    minute        = atoi(&data[14]);
    second        = atoi(&data[17]);
    
    SetDateTime(year, month, dayOfMonth, hour, minute, second);
 //   printf("Cur Time : %4d-%02d-%02d %02d:%02d:%02d\r\n",  year, month, dayOfMonth, hour, minute, second);    
//    DEBUG_printf("Cur Time : %4d-%02d-%02d %02d:%02d:%02d\r\n",  year, month, dayOfMonth, hour, minute, second);
}   

void SetDateTime(uint16 year, uint16 month, uint16 day, uint16 hour, uint16 minute, uint16 second)
{
    uint32 time = 0;
 
    time = RTC_SetHours(time, hour);    
    time = RTC_SetMinutes(time, minute);       
    time = RTC_SetSecond(time, second);    

    uint32 date = 0;
    date = RTC_SetYear(date, year);
    date = RTC_SetMonth(date, month);
    date = RTC_SetDay(date, day);
    
    RTC_SetDateAndTime(time,date); 
    
    g_bIsAvaliableDateTime = TRUE;
}

void PrintRTC()
{
  //  RTC_Update();
    
    char timeBuffer[16u];
    char dateBuffer[16u];

    uint32 time;
    uint32 date;
    
    /* Get Date and Time from RTC */
    time = RTC_GetTime();
    date = RTC_GetDate();

    /* Print Date and Time to UART */
    sprintf(timeBuffer, "%02lu:%02lu:%02lu", RTC_GetHours(time), RTC_GetMinutes(time), RTC_GetSecond(time));
    sprintf(dateBuffer, "%02lu/%02lu/%02lu", RTC_GetMonth(date), RTC_GetDay(date), RTC_GetYear(date));
        
//    DEBUG_printf("%s\r\n",timeBuffer);
//    DEBUG_printf("%s\r\n\r\n",dateBuffer);
}

#ifdef RTC_WDT_TEST   
/*******************************************************************************
* Function Name: UpdateTimeIsrHandler
********************************************************************************
* Summary: 
*  The interrupt handler for WDT counter 0 interrupts. Toggles the LED_WdtIsr 
*  pin.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void UpdateTimeIsrHandler(void)
{
    RTC_Update();
}

void EnableRtcOperation(void)
{
    /* Switch LFCLK source from ILO to WCO. */
    CySysClkSetLfclkSource(CY_SYS_CLK_LFCLK_SRC_WCO);
    
    /* Configure COUNTER0 to generate interrupt every second. */
    CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK);
    CySysWdtSetClearOnMatch(CY_SYS_WDT_COUNTER0, 1u);
    CySysWdtSetMatch(CY_SYS_WDT_COUNTER0, LFCLK_CYCLES_PER_SECOND);
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
    
    /* Eegister UpdateTimeIsrHandler() by the COUNTER0. */
    CySysWdtSetInterruptCallback(CY_SYS_WDT_COUNTER0, UpdateTimeIsrHandler);
    
    /* Enable the COUNTER0 ISR handler. */
    CySysWdtEnableCounterIsr(CY_SYS_WDT_COUNTER0);
    
    /* Configure the LFCLK_Out pin to drive it by LFCLK. */
  //  LFCLK_Out_SetDriveMode(LFCLK_Out_DM_STRONG);
}

#endif

/* [] END OF FILE */
