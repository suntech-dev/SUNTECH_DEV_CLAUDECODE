
#include "WarningLight.h"
#include "lib/sysTick.h"

static WARNING_LIGHT  g_warning = {  /* WarningLight.c 내부 전용 */
    .bRED = FALSE, 
    .uBlinkTimeRED = 0,
    
    .bGREEN = FALSE,
    .uBlinkTimeGREEN = 0,
    
    .bBUZZER = FALSE,
    .uBlinkTimeBUZZER = 0,
};

void initWarning()
{
   g_warning.index_RED    = registerCounter_1ms(500);
   g_warning.index_GREEN  = registerCounter_1ms(500);  
#ifdef USE_BUZZER     
   g_warning.index_BUZZER = registerCounter_1ms(BUZZER_ON_OFF_TIME_INTERVAL); 
   g_warning.uBlinkTimeBUZZER = BUZZER_ON_OFF_TIME_INTERVAL;
#endif
}

void WarningLightSet(int16 index, uint8 OnOff, uint16 blinkTimeInterval)
{
    switch(index)
    {
        case 1: 
           g_warning.bRED = OnOff;
           g_warning.uBlinkTimeRED = blinkTimeInterval;
           setCountMax_1ms(g_warning.index_RED, blinkTimeInterval);
           break;
        case 2:
           g_warning.bGREEN = OnOff;
           g_warning.uBlinkTimeGREEN = blinkTimeInterval;        
           setCountMax_1ms(g_warning.index_GREEN, blinkTimeInterval);        
           break;        
    }
    
#ifdef USE_BUZZER      
    g_warning.bBUZZER = g_warning.bRED | g_warning.bGREEN;
   // g_warning.uBlinkTimeBUZZER = BUZZER_ON_OFF_TIME_INTERVAL;
   // setCountMax_1ms(g_warning.index_BUZZER, g_warning.uBlinkTimeBUZZER);
#endif    
}

void WarningLight()
{
    // For RED LIGHT
    if(g_warning.bRED)
    {
       if(isFinishCounter_1ms(g_warning.index_RED))
       {
           resetCounter_1ms(g_warning.index_RED);

           if(g_warning.uBlinkTimeRED == 0) RESERVED_OUT_2_Write(TRUE);
           else                             RESERVED_OUT_2_Write(!RESERVED_OUT_2_ReadDataReg());
       }
    } else {
        RESERVED_OUT_2_Write(FALSE);
    }
    
    // For GREEN LIGHT
    if(g_warning.bGREEN)
    {
       if(isFinishCounter_1ms(g_warning.index_GREEN))
       {
           resetCounter_1ms(g_warning.index_GREEN);
           if(g_warning.uBlinkTimeGREEN == 0) RESERVED_OUT_1_Write(TRUE);
           else                               RESERVED_OUT_1_Write(!RESERVED_OUT_1_ReadDataReg());       
       }
    } else {
        RESERVED_OUT_1_Write(FALSE);
    }
    
#ifdef USE_BUZZER    
    // For BUZZER   
    if(g_warning.bBUZZER)
    {
       if(isFinishCounter_1ms(g_warning.index_BUZZER))
       {
           resetCounter_1ms(g_warning.index_BUZZER);
           if(g_warning.uBlinkTimeBUZZER == 0) RESERVED_OUT_3_Write(TRUE);
           else                                RESERVED_OUT_3_Write(!RESERVED_OUT_3_ReadDataReg());       
       }
    } else {
        RESERVED_OUT_3_Write(FALSE);
    }
#endif    
}


                                    
    
/* [] END OF FILE */
