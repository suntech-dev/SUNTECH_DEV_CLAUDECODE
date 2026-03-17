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
#include "defective.h"
#include "lib/WIFI.h"
#include "lib/menu.h"
#include "lib/widget.h"
#include "lib/externalFlash.h"
#include "andonApi.h"
#include "andonJson.h"
#include "andonMessageQueue.h"
#include "jsonUtil.h"

DEFECTIVE_LISTS g_DefectiveLists;

void makeDefectiveList()
{
    enQueueANDON_printf(DEFECTIVE_REQUEST_LIST,"get_defectiveList&mac=%s",
        g_network.MAC
    );
}
void makeDefectiveWarningRequest(int idx)
{
    enQueueANDON_printf(DEFECTIVE_REQUEST_ITEM,"send_defective_warning&mac=%s&defective_idx=%d",
        g_network.MAC,
        idx
    );
    printf("Warning Index : %d\r\n", idx);
}

uint8 defectiveParsing(char *jsonString, int16 sizeOfJson)
{
    /* 공통 파서 위임 (jsonUtil.c parseGenericList) — 이슈 #10 중복 제거 */
    return parseGenericList(jsonString, sizeOfJson,
                            (GENERIC_LISTS *)&g_DefectiveLists,
                            "defective_idx", "defective_name");
}

void SetDefectiveListButtons(LIST_MENU *menu, char *title, DEFECTIVE_LISTS *list);

int doDefective(void *this, uint8 reflash)
{
    MENUNODE *thisMenu = (MENUNODE *) this;
    uint8 bUpdate = FALSE;
    static uint16 page = 0;

    if(g_DefectiveLists.bUpdated)
    {
        g_DefectiveLists.bUpdated = FALSE;
        bUpdate = TRUE;
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
                            g_DefectiveLists.uSelectIndex = idx;
                            return 0;
                        }
                }
            }
            break;
    }

    if(bUpdate)
    {
        char *rightButtonText = "NEXT";
        SetDefectiveListButtons(&g_ListMenu, NULL, &g_DefectiveLists);
        if (isFirtstPage(&g_ListMenu) != TRUE &&  isLastPage(&g_ListMenu) == TRUE) rightButtonText = "FIRST";
        UpdateDrawListButtons(&g_ListMenu,NO_SELECT);
        SetDrawBottomButtons("QUIT", rightButtonText, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_ORANGE);
        bUpdate = FALSE;
    }
    return MENU_RETURN_THIS;
}

void SetDefectiveListButtons(LIST_MENU *menu, char *title, DEFECTIVE_LISTS *list)
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

        DEFECTIVE_LIST_ITEM *item = &list->item[textIdx];
        int16 not_completed_qty = item->not_completed_qty;

        if(not_completed_qty == 0)
            SetButtonStyleColor(btn, BUTTON_STYLE_LIST);
        else
            SetButtonStyleColor(btn, BUTTON_STYLE_R_RED);

        btn->align = TEXT_ALIGN_LEFT;

   //     sprintf(menu->listText[textIdx],"%-10s",item->text);
        //sprintf(menu->listText[textIdx],"%-10s %2d",item->text, item->not_completed_qty);
        sprintf(menu->listText[textIdx],"%-11s %2d",item->text, item->not_completed_qty);
      //  printf("%s %d\r\n", menu->listText[textIdx], textIdx);
        yPos += g_DEFAULT_BUTTON_HEIGHT;
    }

    if(withScroll) SetDrawScrollButton(menu);
}

int doDefectiveRequest(void *this, uint8 reflash)
{
    MENUNODE *thisMenu = (MENUNODE *) this;
    static LIST_MENU menu;
    static char *listText[] = {"WARNING"};
    static uint16 indexSelect;
    uint8 bUpdate = FALSE;

    DEFECTIVE_LIST_ITEM *item = &g_DefectiveLists.item[g_DefectiveLists.uSelectIndex];
    int16 not_completed_qty = item->not_completed_qty;


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
                                        case 0: makeDefectiveWarningRequest(item->idx);
                                        break;
                                      //  case 1: makeDownTimeCompleteRequest(item->idx);
                                      //  break;
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
        sprintf(msg,"%s %d",item->text, item->not_completed_qty);
        SetDrawListButtons(&menu,msg,listText,1 , BUTTON_STYLE_LIST);
        UpdateDrawListButtons(&menu,NO_SELECT);
        SetDrawBottomButtons("QUIT", "SEND", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);
        bUpdate = FALSE;
    }
    return MENU_RETURN_THIS;
}

/* [] END OF FILE */
