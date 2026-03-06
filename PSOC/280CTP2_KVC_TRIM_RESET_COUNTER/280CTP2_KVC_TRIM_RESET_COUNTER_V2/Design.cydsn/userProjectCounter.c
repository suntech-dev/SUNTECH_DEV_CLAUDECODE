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
#include "userProjectCounter.h"
#include "lib/externalFlash.h"
 
#ifdef USER_PROJECT_TRIM_COUNT
 
MACHINE_PARAMETER *g_ptrMachineParameter = NULL;
MACHINE_PARAMETER g_DefaultMachineParameter = {
    .setTrimCount = MAX_SET_TRIM_COUNT,
  //  .curTrimCount = 0,
};
    
void initUserProject()
{
    g_ptrMachineParameter = (MACHINE_PARAMETER *) getExternalConfigUserData();
}

void SetUserProjectDefaultExternalFlashConfig() // refer to externalFlash.h
{
    memcpy(g_ptrMachineParameter,&g_DefaultMachineParameter,sizeof(MACHINE_PARAMETER));
   // g_ptrMachineParameter->machineType = PATTERN_MACHINE;
}

uint8 userProjectDataValidation()
{
    if(g_ptrMachineParameter->setTrimCount < 0 || g_ptrMachineParameter->setTrimCount > MAX_SET_TRIM_COUNT) return FALSE;
    
  //      printf("g_ptrMachineParameter. %d\r\n",g_ptrMachineParameter->curTrimCount);
 //   if(g_ptrMachineParameter->curTrimCount < 0 || g_ptrMachineParameter->curTrimCount > MAX_SET_TRIM_COUNT) return FALSE;    
    
    return TRUE;
}

#endif
/* [] END OF FILE */
