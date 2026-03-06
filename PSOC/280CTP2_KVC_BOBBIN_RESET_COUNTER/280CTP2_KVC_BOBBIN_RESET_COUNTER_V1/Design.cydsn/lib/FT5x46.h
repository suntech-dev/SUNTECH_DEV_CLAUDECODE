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
#ifndef __FT5X46_H__
#define __FT5X46_H__
    
#include "main.h"

#define TC_I2C_SLAVE_ADDR 0x38
#define TC_I2C_READ_TIME  (50u)
#define MAX_TOUCHES 5

#define GEST_NONE       0x00
#define GEST_UP         0x10
#define GEST_LEFT       0x14
#define GEST_DOWN       0x18
#define GEST_RIGHT      0x1C
#define GEST_IN         0x48
#define GEST_OUT        0x49

/* ft5x46 finger register list */
#define FT5X46_TOUCH_LENGTH		6
#define FT5X46_REG_DEVICE_MODE		0
#define FT5X46_REG_GEST_ID			1
#define FT5X46_REG_TD_STATUS		2
#define FT5X46_REG_XH_POS			3
#define FT5X46_REG_XL_POS			4
#define FT5X46_REG_YH_POS			5
#define FT5X46_REG_YL_POS			6
#define FT5X46_REG_XY_POS			7

struct sParamTouch
{
    uint8 no_Touches;
    uint16  x[MAX_TOUCHES];
    uint16  y[MAX_TOUCHES];
};

void TouchHardwareInit(void);
void TouchGetPosition(void);
uint16_t TouchGetX(void);
uint16_t TouchGetY(void);
uint8 TouchReadRegister(uint8_t reg, uint8_t *dest, uint8_t len);

extern uint8_t touchActive;
extern struct sParamTouch ParamTouch;

#endif