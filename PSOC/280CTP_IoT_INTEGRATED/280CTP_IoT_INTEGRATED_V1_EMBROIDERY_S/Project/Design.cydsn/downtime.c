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
#include "downtime.h"
#include "lib/WIFI.h"
#include "lib/menu.h"
#include "lib/widget.h"
#include "lib/externalFlash.h"
#include "lib/sysTick.h"
#include "andonApi.h"
#include "andonJson.h"
#include "andonMessageQueue.h"
#include "jsonUtil.h"


DOWNTIME_LISTS g_DownTimeLists;

void makeDownTimeList()
{
    enQueueANDON_printf(DOWNTIME_REQUEST_LIST,"get_downtimeList&mac=%s",
        g_network.MAC
    );
}
void makeDownTimeWarningRequest(int idx)
{
    enQueueANDON_printf(DOWNTIME_REQUEST_ITEM,"send_downtime_warning&mac=%s&downtime_idx=%d",
        g_network.MAC,
        idx
    );
    printf("Warning Index : %d\r\n", idx);
}

void makeDownTimeCompleteRequest(int idx)
{
    enQueueANDON_printf(DOWNTIME_REQUEST_ITEM,"send_downtime_completed&mac=%s&downtime_idx=%d",
        g_network.MAC,
        idx
    );
    printf("Complete Index : %d\r\n", idx);
}


uint8 downTimeParsing(char *jsonString, int16 sizeOfJson)
{
    /* 공통 파서 위임 (jsonUtil.c parseGenericList) — 이슈 #10 중복 제거 */
    return parseGenericList(jsonString, sizeOfJson,
                            (GENERIC_LISTS *)&g_DownTimeLists,
                            "downtime_idx", "downtime_name");
}

void SetDowntimeListButtons(LIST_MENU *menu, char *title, DOWNTIME_LISTS *list);

uint8 bIsOnlyExistWarning(DOWNTIME_LISTS *list, int index)
{
 //   uint8 isThisWarning = FALSE;
    if(list->noOfList <= index) return FALSE;

    for(int i=0; i < list->noOfList; i++) if(i != index)
    {
        if(list->item[i].not_completed_qty > 0) return FALSE;
    }

    return TRUE;
}

void ForcefullyMarkDowntimeAsComplete()
{
    for(int i=0; i < g_DownTimeLists.noOfList; i++)
    {
        if(g_DownTimeLists.item[i].not_completed_qty > 0)
        {
            makeDownTimeCompleteRequest(i);
            break;
        }
    }
}

int doDowntime(void *this, uint8 reflash)
{
    MENUNODE *thisMenu = (MENUNODE *) this;
    uint8 bUpdate = FALSE;
    static uint16 page = 0;
    static uint8 showWarning = 0;
    static uint8 WarningTimerIndex = 0xFF;

    if(g_DownTimeLists.bUpdated)
    {
        g_DownTimeLists.bUpdated = FALSE;
        bUpdate = TRUE;
    }

    switch(showWarning)
    {
        case 1:
            if(WarningTimerIndex == 0xFF) WarningTimerIndex = registerCounter_1ms(2000);
            resetCounter_1ms(WarningTimerIndex);
            Buzzer(BUZZER_WARNING,4);

            RECT bodyRect = GetBodyArea();
            EraseBlankArea(bodyRect.top,FALSE);
            int y = bodyRect.top + 35;
            LCD_printf(30, y += 35, WHITE, BLACK, Grotesk16x32, "Already");
            LCD_printf(30, y += 35, WHITE, BLACK, Grotesk16x32, "downtime");
            LCD_printf(30, y += 35, WHITE, BLACK, Grotesk16x32, "in progress");
           // printf("Already downtime in progress\r\n");
            showWarning++;
        break;
        case 2:
            if(isFinishCounter_1ms(WarningTimerIndex))
            {
                Buzzer(BUZZER_STOP,0);
                showWarning = 0;
                bUpdate = TRUE;
            }
            break;
    }

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
        {
           bUpdate = TRUE;
           g_ListMenu.curPage = page;
        }
        break;

        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;

                uint16 idx = getIndexOfClickedListMenu(&tc, &g_ListMenu);

                switch(idx)
                {
                    case NO_CLICK :
                        {
                            switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                            {
                                case BOTTOM_LEFT:
                                    page = g_ListMenu.curPage;
                                    return MENU_RETURN_PARENT;
                                     break;
                                case BOTTOM_RIGHT:
                                {
                                    if (isFirtstPage(&g_ListMenu) != TRUE &&  isLastPage(&g_ListMenu) == TRUE) g_ListMenu.curPage=0;
                                    else g_ListMenu.curPage++;
                                    page = g_ListMenu.curPage;
                                    bUpdate = TRUE;
                                    break;
                                }
                            }
                            break;
                        }
                    default:
                        if(idx < g_ListMenu.sizeOfList)
                        {
                            if(bIsOnlyExistWarning(&g_DownTimeLists,idx))
                            {
                               g_DownTimeLists.uSelectIndex = idx;
                               return 0;
                            } else {
                                showWarning = 1;
                                return MENU_RETURN_THIS;
                            }
                        }
                }
            }
            break;
    }

    if(bUpdate)
    {
        char *rightButtonText = "NEXT";
        SetDowntimeListButtons(&g_ListMenu, NULL, &g_DownTimeLists);
        if (isFirtstPage(&g_ListMenu) != TRUE &&  isLastPage(&g_ListMenu) == TRUE) rightButtonText = "FIRST";
        UpdateDrawListButtons(&g_ListMenu,NO_SELECT);
        SetDrawBottomButtons("QUIT", rightButtonText, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_ORANGE);
        bUpdate = FALSE;
    }
    return MENU_RETURN_THIS;
}

void SetDowntimeListButtons(LIST_MENU *menu, char *title, DOWNTIME_LISTS *list)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
    RECT bodyRect = GetBodyArea();
    uint16 yPos = bodyRect.top;
    uint16 yPosTitle;

    menu->noOfDisplayButton = getMaxNoOfItemInListButton();

    if(title != NULL) menu->noOfDisplayButton--;

    menu->sizeOfList = list->noOfList;

    uint8 withScroll = FALSE;// isExistScroll(menu);

    if((yPosTitle = DrawTitleButton(title,withScroll)) > 0) yPos = yPosTitle;

    for(int i=0; i < menu->noOfDisplayButton; i++)
    {
        BUTTON *btn = &menu->btns[i];
        SetDefaultButtonStyle(btn);
        SetButtonSize      (btn, yPos, g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y,withScroll);

        uint16 textIdx = menu->noOfDisplayButton * menu->curPage + i;
        if(textIdx >= menu->sizeOfList) break;

        DOWNTIME_LIST_ITEM *item = &list->item[textIdx];
        int16 not_completed_qty = item->not_completed_qty;

        if(not_completed_qty == 0)
            SetButtonStyleColor(btn, BUTTON_STYLE_LIST);
        else
            SetButtonStyleColor(btn, BUTTON_STYLE_R_RED);

        //btn->font  = Font16x24;        /* SetButtonStyleColor 이후 설정 — 내부에서 SetDefaultButtonStyle 호출로 덮어씌워지므로 */
        btn->align = TEXT_ALIGN_LEFT;

        sprintf(menu->listText[textIdx],"%-10s",item->text);
      //  printf("%s %d\r\n", menu->listText[textIdx], textIdx);
        yPos += g_DEFAULT_BUTTON_HEIGHT;
    }

    if(withScroll) SetDrawScrollButton(menu);
}

int doDowntimeRequest(void *this, uint8 reflash)
{
    MENUNODE *thisMenu = (MENUNODE *) this;
    static LIST_MENU menu;
    static char *listText[] = {"WARNING"};
    static uint16 indexSelect;
    uint8 bUpdate = FALSE;

    DOWNTIME_LIST_ITEM *item = &g_DownTimeLists.item[g_DownTimeLists.uSelectIndex];
    int16 not_completed_qty = item->not_completed_qty;
    if(not_completed_qty == 0)
    {
        indexSelect = 0;
        listText[0] = "WARNING";
    }
    else
    {
        indexSelect = 1;
        listText[0] = "COMPLETE";
    }

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                menu.curPage = 0;
                bUpdate = TRUE;
            }
            break;

        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;

                uint16 idx = getIndexOfClickedListMenu(&tc, &menu);

                switch(idx)
                {
                    case NO_CLICK :
                        {
                            switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                            {
                                case BOTTOM_LEFT:
                                    return MENU_RETURN_PARENT;
                                case BOTTOM_RIGHT:
                                {
                                    switch(indexSelect)
                                    {
                                        case 0: makeDownTimeWarningRequest(item->idx);
                                        break;
                                        case 1: makeDownTimeCompleteRequest(item->idx);
                                        break;
                                    }
                                    return MENU_RETURN_PARENT;
                                }
                            }
                            break;
                        }
                    default:
                        break;
                }
            }
            break;
    }
    if(bUpdate)
    {
        char msg[MAX_BUTTON_TEXT_SIZE];
        sprintf(msg,"%s",item->text);
        SetDrawListButtons(&menu,msg,listText,1 , BUTTON_STYLE_LIST);
        UpdateDrawListButtons(&menu,NO_SELECT);
        SetDrawBottomButtons("QUIT", "SEND", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);
        bUpdate = FALSE;
    }
    return MENU_RETURN_THIS;
}


uint8 downTimeRequestItem(char *jsonString, int16 sizeOfJson)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[80]; /* 최대 토큰 수: 80 (10개 항목 JSON = 73 tokens, 여유 7) */

    jsmn_init(&p);
    r = jsmn_parse(&p, jsonString, sizeOfJson, t, sizeof(t) / sizeof(t[0]));
    printf("%s\n",jsonString);
    if (r < 0) {
        #ifdef DEBUG_ANDON_JSON
            printf("Failed to parse JSON: %d\r\n", r);
        #endif
        return FALSE;
    }

    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        #ifdef DEBUG_ANDON_JSON
            printf("Object expected type: %d\r\n", t[0].type);
        #endif
        return FALSE;
    }
    char buff[20];
    for (i = 1; i < r; i++)
    {
    }

    return TRUE;
}
/* [] END OF FILE */
