/* ========================================
 *
 * Copyright Suntech, 2023.04.10
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/

#ifndef _COUNT_H_
#define _COUNT_H_
    
#include "main.h"

// 중간에 uint32를 넣으면 죽는다 왜?
typedef struct {
    // for sewing
    uint16 sewingActualH           ; // sc  : shift actual
    uint16 sewingActualL           ; // sc  : shift actual
    uint16 sewingCycleTime         ; // ct  : cycle time
    uint16 sewingCycleTimeSumH     ; // cts : cycle time Sum
    uint16 sewingCycleTimeSumL     ; // cts : cycle time Sum    
    uint16 sewingMotorRunTime      ; // mrt : motor run time 
    uint16 sewingMotorRunTimeSumH  ; // mrt : motor run time Sum
    uint16 sewingMotorRunTimeSumL  ; // mrt : motor run time Sum    
    uint16 sewingNoStitch          ; // sq  : no of stitch
    uint16 sewingNoStitchSumH      ; // sqs : no of stitch Sum
    uint16 sewingNoStitchSumL      ; // sqs : no of stitch Sum    
    uint16 sewingTrimCount         ; // tc  : trim count
    uint16 sewingTrimCountSumH     ; // tcs : trim count Sum  
    uint16 sewingTrimCountSumL     ; // tcs : trim count Sum  
    
    // for pattern
    uint16 patternActualH          ; // sc  : shift actual
    uint16 patternActualL          ; // sc  : shift actual    
    uint16 patternCount            ; // Count
    uint16 patternNo               ; // no  : design number
    uint16 patternEmergencyTime    ; // et  : Emergency Stop Time
    uint16 patternEmergencyTimeSumH; // sc  : Emergency Stop Time Sum
    uint16 patternEmergencyTimeSumL; // sc  : Emergency Stop Time Sum    
    uint16 patternCycleTime        ; // ct  : cycle time
    uint16 patternCycleTimeSumH    ; // cts : cycle time Sum   
    uint16 patternCycleTimeSumL    ; // cts : cycle time Sum       
    uint16 patternMotorRunTime     ; // mrt : motor run time 
    uint16 patternMotorRunTimeSumH ; // mrt : motor run time Sum
    uint16 patternMotorRunTimeSumL ; // mrt : motor run time Sum    
    uint16 patternNoStitch         ; // sq  : no of stitch
    uint16 patternNoStitchSumH     ; // sqs : no of stitch Sum
    uint16 patternNoStitchSumL     ; // sqs : no of stitch Sum    
    uint16 patternStitchLength     ; // sl  : stitch Length
    uint16 patternStitchLengthSumH ; // sls : stitch Length Sum  
    uint16 patternStitchLengthSumL ; // sls : stitch Length Sum        
    uint16 patternTrimCount        ; // tc  : trim count
    uint16 patternTrimCountSumH    ; // tcs : trim count Sum 
    uint16 patternTrimCountSumL    ; // tcs : trim count Sum      
    uint16 patternSPM              ; // spm : stitching Speed       
    
    uint16 andonEntry;
    uint16 reset; // if reset : 0
} COUNT;
    
void initCount();    
void CountLoop();
void ResetCount();
void SetCountLoop();

extern void (*CountFunc)();
//extern COUNT *g_ptrCount;
extern uint8 g_updateCountMenu;
COUNT *getCount();

void WorkingTimeCount();
uint32 GetWorkingTimeCount();
void ResetWorkingTimeCount();
#endif
    
/* [] END OF FILE */
