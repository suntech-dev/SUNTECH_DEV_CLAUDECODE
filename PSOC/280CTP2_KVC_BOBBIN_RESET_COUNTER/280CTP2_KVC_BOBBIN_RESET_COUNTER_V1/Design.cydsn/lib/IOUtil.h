//
// Input-Output Timer Utility
//
//                   theGuruship@gmail.com
//                   Rev. 0.1    2017-11-3
//

#ifndef _IOT_UTIL_H_
#define _IOT_UTIL_H_
#include "IODefine.h"

void initIOUtil(); 
void getInput();

//////////////////////////////////////////////////////////////////////////////////////////
// Input-Output Utility
//////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    char previous  : 1; // Previouse State
    char current   : 1; // Current State
    char upEdge    : 1; // Up(risizing)
    char downEdge  : 1; // Down(falling)
    char activate  : 1;
    char dummy     : 3;
} SIOUtil;

extern SIOUtil g_IOUtil[MAX_IO];
#define  R(i)  (i ^ 1);

#define  I(i)   g_IOUtil[i].current
#define _I(i)  (I(a) ^ 1)
#define uI(i)   g_IOUtil[i].upEdge
#define dI(i)   g_IOUtil[i].downEdge
#define cI(i)   (uI(i) || dI(i))

#define  O(i)   g_IOUtil[i].current
#define _O(i)  (O(i) ^ 1)
#define uO(i)   g_IOUtil[i].upEdge
#define dO(i)   g_IOUtil[i].downEdge

void initIO();
void updateIO();
void glitchFiltering();
//////////////////////////////////////////////////////////////////////////////////////////
// Input-Output Timer Utility
//////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    char sensor    : 1; // Sensor condition
    char previous  : 1; // Previouse Timer On Flag
    char current   : 1; // Current Timer On Flag
    char upEdge    : 1; // Up(risizing) Edge Flag
    char downEdge  : 1; // Down(falling) Edge Flag
    char dummy     : 3;

    unsigned int elapsedTime;
    unsigned int targetTime;
} SIOTUtil;
extern SIOTUtil g_IOTUtil[MAX_IOT];
 
#define  T(i) (g_IOTUtil[i].current)
#define _T(i) (T(i) ^ 1)
#define uT(i) (g_IOTUtil[i].upEdge)
#define dT(i) (g_IOTUtil[i].downEdge)

void initIOT();
void setIOTimer(int index, unsigned int targetTime, char sensorStat);
void updateIOStat();
void updateIOTimer();
char waitTime(unsigned int id, unsigned long timeout);
#endif
