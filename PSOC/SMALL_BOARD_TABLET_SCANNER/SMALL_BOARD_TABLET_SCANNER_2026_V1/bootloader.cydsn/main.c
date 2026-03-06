/* 
 * 초성 & 안도 신형 OP + OEE 통신보드
 * 바코드 트리거 적용 (초성 구형 OP)
 * 스위치 3 동일하게 적용
 * 18-01-08 안도 신형 버전 성공
 * 18-01-14 HWI 초성 신형 OP 성공
 * 18-02-07 바코드 1자리, 2자리, 3자리 모두 사용 가능. 최종 성공
 * 18-02-14 PWI 초성 신형 OP 성공한 버전

*/


#include "project.h"

unsigned long int timerCount =0;

void SysTickISRCallback(void)
{ 
    timerCount++;
    
    if((timerCount % 100) == 0) LED_Write(~LED_Read());    
}

void initSysTick()
{
    CySysTickStart();
    uint32 i;
    
    /* Find unused callback slot and assign the callback. */
    for (i = 0u; i < CY_SYS_SYST_NUM_OF_CALLBACKS; ++i)
    {
        if (CySysTickGetCallback(i) == NULL)
        {
            /* Set callback */
            CySysTickSetCallback(i, SysTickISRCallback);
            break;
        }
    }
}


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
 
    initSysTick();
    Bootloader_Start();

    for(;;)
    {
        
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
