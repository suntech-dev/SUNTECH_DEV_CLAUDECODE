/*****************************************************************************
 *
 * This document contains proprietary information and except with
 * written permission of Displaytech, Ltd., such information shall
 * not be published or disclosed to others or used for any purpose
 * other than for the operation and maintenance of the equipment
 * and software with which it was procured, and the document shall
 * not be copied in whole or in part.
 *
 * Copyright 2008-2014 Displaytech, Ltd. All Rights Reserved.
 *
 *****************************************************************************/

#include <string.h>
#include "main.h"
#include "FT5x46.h"

uint8_t numTouches = 0;
uint8_t gesture = 0;
uint8_t touchActive = 0;
struct sParamTouch ParamTouch;

uint8 TouchReadRegister(uint8_t reg, uint8_t *dest, uint8_t len)
{
    uint8_t error;
    uint8_t i;
    
    error = I2C_TC_I2CMasterSendStart(TC_I2C_SLAVE_ADDR,0, TC_I2C_READ_TIME);
    if(error == I2C_TC_I2C_MSTR_NO_ERROR) error = I2C_TC_I2CMasterWriteByte(reg, TC_I2C_READ_TIME);
    if(error == I2C_TC_I2C_MSTR_NO_ERROR) error = I2C_TC_I2CMasterSendRestart(TC_I2C_SLAVE_ADDR,1, TC_I2C_READ_TIME);
    for(i=0; i<len-1;i++)
    {
        if(error ==  I2C_TC_I2C_MSTR_NO_ERROR) I2C_TC_I2CMasterReadByte(I2C_TC_I2C_ACK_DATA,dest + i, TC_I2C_READ_TIME);
    }
    if(error ==  I2C_TC_I2C_MSTR_NO_ERROR) I2C_TC_I2CMasterReadByte(I2C_TC_I2C_NAK_DATA, dest+(i-1), TC_I2C_READ_TIME);
    
    I2C_TC_I2CMasterSendStop(TC_I2C_READ_TIME);
    I2C_TC_I2CMasterClearStatus();
    CyDelayUs(100);
    return error;
}

void byteReverse(uint8_t* dest, uint8_t* src, uint8_t len)
{
    src += (len - 1);

    while(len--)
        *dest++ = *src--;
}

void TouchHardwareInit(void)
{
    //Reset the device
    TC_RESET_Write(0);
    CyDelay(10);
    TC_RESET_Write(1);
    CyDelay(500);
}


void TouchPos(uint8_t* data, uint8_t touch_num)
{
    uint16_t pos;
   // char stBuf[100];
  //DEBUG_printf("5");    
    if((data[0] & 0x80) && ((data[2] & 0xF0) == (touch_num << 4)))
    {     
        pos  = (data[0] & 0x0F) << 8;
        pos += data[1];    
        ParamTouch.x[touch_num] = pos;
        
        pos  = (data[2] & 0x0F) << 8;
        pos += data[3];    
        ParamTouch.y[touch_num] = pos;
        
        touchActive = 1;
      //  sprintf(stBuf,"Touch %i X: %i, Y: %i \r\n",touch_num, ParamTouch.x[touch_num], ParamTouch.y[touch_num]);
      //  UART_UartPutString(stBuf);
    }
    else
    {
      //  DEBUG_printf("Touch Error \r\n");
    }
}

void TouchGetPosition(void)
{
    uint8 error;
    uint8_t i;
    uint8_t rBuf[10];
    
    error = TouchReadRegister(FT5X46_REG_DEVICE_MODE, rBuf, 9);
    if(error == I2C_TC_I2C_MSTR_NO_ERROR)
    {
        if(rBuf[FT5X46_REG_DEVICE_MODE] == 0x00)
        {
            numTouches = rBuf[FT5X46_REG_TD_STATUS];
            
            if((numTouches > 0) && (numTouches <= MAX_TOUCHES))
            {
                for( i = 0; i < numTouches; i++)
                {
                    TouchReadRegister(FT5X46_REG_XH_POS + FT5X46_TOUCH_LENGTH*i, rBuf, 6);
                    
                    TouchPos(rBuf, i);      
                }
            }            
        }
    }
    return;
}

uint16_t mchpTouchScreenVersion = 0x0000;
void TouchStoreCalibration(void) { return; }
void TouchLoadCalibration(void)  { return; }
void TouchCalHWGetPoints(void) { return; }
