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
 * 프로토콜: "actual_qty;cycle_time_s;thread_breakage_qty;motor_runtime_s;\r\n"
 * 예시:     "1;300;2;250;\r\n"
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
 * 프로토콜 (후행 ';' 있는 경우): "actual_qty;cycle_time_s;thread_breakage_qty;motor_runtime_s;\r\n"
 * 프로토콜 (후행 ';' 없는 경우): "actual_qty;cycle_time_s;thread_breakage_qty;motor_runtime_s\r\n"
 *   → 실제 자수기 장비는 후행 ';' 없이 전송 — \r\n 도달 시 sc==3 보완 처리
 * '\r', '\n' 은 sc==3 이면 파싱 시도, 그 외 버퍼 리셋
 * index=0 에서 숫자가 아닌 바이트는 무시 (echo 루프백 쓰레기 방어)
 * index>0 에서 숫자·';' 외 바이트는 즉시 버퍼 리셋 (MCSTATUS "0x??" 오염 방지)
 * ================================================================ */
uint8 uartJsonLoop()
{
    COUNT *ptrCount = getCount();

    while (UART_SpiUartGetRxBufferSize() > 0)
    {
        char c = UART_UartGetChar();

        /* '\r' / '\n' : 줄 끝 처리.
         * 자수기는 후행 ';' 없이 "2;82;0;75\r\n" 형태로 전송한다.
         * → 세미콜론이 3개인 경우: 후행 ';' 를 보완하여 파싱 시도.
         * → 그 외(MCSTATUS hex 잔류 등): 버퍼 리셋.
         * 후행 ';' 가 있는 경우("2;82;0;75;\r\n")는 4번째 ';' 수신 시 이미
         * 파싱·리셋 완료 → \r\n 도달 시 index=0 → 이 블록 미실행. */
        if (c == '\r' || c == '\n' || c == '\0')
        {
            if (g_UART_buff_index > 0)
            {
                /* 세미콜론 개수 확인 */
                uint8 sc = 0;
                int   si;
                for (si = 0; si < g_UART_buff_index; si++)
                {
                    if (g_UART_buff[si] == ';') sc++;
                }

                if (sc == 3 && g_UART_buff_index < UART_BUFFER_SIZE - 2)
                {
                    /* 후행 ';' 보완 후 파싱 시도 */
                    g_UART_buff[g_UART_buff_index++] = ';';
                    g_UART_buff[g_UART_buff_index]   = '\0';

                    if (uartEmbParsor(ptrCount))
                    {
                        if (g_bUartTestMode)
                        {
                            uartTestAddLine(g_UART_buff);
                        }
                        g_updateCountMenu = TRUE;
                        makeAndonPatternCount();
                        ADD_CONVERT_TO_4BYTE(ptrCount->patternActualH, ptrCount->patternActualL,
                                             ptrCount->patternCount * 10u);
                        ptrCount->patternCount = 0;
                        ForcefullyMarkDowntimeAsComplete();

                        g_UART_buff_index = 0;
                        memset(g_UART_buff, 0, UART_BUFFER_SIZE);
                        return TRUE;
                    }
                    /* sc==3 이지만 파싱 실패 → UART TEST 진단 표시 */
                    if (g_bUartTestMode) { uartTestAddLine(g_UART_buff); }
                }
                else if (g_bUartTestMode && sc >= 1)
                {
                    /* sc가 3 아닌 세미콜론 포함 라인 → UART TEST 진단 표시
                     * 자수기 실제 포맷 확인용: 후행 ; 여부, 필드 개수 등 육안 점검 */
                    g_UART_buff[g_UART_buff_index] = '\0';
                    uartTestAddLine(g_UART_buff);
                }

                g_UART_buff_index = 0;
                memset(g_UART_buff, 0, UART_BUFFER_SIZE);
            }
            continue;
        }

        /* 패킷 시작 바이트는 반드시 숫자(actual_qty 첫 자리)여야 함.
         * printf echo 루프백에 의해 쌓인 쓰레기 바이트를 여기서 차단. */
        if (g_UART_buff_index == 0 && (c < '0' || c > '9')) continue;

        /* 패킷 누적 중 유효하지 않은 문자(숫자·세미콜론 외) 수신 시 즉시 버퍼 리셋.
         * "MCSTATUS ... 0x41,0xff" 에서 '0' 이 버퍼를 시작한 뒤
         * 'x', ',', 알파벳 등에서 즉시 리셋하여 쓰레기 누적 방지. */
        if (g_UART_buff_index > 0 && (c < '0' || c > '9') && c != ';')
        {
            g_UART_buff_index = 0;
            memset(g_UART_buff, 0, UART_BUFFER_SIZE);
            continue;
        }

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

                /* 파싱 실패 → UART TEST 진단 표시 */
                if (g_bUartTestMode) { uartTestAddLine(g_UART_buff); }
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
 * 형식: "actual_qty;cycle_time_s;thread_breakage_qty;motor_runtime_s;"
 * 반환: TRUE=파싱 성공, FALSE=실패
 * ================================================================ */
static char uartEmbParsor(COUNT *ptrCount)
{
    char    tempBuf[UART_BUFFER_SIZE];
    char   *ptr      = tempBuf;
    char   *token    = NULL;
    uint8   fieldIdx = 0;

    uint16 actual_qty          = 0;
    uint32 cycle_time_s        = 0;
    uint16 thread_breakage_qty = 0;
    uint32 motor_runtime_s     = 0;

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
            case 1: cycle_time_s        = (uint32) atol(ptr); break;
            case 2: thread_breakage_qty = (uint16) atoi(ptr); break;
            case 3: motor_runtime_s     = (uint32) atol(ptr); break;
        }
        ptr = sep + 1;
        fieldIdx++;
    }

    if (fieldIdx < 4) return FALSE;

    /* 범위 유효성 검사 */
    if (actual_qty          == 0u)    return FALSE;  /* 0수량 패킷 무시 */
    if (actual_qty          > 10000u) return FALSE;
    if (cycle_time_s        > 10800u) return FALSE;  /* 최대 3시간 */
    if (thread_breakage_qty > 1000u)  return FALSE;
    if (motor_runtime_s     > 10800u) return FALSE;  /* 최대 3시간 */

    /* ── COUNT 구조체에 할당 ── */

    /* actual_qty: 증분값 누산 */
    ptrCount->patternCount += actual_qty;

    /* cycle_time: 초 단위 그대로 저장 */
    ptrCount->patternCycleTime = (uint16)(cycle_time_s);
    ADD_CONVERT_TO_4BYTE(ptrCount->patternCycleTimeSumH, ptrCount->patternCycleTimeSumL,
                         ptrCount->patternCycleTime);

    /* thread_breakage_qty: 단위 변환 없음 */
    ptrCount->embThreadBreakageQty = thread_breakage_qty;
    ADD_CONVERT_TO_4BYTE(ptrCount->embThreadBreakageQtySumH, ptrCount->embThreadBreakageQtySumL,
                         ptrCount->embThreadBreakageQty);

    /* motor_runtime: 초 단위 그대로 저장 */
    ptrCount->patternMotorRunTime = (uint16)(motor_runtime_s);
    ADD_CONVERT_TO_4BYTE(ptrCount->patternMotorRunTimeSumH, ptrCount->patternMotorRunTimeSumL,
                         ptrCount->patternMotorRunTime);

    return TRUE;
}

/* [] END OF FILE */
