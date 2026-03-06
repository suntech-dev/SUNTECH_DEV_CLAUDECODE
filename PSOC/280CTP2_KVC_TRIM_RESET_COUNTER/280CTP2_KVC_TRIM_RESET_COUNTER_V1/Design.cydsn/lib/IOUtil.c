/* ========================================
 *
 * Copyright Suntech, 2023.02.01
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "main.h"
#include "IOUtil.h"
#include "sysTick.h"

void initIOUtil()
{
   initIO();
   initIOT();

   for(int i=0; i < 10; i++) glitchFiltering();
}

void updateIOStat()
{
    doOutput();
        
    glitchFiltering();
    updateIO(); 
    updateIOTimer();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Input-Output  Utility
//////////////////////////////////////////////////////////////////////////////////////////
SIOUtil g_IOUtil[MAX_IO];

void initIO()
{
    for (int i = 0; i < MAX_IO; i++)
    {
        g_IOUtil[i].previous = 0;
        g_IOUtil[i].current  = 0;
        g_IOUtil[i].upEdge   = 0;
        g_IOUtil[i].downEdge = 0;
        g_IOUtil[i].activate = 1;        
    }
}

void glitchFiltering()
{
    unsigned long v;   // int is 2 byte in arduino....
    static unsigned long keep=0;
    static unsigned long arrayIn[4], arrayIndex=0;
             
    // Glitch filtering......................................
    arrayIn[arrayIndex++] = getSensor();
    arrayIndex %= 4; // 0, 1, 2, 3, 0, 1, 2, .....

    keep |= arrayIn[0] & arrayIn[1] & arrayIn[2] & arrayIn[3]; 
    keep &= arrayIn[0] | arrayIn[1] | arrayIn[2] | arrayIn[3]; 
    //....................................................... 

    v = keep;
    for (int i = START_INPUT; i < END_OF_DEFIO; i++)
    {
        I(i) = (v & 1);// ? ON: OFF;
        v >>= 1;
    }         
}

void updateIO()
{  
    for (int i = 0; i < MAX_IO; i++) if(g_IOUtil[i].activate)
    {
        g_IOUtil[i].upEdge   = ((!g_IOUtil[i].previous) &  g_IOUtil[i].current);
        g_IOUtil[i].downEdge = ( g_IOUtil[i].previous & !g_IOUtil[i].current);
        g_IOUtil[i].previous =  g_IOUtil[i].current;
    }  
}
//////////////////////////////////////////////////////////////////////////////////////////
// Input-Output Timer Utility
//////////////////////////////////////////////////////////////////////////////////////////
SIOTUtil g_IOTUtil[MAX_IOT];

void initIOT()
{
    for (int i = 0; i < MAX_IOT; i++)
    {
        g_IOTUtil[i].sensor      = OFF;
        g_IOTUtil[i].previous    = OFF;
        g_IOTUtil[i].current     = OFF;
        g_IOTUtil[i].upEdge      = OFF;
        g_IOTUtil[i].downEdge    = OFF;
        g_IOTUtil[i].elapsedTime = 0;
        g_IOTUtil[i].targetTime  = INT_FAST32_MAX;               
    }        
}

void setIOTimer(int index, unsigned int targetTime, char sensorStat)
{
    g_IOTUtil[index].targetTime = targetTime;
    g_IOTUtil[index].sensor     = sensorStat;    
}

void updateIOTimer()
{
    uint8 b1ms = get1msecTimerReset();
    
    for (int i = 0; i < MAX_IOT; i++)
    {  
        if (g_IOTUtil[i].sensor) //sensor detected
        {  
            if (g_IOTUtil[i].elapsedTime < g_IOTUtil[i].targetTime && b1ms) g_IOTUtil[i].elapsedTime++;
        }
        else //sensor not detected
        {
            g_IOTUtil[i].elapsedTime = 0;
            g_IOTUtil[i].current     = OFF;
        }

        
//        g_IOTUtil[i].current   = (g_IOTUtil[i].elapsedTime >= g_IOTUtil[i].targetTime);   
//        g_IOTUtil[i].upEdge    = (( g_IOTUtil[i].current) && (~g_IOTUtil[i].previous));
//        g_IOTUtil[i].downEdge  = ((~g_IOTUtil[i].current) && ( g_IOTUtil[i].previous));
//        g_IOTUtil[i].previous  = g_IOTUtil[i].current;  
        
        g_IOTUtil[i].current   = (g_IOTUtil[i].elapsedTime >= g_IOTUtil[i].targetTime);   
        g_IOTUtil[i].upEdge    = (( g_IOTUtil[i].current) && (!g_IOTUtil[i].previous));
        g_IOTUtil[i].downEdge  = ((!g_IOTUtil[i].current) && ( g_IOTUtil[i].previous));
        g_IOTUtil[i].previous  = g_IOTUtil[i].current;  
/*       
        g_IOTUtil[i].current   = (g_IOTUtil[i].elapsedTime >= g_IOTUtil[i].targetTime)              ? ON : OFF;   
        g_IOTUtil[i].upEdge    = ((g_IOTUtil[i].current > 0)  && (g_IOTUtil[i].previous == OFF)) ? ON : OFF;
        g_IOTUtil[i].downEdge  = ((g_IOTUtil[i].current == OFF) && (g_IOTUtil[i].previous == ON  )) ? ON : OFF;
        g_IOTUtil[i].previous  = g_IOTUtil[i].current;  */
     }
    
    doTimer();
}

char waitTime(unsigned int id, unsigned long timeout)
{
    if(timeout == 0)
        setIOTimer(id, 1, OFF);
    else
        setIOTimer(id, timeout, ON);
        
    if(uT(id))
    {
        setIOTimer(id, 1, OFF);
        return TRUE;
    }
    return FALSE;    
}