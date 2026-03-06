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
#ifndef _ANDON_API_H_
#define _ANDON_API_H_
#include "main.h"
    
enum ANDON_AUTO_REQUEST {
    ANDON_NONE = 0,
    ANDON_CURRENT_TIME = 1,
    ANDON_GET_DEVICE_NAME,
    ANDON_SEND_SEWING_COUNT,
    ANDON_SEND_PATTERN_COUNT,       
    ANDON_REQUEST_LIST,
    ANDON_REQUEST_TEXT_LIST,    
    ANDON_NOTICE_CHECK,    
    END_OF_AUTO_REQUEST,
    ANDON_LIST_SEND,
    ANDON_REPLY_TEXT,    
    ANDON_LIST_STAT,
    ANDON_DEVICE_REGISTRATION,  
    ANDON_REQUEST_TEXT,
    ANDON_REQUEST_SUB_CATEGORY_NAME,    
    ANDON_PATTERN_COUNT,      
    END_OF_ANDON_DATA
};

#define MAX_LIST 5

enum REPLY_RESULT {
    RR_INIT = 0,
    RR_SUCCESS,
    RR_FAILURE,
};
/////////////////////////////////////////////////////////
//////////// LIST ////////////////////////////////////
/////////////////////////////////////////////////////////
enum LIST_ITEM_STAT {
    LIS_INIT =0,
    LIS_SENT, // 데이터 전송
 //   LIS_IDX,  // 데이터 전송 응답 받음(idx)
    LIS_W = 'W',
    LIS_C = 'C',
    LIS_P = 'P',
};
        
typedef struct tagListItem {
    uint8 bUpdated;
    uint8 status;
    char  text[32];
    int16 idx;
    int16 not_completed_qty;   
} LIST_ITEM;

typedef struct tagList
{
    uint8 bUpdated;
    uint8 noOfList;
    uint8 uSelectIndex;    
    LIST_ITEM item[MAX_LIST];
} LISTS;

extern LISTS g_Lists;

/////////////////////////////////////////////////////////
//////////// INFO ////////////////////////////////////
/////////////////////////////////////////////////////////
#define MAX_COL_NOTICE 14
#define MAX_ROW_NOTICE 20    
#define NO_OF_LINE_ONE_PAGE 9

typedef struct tagInfo {
    uint8 bUpdateDeviceInfo;
//    char DeviceName[20];
    char DeviceNo[20];    
    char LineName[20];
    uint16 DeviceIdx;
    char InstallType[20];

    char Version[20];
    
    uint16 AndonListRequestTime;
    uint16 notice_index; 
    char notice[MAX_ROW_NOTICE][MAX_COL_NOTICE+1];
    uint16 nNoOfLineForNotice;
    uint16 nNoOfPage;
} INFO;

extern INFO  g_Info;
/////////////////////////////////////////////////////////
//////////// functions //////////////////////////////////
/////////////////////////////////////////////////////////
uint8 andonLoop();
void andonResponse(char *ptrText, int16 sizeOfText);

static const char g_strServeURL[] = "/api/index.php";

void makeAndonCurrentTimeRequest();
void makeAndonSewingCount();
void makeAndonPatternCount();
void initAndon();

#endif    
/* [] END OF FILE */
