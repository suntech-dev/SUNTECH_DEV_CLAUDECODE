/* ========================================
 *
 * Copyright Suntech, 2023.04.16
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "uartJson.h"
#include "lib/UI.h"
#include "lib/FT5x46.h"
#include "lib/LEDControl.h"

/* ================================================================
 * 수신 버퍼 — '\n' 수신 시 한 줄로 확정
 * ================================================================ */
#define UART_BUFFER_SIZE  1024

static char g_UART_buff[UART_BUFFER_SIZE];
static int  g_UART_buff_index = 0;

/* ================================================================
 * UART 뷰어 설정
 * ================================================================ */
#define UART_VIEW_MAX_REC         5     /* 최대 보관 라인 수              */
#define UART_VIEW_REC_LEN      1024    /* 라인 당 최대 저장 문자 수       */
#define UART_VIEW_HEADER_H       40     /* 헤더 높이 (px)                 */
#define UART_VIEW_FOOTER_H       40     /* 푸터 높이 (px)                 */
#define UART_VIEW_LINE_H         18     /* 줄 높이 (Font8x16 + 여백)      */
#define UART_VIEW_FONT_W          8     /* 폰트 너비 (px)                 */

#define VIEW_HDR_BG   CONVERT565(0,   0,  100)   /* 헤더 배경 (진한 파란색) */
#define VIEW_BTN_BG   CONVERT565( 40,  40,  80)   /* 버튼 배경               */
#define VIEW_ERR_FG   CONVERT565(255,  60,  60)   /* 에러 라인 (밝은 빨간색) */

/* ================================================================
 * 뷰어 상태 변수
 * ================================================================ */
static char  g_recBuf[UART_VIEW_MAX_REC][UART_VIEW_REC_LEN];
static uint8 g_recTerminated[UART_VIEW_MAX_REC]; /* TRUE=정상(\n), FALSE=비정상(overflow) */
static uint8 g_recHead   = 0;   /* 가장 오래된 라인의 ring index */
static uint8 g_recCount  = 0;   /* 현재 저장된 라인 수           */
static uint16 g_recvCount = 0;  /* 총 수신 라인 수 (누적)        */

static int16  g_scrollLine   = 0;
static uint8  g_bNeedRedraw  = TRUE;
static uint8  g_touchHandled = FALSE;
static uint8  g_autoScroll   = TRUE;   /* TRUE: 새 데이터 수신 시 자동으로 최하단 스크롤 */

/* 타임아웃 flush — \n 없이 데이터가 와도 일정 루프 후 WHITE로 확정 */
// #define UART_IDLE_FLUSH_COUNT   50u    /* 연속 50 루프 수신 없으면 강제 flush */
#define UART_IDLE_FLUSH_COUNT   200u 
static uint16 g_uart_idle_count = 0;

/* ================================================================
 * 내부 함수 선언
 * ================================================================ */
static void   saveRecord(uint8 terminated);
static uint16 getCharsPerLine(void);
static uint16 getVisibleLines(void);
static uint16 calcTotalLines(void);

/* ================================================================
 * 헬퍼: 화면 크기 기반 동적 계산
 * ================================================================ */
static uint16 getCharsPerLine(void)
{
    return g_SCREEN_WIDTH / UART_VIEW_FONT_W;
}

static uint16 getVisibleLines(void)
{
    return (g_SCREEN_HEIGHT - UART_VIEW_HEADER_H - UART_VIEW_FOOTER_H)
           / UART_VIEW_LINE_H;
}

static uint16 calcTotalLines(void)
{
    uint16 total = 0;
    uint16 cpl   = getCharsPerLine();
    uint8  r;
    for (r = 0; r < g_recCount; r++)
    {
        uint8  idx = (g_recHead + r) % UART_VIEW_MAX_REC;
        uint16 len = (uint16) strlen(g_recBuf[idx]);
        if (len == 0) len = 1;  /* 빈 줄도 최소 1행 */
        total += (len + cpl - 1) / cpl;
    }
    return total;
}

/* ================================================================
 * saveRecord — 확정된 g_UART_buff 한 줄을 링 버퍼에 저장
 *
 * terminated: TRUE  = '\n'/'\r' 수신으로 정상 확정 (WHITE 표시)
 *             FALSE = 버퍼 오버플로우로 강제 확정   (RED 표시)
 * ================================================================ */
static void saveRecord(uint8 terminated)
{
    uint8 writeIdx;

    if (g_recCount < UART_VIEW_MAX_REC)
    {
        writeIdx = (g_recHead + g_recCount) % UART_VIEW_MAX_REC;
        g_recCount++;
    }
    else
    {
        /* 가장 오래된 항목 덮어쓰기 */
        writeIdx = g_recHead;
        g_recHead = (g_recHead + 1) % UART_VIEW_MAX_REC;
    }

    strncpy(g_recBuf[writeIdx], g_UART_buff, UART_VIEW_REC_LEN - 1);
    g_recBuf[writeIdx][UART_VIEW_REC_LEN - 1] = '\0';
    g_recTerminated[writeIdx] = terminated;
    g_recvCount++;
}

/* ================================================================
 * uartJsonLoop — 메인 루프에서 호출
 *
 * '\n'/'\r' 수신       → WHITE 확정 (정상 라인)
 * 버퍼 오버플로우      → WHITE 확정 (1023자 초과 시 강제 커밋)
 * 타임아웃 flush       → WHITE 확정 (UART_IDLE_FLUSH_COUNT 루프 무수신)
 * → 모든 경우에 WHITE 로 표시 ('\n' 없는 데이터도 LCD 에 표시됨)
 * ================================================================ */
uint8 uartJsonLoop(void)
{
    uint8 received = FALSE;

    while (UART_SpiUartGetRxBufferSize() > 0)
    {
        char c = UART_UartGetChar();

        if (c == '\0') continue;

        /* 버퍼 오버플로우 — '\n' 없이 1023자 도달 → WHITE 확정 */
        if (g_UART_buff_index >= UART_BUFFER_SIZE - 1)
        {
            g_UART_buff[g_UART_buff_index] = '\0';
            g_UART_buff_index = 0;

            saveRecord(TRUE);   /* WHITE: 종료자 없어도 정상 표시 */

            if (g_autoScroll)
            {
                uint16 totalLines = calcTotalLines();
                uint16 visLines   = getVisibleLines();
                if (totalLines > visLines)
                    g_scrollLine = (int16)(totalLines - visLines);
                else
                    g_scrollLine = 0;
            }

            g_bNeedRedraw = TRUE;
            received      = TRUE;

            /* 오버플로우를 유발한 현재 문자는 새 버퍼에 첫 글자로 저장 */
            if (c != '\n' && c != '\r')
            {
                g_UART_buff[g_UART_buff_index++] = c;
            }
            continue;
        }

        /* '\n' 또는 '\r' 수신 → WHITE(정상) 확정
         * (\r\n 연속 수신 시 두 번째 빈 커밋은 index==0 조건으로 자동 무시됨) */
        if (c == '\n' || c == '\r')
        {
            if (g_UART_buff_index > 0)
            {
                g_UART_buff[g_UART_buff_index] = '\0';
                g_UART_buff_index = 0;

                saveRecord(TRUE);  /* WHITE: 정상 종료 */

                if (g_autoScroll)
                {
                    uint16 totalLines = calcTotalLines();
                    uint16 visLines   = getVisibleLines();
                    if (totalLines > visLines)
                        g_scrollLine = (int16)(totalLines - visLines);
                    else
                        g_scrollLine = 0;
                }

                g_bNeedRedraw = TRUE;
                received      = TRUE;
            }
            continue;
        }

        g_UART_buff[g_UART_buff_index++] = c;
    }

    /* ----------------------------------------------------------------
     * 타임아웃 flush:
     *   \n/\r 없이 데이터가 들어와도 UART_IDLE_FLUSH_COUNT 루프 동안
     *   새 수신이 없으면 버퍼 내용을 WHITE 로 확정해 LCD 에 표시한다.
     * ---------------------------------------------------------------- */
    if (received)
    {
        /* 이번 루프에 수신 있음 → 카운터 리셋 */
        g_uart_idle_count = 0;
    }
    else if (g_UART_buff_index > 0)
    {
        /* 미완성 데이터가 버퍼에 있고 수신이 없는 루프 */
        g_uart_idle_count++;
        if (g_uart_idle_count >= UART_IDLE_FLUSH_COUNT)
        {
            g_uart_idle_count = 0;
            g_UART_buff[g_UART_buff_index] = '\0';
            g_UART_buff_index = 0;

            saveRecord(TRUE);  /* WHITE: 타임아웃 flush */

            if (g_autoScroll)
            {
                uint16 totalLines = calcTotalLines();
                uint16 visLines   = getVisibleLines();
                g_scrollLine = (totalLines > visLines)
                               ? (int16)(totalLines - visLines) : 0;
            }

            g_bNeedRedraw = TRUE;
            received      = TRUE;
        }
    }
    else
    {
        /* 버퍼 비어있고 수신도 없음 → 카운터 리셋 */
        g_uart_idle_count = 0;
    }

    /* UART 수신 시 LED2 Green 깜박임 (RX 인디케이터) */
    if (received)
    {
        // g_uLED2_Color      = LED_GREEN;
        // g_bLED2_Flickering = TRUE;
    }

    return received;
}

/* ================================================================
 * uartJsonDrawScreen — LCD 뷰어 화면 그리기 (변경 시에만)
 *
 *  [y:  0 ~  39]       헤더: 타이틀 + 수신 카운트
 *  [y: 40 ~ (H-40)]   바디: WHITE=정상 / RED=종료자 없음
 *  [y: (H-40) ~ H]    푸터: [UP] [DOWN] [CLEAR]
 * ================================================================ */
void uartJsonDrawScreen(void)
{
    if (!g_bNeedRedraw) return;
    g_bNeedRedraw = FALSE;

    uint16 screenW    = g_SCREEN_WIDTH;
    uint16 screenH    = g_SCREEN_HEIGHT;
    uint16 bodyTop    = UART_VIEW_HEADER_H;
    uint16 bodyBottom = screenH - UART_VIEW_FOOTER_H;
    uint16 cpl        = getCharsPerLine();
    uint16 visLines   = getVisibleLines();

    /* --- 헤더 --- */
    FillRectangle(0, 0, screenW, bodyTop, VIEW_HDR_BG);
    LCD_printf(4, 14, WHITE, VIEW_HDR_BG, Font8x16,
               "CTP280 UART RX TESTER V1  [%u]", g_recvCount);

    /* --- 바디 배경 --- */
    FillRectangle(0, bodyTop, screenW, bodyBottom, BLACK);

    /* --- 텍스트 렌더링 ---
     * 모든 레코드 → WHITE (\n 수신, 오버플로우, 타임아웃 flush 모두 WHITE) */
    int16  absLine  = 0;
    uint16 dispLine = 0;
    char   lineBuf[41];  /* 최대 40자 + null */
    uint8  r;

    for (r = 0; r < g_recCount && dispLine < visLines; r++)
    {
        uint8  idx  = (g_recHead + r) % UART_VIEW_MAX_REC;
        uint16 len  = (uint16) strlen(g_recBuf[idx]);
        COLOR  fg   = g_recTerminated[idx] ? WHITE : VIEW_ERR_FG;

        if (len == 0) len = 1;

        uint16 pos = 0;
        while (pos < len && dispLine < visLines)
        {
            if (absLine >= g_scrollLine)
            {
                uint16 copyLen = len - pos;
                if (copyLen > cpl) copyLen = cpl;
                memcpy(lineBuf, g_recBuf[idx] + pos, copyLen);
                lineBuf[copyLen] = '\0';

                uint16 y = bodyTop + dispLine * UART_VIEW_LINE_H + 1;
                LCD_printf(0, y, fg, BLACK, Font8x16, "%s", lineBuf);
                dispLine++;
            }
            pos    += cpl;
            absLine++;
        }
        /* 빈 줄 처리 */
        if (len == 1 && g_recBuf[idx][0] == '\0') absLine++;
    }

    /* --- 푸터: UP / DOWN / CLEAR 버튼 (3분할) --- */
    {
        uint16 btnY  = bodyBottom;
        uint16 btn1W = screenW / 3;           /* ~106px */
        uint16 btn2W = screenW * 2 / 3;       /* ~213px */
        COLOR  clrBg = CONVERT565(100, 20, 20); /* CLEAR 버튼: 어두운 빨간색 */

        FillRectangle(0,          btnY, btn1W - 1, screenH, VIEW_BTN_BG);
        FillRectangle(btn1W + 1,  btnY, btn2W - 1, screenH, VIEW_BTN_BG);
        FillRectangle(btn2W + 1,  btnY, screenW,   screenH, clrBg);

        LCD_printf(btn1W / 2 - 8,                    btnY + 14,
                   WHITE, VIEW_BTN_BG, Font8x16, "UP");
        LCD_printf(btn1W + (btn1W / 2) - 16,         btnY + 14,
                   WHITE, VIEW_BTN_BG, Font8x16, "DOWN");
        LCD_printf(btn2W + (btn1W / 2) - 20,         btnY + 14,
                   WHITE, clrBg,       Font8x16, "CLEAR");
    }
}

/* ================================================================
 * uartJsonHandleTouch — 터치로 Up/Down 스크롤 제어
 * ================================================================ */
void uartJsonHandleTouch(void)
{
    TOUCH tc = GetTouch();

    if (!tc.isClick)
    {
        g_touchHandled = FALSE;
        return;
    }
    if (g_touchHandled) return;
    g_touchHandled = TRUE;

    /* 푸터 영역(하단 40px) 밖이면 무시 */
    if (tc.point.y < (g_SCREEN_HEIGHT - UART_VIEW_FOOTER_H)) return;

    Buzzer(BUZZER_CLICK, 0);   /* UP / DOWN / CLEAR 버튼 터치 피드백 (~50ms) */

    {
        uint16 totalLines = calcTotalLines();
        uint16 visLines   = getVisibleLines();
        int16  maxScroll  = (int16) totalLines - (int16) visLines;
        if (maxScroll < 0) maxScroll = 0;

        {
            uint16 btn1W = g_SCREEN_WIDTH / 3;
            uint16 btn2W = g_SCREEN_WIDTH * 2 / 3;

            if (tc.point.x < btn1W)                  /* ▲ UP */
            {
                if (g_scrollLine > 0)
                {
                    g_scrollLine--;
                    g_autoScroll  = FALSE;
                    g_bNeedRedraw = TRUE;
                }
            }
            else if (tc.point.x < btn2W)             /* ▼ DOWN */
            {
                if (g_scrollLine < maxScroll)
                {
                    g_scrollLine++;
                    g_bNeedRedraw = TRUE;
                }
                if (g_scrollLine >= maxScroll)
                {
                    g_autoScroll = TRUE;
                }
            }
            else                                     /* ✕ CLEAR */
            {
                memset(g_recBuf,        0, sizeof(g_recBuf));
                memset(g_recTerminated, 0, sizeof(g_recTerminated));
                g_recHead         = 0;
                g_recCount        = 0;
                g_recvCount       = 0;
                memset(g_UART_buff, 0, sizeof(g_UART_buff));
                g_UART_buff_index = 0;
                g_scrollLine      = 0;
                g_autoScroll      = TRUE;
                g_bNeedRedraw     = TRUE;
            }
        }
    }
}

/* [] END OF FILE */
