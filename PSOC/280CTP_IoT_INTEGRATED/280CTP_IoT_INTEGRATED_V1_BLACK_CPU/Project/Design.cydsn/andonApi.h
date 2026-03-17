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
        ANDON_DEVICE_REGISTRATION,
        ANDON_GET_DEVICE_NAME,   
        ANDON_REQUEST_ITEM,
        ANDON_REQUEST_LIST,
        ANDON_REQUEST_TEXT_LIST,   
        ANDON_REQUEST_TARGET, 
        ANDON_REQUEST_TEXT,
        ANDON_REQUEST_SUB_CATEGORY_NAME,
        ANDON_LIST_SEND,    
        ANDON_LIST_STAT,
        ANDON_NOTICE_CHECK,
        ANDON_REPLY_TEXT,     
        ANDON_PATTERN_COUNT,  
        ANDON_START, 
        ANDON_SEND_SEWING_COUNT,
        ANDON_SEND_PATTERN_COUNT,
        ANDON_UPDATE_RUNTIME_SUM,  
        DOWNTIME_REQUEST_LIST,
        DOWNTIME_REQUEST_ITEM,
        DEFECTIVE_REQUEST_LIST,
        DEFECTIVE_REQUEST_ITEM,    
        END_OF_ANDON_DATA,    
        END_OF_AUTO_REQUEST
};

#define MAX_LIST 5

enum REPLY_RESULT {
    RR_INIT = 0,
    RR_SUCCESS,
    RR_FAILURE,
};

////////////// 
//// LIST //// 
//////////////
enum LIST_ITEM_STAT {
    LIS_INIT =0,
    LIS_SENT, // 데이터 전송
    //LIS_IDX,  // 데이터 전송 응답 받음(idx)
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
} ANDON_LIST_ITEM;

typedef struct tagList
{
    uint8 bUpdated;
    uint8 noOfList;
    uint8 uSelectIndex;    
    ANDON_LIST_ITEM item[MAX_LIST];
} ANDON_LISTS;

extern ANDON_LISTS g_AndonLists;

////////////// 
//// Info ////
//////////////
#define MAX_COL_NOTICE 14
#define MAX_ROW_NOTICE 20    
#define NO_OF_LINE_ONE_PAGE 9

typedef struct tagInfo {
    uint8 bUpdateDeviceInfo;
    //char DeviceName[20];
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

///////////////////
//// functions ////
///////////////////
uint8 andonLoop();
void andonResponse(char *ptrText, int16 sizeOfText);

/* API 엔드포인트는 server.h의 DEFAULT_API_ENDPOINT 에서 관리
 * andonApi.c 에서 DEFAULT_API_ENDPOINT 를 직접 사용하므로 이 변수는 제거됨
 *
 * 이전 이력:
 * //static const char g_strServeURL[] = "/csg/api/index.php";
 * //static const char g_strServeURL[] = "/api/index.php";
 * //static const char g_strServeURL[] = "/robot2/api/sewing.php";
 * //static const char g_strServeURL[] = "/samho/api/sewing.php";
 * //static const char g_strServeURL[] = "/api/sewing.php";   ← server.h DEFAULT_API_ENDPOINT 으로 이동
 */

void makeAndonCurrentTimeRequest();
void makeAndonStart();
void makeAndonList();
void makeAndonWarningRequest(int idx);
void makeAndonCompleteRequest(int idx);
void makeAndonPatternCount();
void makeRequestTarget();
void updateRuntimeSum(uint32 runtimeSum, uint32 elaspedTime);
void initAndon();

extern uint8 g_bTargetReceived;
extern uint8 g_index_request_interval;
extern uint8 g_bReceivedAndonStart;
#endif    
/* [] END OF FILE */
