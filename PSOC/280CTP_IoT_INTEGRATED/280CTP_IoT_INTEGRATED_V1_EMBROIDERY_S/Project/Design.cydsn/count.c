/* ========================================
 *
 * Copyright Suntech, 2023.04.13
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "main.h"
#include "count.h"
#include "lib/internalFlash.h"
#include "userProjectPatternSewing.h"
#include "andonApi.h"
#include "uartJson.h"

uint8 g_updateCountMenu=FALSE;

COUNT *g_ptrCount = NULL;

static uint32 g_uWorkingTimeCount=0;   /* 접근자 함수(Get/Reset)로만 외부 접근 */
static uint8  g_bCountFlashDirty = FALSE; /* UART 패킷 수신 후 플래시 저장 대기 플래그 */

void PatternCountLoop();

void (*CountFunc)() = &PatternCountLoop;

void initCount()
{
    if(sizeof(COUNT) > sizeof(g_internalFlash.data))
    {
        printf("Wrong... COUNT size is large than g_internalFlash.data\r\n");
    }
    g_ptrCount = (COUNT *) &g_internalFlash.data;

    SetCountLoop();
}

void SetCountLoop()
{
    /* BLACK_CPU: PATTERN_MACHINE 고정 */
    CountFunc = &PatternCountLoop;
    trim_Int_Stop();
}

void PatternCountLoop()
{
    /* UART 하드웨어 버퍼에 쌓인 패킷을 한 번에 모두 소진 */
    while (uartJsonLoop() == TRUE)
    {
        g_updateCountMenu = TRUE;
        g_bCountFlashDirty = TRUE;  /* 1초 주기 지연 저장 예약 */
    }
}

/* ================================================================
 * CountSaveFlashIfDirty — UART 패킷 수신 후 플래시 지연 저장
 *
 * Em_EEPROM_Write()는 내부적으로 전역 인터럽트를 ~20ms 비활성화.
 * uartJson.c 핫 패스에서 직접 호출하면 UART FIFO 오버플로우 발생.
 * main.c의 1초 타이머 블록에서 호출하여 안전한 시점에 저장한다.
 * ================================================================ */
void CountSaveFlashIfDirty(void)
{
    if (g_bCountFlashDirty == FALSE) return;
    g_bCountFlashDirty = FALSE;
    SaveInternalFlash();
}

COUNT *getCount()
{
    return g_ptrCount;
}

void ResetCount()
{
    g_ptrCount->patternActualH = 0;
    g_ptrCount->patternActualL = 0;
    g_ptrCount->patternCycleTimeSumH = 0;
    g_ptrCount->patternCycleTimeSumL = 0;
    g_ptrCount->patternMotorRunTimeSumH = 0;
    g_ptrCount->patternMotorRunTimeSumL = 0;
    g_ptrCount->patternNoStitchSumH = 0;
    g_ptrCount->patternNoStitchSumL = 0;
    g_ptrCount->patternStitchLengthSumH = 0;
    g_ptrCount->patternStitchLengthSumL = 0;
    g_ptrCount->patternTrimCountSumH = 0;
    g_ptrCount->patternTrimCountSumL = 0;
    g_ptrCount->patternEmergencyTimeSumH = 0;
    g_ptrCount->patternEmergencyTimeSumL = 0;
    g_ptrCount->sewingActualH = 0;
    g_ptrCount->sewingActualL = 0;

    g_ptrCount->embThreadBreakageQty     = 0;
    g_ptrCount->embThreadBreakageQtySumH = 0;
    g_ptrCount->embThreadBreakageQtySumL = 0;

    SaveInternalFlash();
}

void WorkingTimeCount()
{
    g_uWorkingTimeCount++;
}

uint32 GetWorkingTimeCount()
{
    return g_uWorkingTimeCount;
}

void ResetWorkingTimeCount()
{
    g_uWorkingTimeCount = 0;
}
/* [] END OF FILE */
