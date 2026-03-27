/* ========================================
 *
 * Copyright Suntech, 2026.03.26
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
 *
 * EMBROIDERY_S 전용 UART 파서
 * 프로토콜: "actual_qty;cycle_time_ms;thread_breakage_qty;motor_runtime_ms;\r\n"
 * 예시:     "1;300000;2;250000;\r\n"
 *
*/
#include "uartJson.h"
#include "count.h"
#include "andonApi.h"
#include "uartTestMenu.h"
#include "userProjectPatternSewing.h"
#include "lib/internalFlash.h"
#include "lib/widget.h"
#include "downtime.h"

#define UART_BUFFER_SIZE 512

static char g_UART_buff[UART_BUFFER_SIZE];
static int  g_UART_buff_index = 0;

static char uartEmbParsor(COUNT *ptrCount);

/* ================================================================
 * uartJsonLoop — 메인 루프에서 CountFunc() 를 통해 호출
 *
 * 프로토콜: "actual_qty;cycle_time_ms;thread_breakage_qty;motor_runtime_ms;\r\n"
 * 종료 조건: 4번째 ';' 수신
 * '\r', '\n' 은 버퍼에 저장하지 않고 무시
 * index=0 에서 숫자가 아닌 바이트는 무시 (echo 루프백 쓰레기 방어)
 * ================================================================ */
uint8 uartJsonLoop()
{
    COUNT *ptrCount = getCount();

    while (UART_SpiUartGetRxBufferSize() > 0)
    {
        char c = UART_UartGetChar();

        if (c == '\0' || c == '\r' || c == '\n') continue;

        /* 패킷 시작 바이트는 반드시 숫자(actual_qty 첫 자리)여야 함.
         * printf echo 루프백에 의해 쌓인 쓰레기 바이트를 여기서 차단. */
        if (g_UART_buff_index == 0 && (c < '0' || c > '9')) continue;

        if (g_UART_buff_index >= UART_BUFFER_SIZE - 1)
        {
            g_UART_buff_index = 0;
            continue;
        }

        g_UART_buff[g_UART_buff_index++] = c;

        if (c == ';')
        {
            /* 세미콜론 개수 카운트 */
            uint8 semicolonCount = 0;
            int   k;
            for (k = 0; k < g_UART_buff_index; k++)
            {
                if (g_UART_buff[k] == ';') semicolonCount++;
            }

            if (semicolonCount >= 4)
            {
                g_UART_buff[g_UART_buff_index] = '\0';

                if (uartEmbParsor(ptrCount))
                {
                    /* UART TEST 모드 활성 시 수신 문자열 미러 */
                    if (g_bUartTestMode)
                    {
                        uartTestAddLine(g_UART_buff);
                    }

                    /* actual_qty 서버 전송 (patternCount = 이번 패킷 수량, 리셋 전에 호출) */
                    g_updateCountMenu = TRUE;
                    makeAndonPatternCount();

                    /* actual_qty 누산 → patternActualH/L 이관 (패킷마다 즉시)
                     * LCD 표시는 patternActualH/L ÷ 10 이므로 ×10 단위로 저장 */
                    ADD_CONVERT_TO_4BYTE(ptrCount->patternActualH, ptrCount->patternActualL,
                                         ptrCount->patternCount * 10u);
                    ptrCount->patternCount = 0;

                    ForcefullyMarkDowntimeAsComplete();
                    /* SaveInternalFlash() 는 Em_EEPROM_Write() 내부에서 전역 인터럽트를
                     * ~20ms 비활성화하므로 UART FIFO 오버플로우 발생.
                     * count.c의 CountSaveFlashIfDirty()를 통해 1초 주기로 지연 저장한다. */

                    g_UART_buff_index = 0;
                    memset(g_UART_buff, 0, UART_BUFFER_SIZE);
                    return TRUE;
                }

                g_UART_buff_index = 0;
                memset(g_UART_buff, 0, UART_BUFFER_SIZE);
            }
        }
    }
    return FALSE;
}

/* ================================================================
 * uartEmbParsor — 세미콜론 구분 파서
 *
 * 형식: "actual_qty;cycle_time_ms;thread_breakage_qty;motor_runtime_ms;"
 * 반환: TRUE=파싱 성공, FALSE=실패
 * ================================================================ */
static char uartEmbParsor(COUNT *ptrCount)
{
    char    tempBuf[UART_BUFFER_SIZE];
    char   *ptr      = tempBuf;
    char   *token    = NULL;
    uint8   fieldIdx = 0;

    uint16 actual_qty          = 0;
    uint32 cycle_time_ms       = 0;
    uint16 thread_breakage_qty = 0;
    uint32 motor_runtime_ms    = 0;

    snprintf(tempBuf, sizeof(tempBuf), "%s", g_UART_buff);

    while (fieldIdx < 4)
    {
        char *sep = strchr(ptr, ';');
        if (sep == NULL) break;
        *sep = '\0';

        if (*ptr == '\0')
        {
            /* 빈 필드 — 무효 */
            return FALSE;
        }

        switch (fieldIdx)
        {
            case 0:
                /* actual_qty 필드: 반드시 숫자로 시작해야 함
                 * 선행 제어 문자가 있으면 atoi()가 0을 반환하므로 명시적 체크 */
                if (*ptr < '0' || *ptr > '9') return FALSE;
                actual_qty = (uint16) atoi(ptr);
                break;
            case 1: cycle_time_ms       = (uint32) atol(ptr); break;
            case 2: thread_breakage_qty = (uint16) atoi(ptr); break;
            case 3: motor_runtime_ms    = (uint32) atol(ptr); break;
        }
        ptr = sep + 1;
        fieldIdx++;
    }

    if (fieldIdx < 4) return FALSE;

    /* 범위 유효성 검사 */
    if (actual_qty          == 0u)      return FALSE;  /* 0수량 패킷 무시 */
    if (actual_qty          > 10000u)   return FALSE;
    if (cycle_time_ms       > 3600000u) return FALSE;
    if (thread_breakage_qty > 1000u)    return FALSE;
    if (motor_runtime_ms    > 3600000u) return FALSE;

    printf("PARSE: actual_qty=%u ct=%lu tb=%u mrt=%lu\r\n",
           actual_qty, cycle_time_ms, thread_breakage_qty, motor_runtime_ms);

    /* ── COUNT 구조체에 할당 ── */

    /* actual_qty: 증분값 누산 */
    ptrCount->patternCount += actual_qty;

    /* cycle_time: ms → 0.1초 단위 변환 (÷100), 최대 36000 → uint16 안전 */
    ptrCount->patternCycleTime = (uint16)(cycle_time_ms / 100u);
    ADD_CONVERT_TO_4BYTE(ptrCount->patternCycleTimeSumH, ptrCount->patternCycleTimeSumL,
                         ptrCount->patternCycleTime);

    /* thread_breakage_qty: 단위 변환 없음 */
    ptrCount->embThreadBreakageQty = thread_breakage_qty;
    ADD_CONVERT_TO_4BYTE(ptrCount->embThreadBreakageQtySumH, ptrCount->embThreadBreakageQtySumL,
                         ptrCount->embThreadBreakageQty);

    /* motor_runtime: ms → 0.1초 단위 변환 (÷100), 최대 36000 → uint16 안전 */
    ptrCount->patternMotorRunTime = (uint16)(motor_runtime_ms / 100u);
    ADD_CONVERT_TO_4BYTE(ptrCount->patternMotorRunTimeSumH, ptrCount->patternMotorRunTimeSumL,
                         ptrCount->patternMotorRunTime);

    return TRUE;
}

/* [] END OF FILE */
