
/* ========================================
 *
 * Copyright Suntech, 2023.03.30
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "userProjectPatternSewing.h"
#include "lib/externalFlash.h"

#ifdef USER_PROJECT_PATTERN_SEWING_MACHINE

MACHINE_PARAMETER *g_ptrMachineParameter = NULL;
MACHINE_PARAMETER g_DefaultMachineParameter = {
    .machineType = PATTERN_MACHINE,
    
    // pattern machine
    ///.patternTarget    = 28800,
    .patternTarget    = 1000,
    .patternPairCount =    2,
    .patternPair      =   25,
    // sewing machine
    ///.sewingTarget     = 28800,
    .sewingTarget     = 1000,
    .sewingPairTrim   =    2,
    .sewingPair       =    5,
    
    .andon_enable     =    TRUE,
    .current_enable   =    FALSE,
    .current_sensor_threshold = 13,
    .bAutoReset       =    ON,
};
    
void initUserProject()
{
    g_ptrMachineParameter = (MACHINE_PARAMETER *) getExternalConfigUserData();
}

void SetUserProjectDefaultExternalFlashConfig() // refer to externalFlash.h
{
    memcpy(getExternalConfigUserData(), &g_DefaultMachineParameter, sizeof(g_DefaultMachineParameter));
}

uint8 userProjectDataValidation()
{
 //   printf("target = %u\r\n", g_ptrMachineParameter->patternTarget);
    switch(g_ptrMachineParameter->machineType)
    {
        case PATTERN_MACHINE:
            if(g_ptrMachineParameter->patternTarget  >= UINT16_MAX || g_ptrMachineParameter->patternTarget == 0)
                g_ptrMachineParameter->patternTarget = g_DefaultMachineParameter.patternTarget;
            if(g_ptrMachineParameter->patternPairCount > 99|| g_ptrMachineParameter->patternPairCount == 0)
                g_ptrMachineParameter->patternPairCount = g_DefaultMachineParameter.patternPairCount;
            if(g_ptrMachineParameter->patternPair > 999    || g_ptrMachineParameter->patternPair == 0)
                g_ptrMachineParameter->patternPair = g_DefaultMachineParameter.patternPair;    
            break;
        case SEWING_MACHINE:
            if(g_ptrMachineParameter->sewingTarget  >= UINT16_MAX || g_ptrMachineParameter->sewingTarget == 0)
                g_ptrMachineParameter->sewingTarget = g_DefaultMachineParameter.sewingTarget;
            if(g_ptrMachineParameter->sewingPairTrim > 99|| g_ptrMachineParameter->sewingPairTrim == 0)
                g_ptrMachineParameter->sewingPairTrim = g_DefaultMachineParameter.sewingPairTrim;
            if(g_ptrMachineParameter->sewingPair > 999    || g_ptrMachineParameter->sewingPair == 0)
                g_ptrMachineParameter->sewingPair = g_DefaultMachineParameter.sewingPair;              
            break;
        default:
            memcpy(&g_ptrMachineParameter->machineType, &g_DefaultMachineParameter, sizeof(MACHINE_PARAMETER));
            g_ptrMachineParameter->machineType = PATTERN_MACHINE;            
            break;
    }
    
   
    
    
    //g_ptrMachineParameter->machineType = SEWING_MACHINE;
   // if(ret==FALSE) SaveExternalFlashConfig();
    return TRUE;
}

#endif
/* [] END OF FILE */
