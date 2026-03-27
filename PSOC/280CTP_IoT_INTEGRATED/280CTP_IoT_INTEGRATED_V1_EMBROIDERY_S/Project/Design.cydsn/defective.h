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
#ifndef _DEFECTIVE_H_
#define _DEFECTIVE_H_
#include "main.h"
#include "lib/menu.h"
#define MAX_DEFECTIVE_LIST 20

typedef struct tagDefectiveListItem {
    uint8 bUpdated;
    uint8 status;
    char  text[32];
    int16 idx;
    int16 not_completed_qty;   
} DEFECTIVE_LIST_ITEM;

typedef struct tagDefectiveList
{
    uint8 bUpdated;
    uint8 noOfList;
    uint8 uSelectIndex;    
    DEFECTIVE_LIST_ITEM item[MAX_DEFECTIVE_LIST];
} DEFECTIVE_LISTS;

extern DEFECTIVE_LISTS g_DefectiveLists;

void makeDefectiveList();
uint8 defectiveParsing(char *jsonString, int16 sizeOfJson);
uint8 defectiveRequestItem(char *jsonString, int16 sizeOfJson);
int doDefective(void *this, uint8 reflash);
int doDefectiveRequest(void *this, uint8 reflash);

#endif
/* [] END OF FILE */
