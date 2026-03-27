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
 * UART TEST 뷰어 메뉴 — EMBROIDERY_S 전용
 *
 * 동작 방식 (미러):
 *   uartJson.c 의 파서가 패킷 수신 성공 시 uartTestAddLine() 을 호출한다.
 *   이 파일은 UART 를 직접 읽지 않고 LCD 표시만 담당한다.
 *   파싱 및 서버 전송은 항상 정상 동작한다.
 *
*/
#include "uartTestMenu.h"
#include "lib/widget.h"
#include "lib/UI.h"
#include "menuDesign.h"

/* ── 뷰어 상수 ── */
#define UART_VIEW_MAX_REC        5
#define UART_VIEW_REC_LEN     1024
#define UART_VIEW_HEADER_H      40
#define UART_VIEW_FOOTER_H      40
#define UART_VIEW_LINE_H        18
#define UART_VIEW_FONT_W         8

#define VIEW_HDR_BG   CONVERT565(  0,   0, 100)
#define VIEW_BTN_BG   CONVERT565( 40,  40,  80)

/* ── 전역 플래그 ── */
uint8 g_bUartTestMode = FALSE;

/* ── 내부 상태 ── */
static char   g_recBuf[UART_VIEW_MAX_REC][UART_VIEW_REC_LEN];
static uint8  g_recHead    = 0;
static uint8  g_recCount   = 0;
static uint16 g_recvCount  = 0;
static int16  g_scrollLine  = 0;
static uint8  g_bNeedRedraw = TRUE;
static uint8  g_touchHandled = FALSE;
static uint8  g_autoScroll   = TRUE;

/* ── 내부 함수 선언 ── */
static uint16 getCharsPerLine(void);
static uint16 getVisibleLines(void);
static uint16 calcTotalLines(void);
static void   uartTestDrawScreen(void);
static void   uartTestHandleTouch(void);

/* ================================================================
 * 헬퍼
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
        uint16 len = (uint16)strlen(g_recBuf[idx]);
        if (len == 0) len = 1;
        total += (len + cpl - 1) / cpl;
    }
    return total;
}

/* ================================================================
 * uartTestAddLine — uartJson.c 파싱 성공 시 미러 호출
 * ================================================================ */
void uartTestAddLine(const char *line)
{
    uint8 writeIdx;

    if (g_recCount < UART_VIEW_MAX_REC)
    {
        writeIdx = (g_recHead + g_recCount) % UART_VIEW_MAX_REC;
        g_recCount++;
    }
    else
    {
        writeIdx  = g_recHead;
        g_recHead = (g_recHead + 1) % UART_VIEW_MAX_REC;
    }

    strncpy(g_recBuf[writeIdx], line, UART_VIEW_REC_LEN - 1);
    g_recBuf[writeIdx][UART_VIEW_REC_LEN - 1] = '\0';
    g_recvCount++;

    if (g_autoScroll)
    {
        uint16 totalLines = calcTotalLines();
        uint16 visLines   = getVisibleLines();
        g_scrollLine = (totalLines > visLines)
                       ? (int16)(totalLines - visLines) : 0;
    }
    g_bNeedRedraw = TRUE;
}

/* ================================================================
 * uartTestDrawScreen — LCD 뷰어 화면 그리기
 *
 *  [y:  0 ~  39]        헤더: 타이틀 + 수신 카운트
 *  [y: 40 ~ (H-40)]    바디: 수신 문자열
 *  [y: (H-40) ~ H]     푸터: [UP] [DOWN] [CLEAR]
 * ================================================================ */
static void uartTestDrawScreen(void)
{
    if (!g_bNeedRedraw) return;
    g_bNeedRedraw = FALSE;

    uint16 screenW    = g_SCREEN_WIDTH;
    uint16 screenH    = g_SCREEN_HEIGHT;
    uint16 bodyTop    = UART_VIEW_HEADER_H;
    uint16 bodyBottom = screenH - UART_VIEW_FOOTER_H;
    uint16 cpl        = getCharsPerLine();
    uint16 visLines   = getVisibleLines();

    /* 헤더 */
    FillRectangle(0, 0, screenW, bodyTop, VIEW_HDR_BG);
    LCD_printf(4, 14, WHITE, VIEW_HDR_BG, Font8x16,
               "EMBROIDERY UART RX TEST  [%u]", g_recvCount);

    /* 바디 배경 */
    FillRectangle(0, bodyTop, screenW, bodyBottom, BLACK);

    /* 텍스트 렌더링 */
    int16  absLine  = 0;
    uint16 dispLine = 0;
    char   lineBuf[41];
    uint8  r;

    for (r = 0; r < g_recCount && dispLine < visLines; r++)
    {
        uint8  idx = (g_recHead + r) % UART_VIEW_MAX_REC;
        uint16 len = (uint16)strlen(g_recBuf[idx]);
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
                LCD_printf(0, y, WHITE, BLACK, Font8x16, "%s", lineBuf);
                dispLine++;
            }
            pos    += cpl;
            absLine++;
        }
        if (len == 1 && g_recBuf[idx][0] == '\0') absLine++;
    }

    /* 푸터: QUIT / UP / DOWN / CLEAR (4등분) */
    {
        uint16 btnY  = bodyBottom;
        uint16 bw    = screenW / 4;          /* 버튼 1개 너비 */
        COLOR  clrQuit  = CONVERT565( 20, 100,  20);
        COLOR  clrClear = CONVERT565(100,  20,  20);

        FillRectangle(0,          btnY, bw     - 1, screenH, clrQuit);
        FillRectangle(bw      + 1, btnY, bw * 2 - 1, screenH, VIEW_BTN_BG);
        FillRectangle(bw * 2 + 1, btnY, bw * 3 - 1, screenH, VIEW_BTN_BG);
        FillRectangle(bw * 3 + 1, btnY, screenW,     screenH, clrClear);

        LCD_printf(bw / 2 - 16,           btnY + 14, WHITE, clrQuit,    Font8x16, "QUIT");
        LCD_printf(bw + bw / 2 - 8,       btnY + 14, WHITE, VIEW_BTN_BG, Font8x16, "UP");
        LCD_printf(bw * 2 + bw / 2 - 16,  btnY + 14, WHITE, VIEW_BTN_BG, Font8x16, "DOWN");
        LCD_printf(bw * 3 + bw / 2 - 20,  btnY + 14, WHITE, clrClear,   Font8x16, "CLEAR");
    }
}

/* ================================================================
 * uartTestHandleTouch — UP / DOWN / CLEAR 터치 처리
 * ================================================================ */
static void uartTestHandleTouch(void)
{
    TOUCH tc = GetTouch();

    if (!tc.isClick)
    {
        g_touchHandled = FALSE;
        return;
    }
    if (g_touchHandled) return;
    g_touchHandled = TRUE;

    /* 푸터 영역 밖이면 무시 */
    if (tc.point.y < (g_SCREEN_HEIGHT - UART_VIEW_FOOTER_H)) return;

    Buzzer(BUZZER_CLICK, 0);

    {
        uint16 totalLines = calcTotalLines();
        uint16 visLines   = getVisibleLines();
        int16  maxScroll  = (int16)totalLines - (int16)visLines;
        if (maxScroll < 0) maxScroll = 0;

        uint16 bw   = g_SCREEN_WIDTH / 4;  /* QUIT=0~bw, UP=bw~2bw, DOWN=2bw~3bw, CLEAR=3bw~ */

        if (tc.point.x < bw)             /* QUIT — doUartTestMenu 에서 처리, 여기선 무시 */
        {
            /* nothing — doUartTestMenu case FALSE 가 먼저 감지함 */
        }
        else if (tc.point.x < bw * 2)   /* ▲ UP */
        {
            if (g_scrollLine > 0)
            {
                g_scrollLine--;
                g_autoScroll  = FALSE;
                g_bNeedRedraw = TRUE;
            }
        }
        else if (tc.point.x < bw * 3)   /* ▼ DOWN */
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
        else                             /* ✕ CLEAR */
        {
            memset(g_recBuf, 0, sizeof(g_recBuf));
            g_recHead         = 0;
            g_recCount        = 0;
            g_recvCount       = 0;
            g_scrollLine      = 0;
            g_autoScroll      = TRUE;
            g_bNeedRedraw     = TRUE;
        }
    }
}

/* ================================================================
 * doUartTestMenu — 메뉴 노드 함수
 * ================================================================ */
int doUartTestMenu(void *this, uint8 reflash)
{
    switch (reflash)
    {
        case TRUE:   /* 화면 진입 */
            g_bUartTestMode = TRUE;
            memset(g_recBuf, 0, sizeof(g_recBuf));
            g_recHead      = 0;
            g_recCount     = 0;
            g_recvCount    = 0;
            g_scrollLine   = 0;
            g_autoScroll   = TRUE;
            g_bNeedRedraw  = TRUE;
            g_touchHandled = FALSE;
            uartTestDrawScreen();
            break;

        case FALSE:  /* 반복 호출 */
        {
            TOUCH tc = GetTouch();
            /* QUIT 버튼: 푸터 좌측 1/4 영역 터치 */
            if (tc.isClick
                && tc.point.y >= (g_SCREEN_HEIGHT - UART_VIEW_FOOTER_H)
                && tc.point.x <  (g_SCREEN_WIDTH  / 4))
            {
                g_bUartTestMode = FALSE;
                Buzzer(BUZZER_CLICK, 0);
                DrawHeader(); /* UART TEST가 덮어쓴 표준 헤더 영역(y:0~39) 복원 */
                return MENU_RETURN_PARENT;
            }
            uartTestDrawScreen();
            uartTestHandleTouch();
            break;
        }
    }
    return MENU_RETURN_THIS;
}

/* [] END OF FILE */
