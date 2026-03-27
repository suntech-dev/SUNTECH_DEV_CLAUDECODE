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
#include "main.h"
#include "IODefine.h"
#include "IOUtil.h"

unsigned long getSensor()
{
    int v = 0;
    
    char shift = 0;
    
//    v |= (TC_INT_Read()                 ? OFF : ON) << shift++;
//    v |= (SW1_Read()                  ? OFF : ON) << shift++;
//    v |= (SW2_Read()                  ? OFF : ON) << shift++;
//    v |= (SW3_Read()                  ? OFF : ON) << shift++;
//    v |= (SW4_Read()                  ? OFF : ON) << shift++;
//    
//    v |= (BOBBIN_WINDER_Read()        ? OFF : ON) << shift++;
//    v |= (THREAD_SENSOR_Read()        ? ON : OFF) << shift++;
//    v |= (RPM_COUNT_Read()            ? OFF : ON) << shift++;       

  //  if(UART_WIFIDATA_GetTxBufferSize() == 0) console_log("%d\r\n", v);
    
    return v;
}


void defineIO()
{        
}

void doTimer()
{
  setIOTimer(t100MS, 100, _T(t100MS));
  setIOTimer(t500MS, 500, _T(t500MS));
  if(uT(t500MS))
  {
//     // sendIOStat();
//       LED_RED_Write(~LED_RED_Read());
  }

#define TIMER_OFF(a,b)     setIOTimer(a,  200,   O(b)); \
                           if(uT(a)) O(b) = OFF;
/*
    TIMER_OFF(tHEAD_TURN_PULSE,       oHEAD_TURN_PULSE);
    TIMER_OFF(tTAPE_TRIM_PULSE,       oTAPE_TRIM_PULSE);
    TIMER_OFF(tTAPE_FEED_UP_PULSE,    oTAPE_FEED_UP_PULSE);
    TIMER_OFF(tTAPE_FEED_DOWN_PULSE,  oTAPE_FEED_DOWN_PULSE);
    TIMER_OFF(tX_PULSE,               oX_PULSE);
    TIMER_OFF(tY_PULSE,               oY_PULSE);*/          
}

void doOutput()
{    
 
}