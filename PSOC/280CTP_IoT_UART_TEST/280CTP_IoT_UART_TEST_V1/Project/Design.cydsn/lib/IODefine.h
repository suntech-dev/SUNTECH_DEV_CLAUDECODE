// Input-Output Timer Utility
//
//                   theGuruship@gmail.com
//                   Rev. 0.1    2017-11-3
//

#ifndef _IO_DEFINE_H_
#define _IO_DEFINE_H_

enum DefIO {
    // FOR OUTPUT /////////////////
//    oLED = 0,                  //  0
    
    
    END_OF_OUTPUT,
      iLCD_TOUCH,
    // FOR INPUT
//    iMODE_SW,                  //  0
//    iUP_SW,                    //  1    
//    iDOWN_SW,                  //  2
//    iSET_SW,                   //  3 

//    iBOBBIN_WINDER,            //  4
//    iTHREAD_SENSING,           //  5   
//    iRPM_COUNT,                //  6       
    END_OF_DEFIO
};   
//---------------------------------------------
#define MAX_IO          20  // for Input-Output
const static unsigned int START_INPUT = END_OF_OUTPUT+1;

void doInput();
void doOutput();
unsigned long getSensor();
void defineIO();
void doTimer();

enum DefTimer {
    t100MS = 0,            //  0
    t500MS,                //  0    
//    tUP_SW,
//    tDOWN_SW,
//    tSplash,
//    tWIFI,
//    tUserCmd,
};

//---------------------------------------------
#define MAX_IOT        8  // for Input-Output Timer

#endif
