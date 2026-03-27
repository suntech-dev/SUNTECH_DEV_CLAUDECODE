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
#ifndef _DOWNTIME_H_
#define _DOWNTIME_H_
#include "main.h"
#include "lib/menu.h"
#define MAX_DOWNTIME_LIST 20

typedef struct tagDownTimeListItem {
    uint8 bUpdated;
    uint8 status;
    char  text[32];
    int16 idx;
    int16 not_completed_qty;   
} DOWNTIME_LIST_ITEM;

typedef struct tagDownTimeList
{
    uint8 bUpdated;
    uint8 noOfList;
    uint8 uSelectIndex;    
    DOWNTIME_LIST_ITEM item[MAX_DOWNTIME_LIST];
} DOWNTIME_LISTS;

extern DOWNTIME_LISTS g_DownTimeLists;

void makeDownTimeList();
uint8 downTimeParsing(char *jsonString, int16 sizeOfJson);
uint8 downTimeRequestItem(char *jsonString, int16 sizeOfJson);
int doDowntime(void *this, uint8 reflash);
int doDowntimeRequest(void *this, uint8 reflash);
void ForcefullyMarkDowntimeAsComplete();
#endif
/* [] END OF FILE */
