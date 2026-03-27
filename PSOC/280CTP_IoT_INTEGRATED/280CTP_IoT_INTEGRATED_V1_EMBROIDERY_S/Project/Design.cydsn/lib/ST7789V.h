/******************************************************************************
* Project Name      : SmartBeeHive
* File Name         : Si7013.h
* Version           : Version 1.0
* Device Used       : CYBLE-022001-00
* Software Used     : PSoC Creator 3.3 SP1
* Compiler          : ARM GCC 4.9.3
* Related Hardware  : 
*
*******************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation. All Rights Reserved.
*******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH 
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the 
* materials described herein. Cypress does not assume any liability arising out 
* of the application or use of any product or circuit described herein. Cypress 
* does not authorize its products for use as critical components in life-support 
* systems where a malfunction or failure may reasonably be expected to result in 
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of 
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
******************************************************************************/

#include <project.h>
#include "fonts.h"

#ifndef __ST7789V_H_
#define __ST7789V_H_

/******************************************************************************
* Macros and Constants
******************************************************************************/

#define ST7789V_COMMAND      (0)
#define ST7789V_DATA         (1)
/*-----------------------------------------------------------------------*/ 
   #define ST7789V_320_240 
//   #define ST7789V_128_64 
//   #define ST7789V_128_32 
//   #define ST7789V_96_16 
/*=========================================================================*/ 
#if defined ST7789V_320_240 
  #define ST7789V_LCDWIDTH                  320 
  #define ST7789V_LCDHEIGHT                 240 
#endif 
#if defined ST7789V_128_64 
  #define ST7789V_LCDWIDTH                  128 
  #define ST7789V_LCDHEIGHT                 64 
#endif 
#if defined ST7789V_128_32 
  #define ST7789V_LCDWIDTH                  128 
  #define ST7789V_LCDHEIGHT                 32 
#endif 
#if defined ST7789V_96_16 
  #define ST7789V_LCDWIDTH                  96 
  #define ST7789V_LCDHEIGHT                 16 
#endif 


#define BLACK       0x0000       
#define NAVY        0x000F       
#define DARKGREEN   0x03E0       
#define DARKCYAN    0x03EF       
#define MAROON      0x7800       
#define PURPLE      0x780F       
#define OLIVE       0x7BE0       
#define LIGHTGREY   0xC618       
#define DARKGREY    0x7BEF       
#define BLUE        0x001F       
#define GREEN       0x07E0       
#define CYAN        0x07FF       
#define RED         0xF800      
#define MAGENTA     0xF81F       
#define YELLOW      0xFFE0       
#define WHITE       0xFFFF       
#define ORANGE      0xFD20       
#define GREENYELLOW 0xAFE5      
#define PINK        0xF81F 



/******************************************************************************
* External Function Prototypes
******************************************************************************/
void ST7789V_Init(void);

//void LCD_DrawString(uint16_t Xpos, uint16_t Ypos, uint8 cnt, char *st);
//void LCD_DrawChar_16(uint16_t Xpos, uint16_t Ypos, const char st);
void ST7789V_pushcolour(uint16_t colour) ;
void ST7789V_fillrect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t colour);
void ST7789V_setaddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);

extern uint16 colorBack;
extern uint16 colorText;

#endif /* __SH1106_H_ */

/* [] END OF FILE */
