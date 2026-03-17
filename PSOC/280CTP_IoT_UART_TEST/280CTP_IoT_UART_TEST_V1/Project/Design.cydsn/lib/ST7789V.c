/******************************************************************************
* Project Name      : SmartBeeHive
* File Name         : ST7789V.c
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

#include "main.h"
#include "ST7789V.h"

#define EX_DATASHEET    0
#define EX_SOURCE       0

#if(EX_DATASHEET)
//Datasheet
static uint8_t init_ENH_OB130SB128[30] = {  0x20, 0x10, 0xb0, 0xc8, 0x02, 0x10, 0x40, 0x81, 0xaf,
                                            0xa1, 0xa6, 0xa8, 0x3f, 0xa4, 0xd3, 0x00, 0xd5, 0xf0, 0xd9,
                                            0x22, 0xda, 0x12, 0xdb, 0x20, 0x8d, 0x14,
                                            };
#else
static uint8_t init_ENH_OB130SB128[30] = {  0x40, 0x81, 0x80, 0xA0, 0xA4, 0xA6, 0xA8, 0x3F, 0xC0,
                                            0xAD, 0x8B, 0x33, 0xD3, 0x00, 0xD5, 0x80, 0xD9, 0x1F, 0xDa,
                                            0x12, 0xdB, 0x40,
                                            };
#endif

uint16 colorBack;
uint16 colorText;

void Delay(uint32 time)
{
    while(time--)
    {
        CY_NOP;   
    }
}

void writeCommand(uint8 cmd)
{
    Ctrl_LCD_RS_Write(ST7789V_COMMAND);
    SPIM_LCD_WriteByte(cmd);
    CyDelayUs(1);
}


void writeData(uint8 data)
{
    Ctrl_LCD_RS_Write(ST7789V_DATA);
    SPIM_LCD_WriteByte(data);
    //CyDelayUs(1);
    Delay(5);
}

//
void ST7789V_setaddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)//set coordinate for print or other function 
{ 
	writeCommand(0x2A); 
	writeData(x1>>8); 
	writeData(x1); 
	writeData(x2>>8); 
	writeData(x2); 

	writeCommand(0x2B); 
	writeData(y1>>8); 
	writeData(y1); 
	writeData(y2>>2); 
	writeData(y2); 

	writeCommand(0x2C);//memory write 
} 


void ST7789V_hard_reset(void)//hard reset display 
{   
    
    LCD_RES_Write(1);
    CyDelay(20);
    LCD_RES_Write(0);
    CyDelay(20);
    LCD_RES_Write(1);
    CyDelay(20);
} 

/******************************************************************************
* Function Name: ST7789V_Init, 4 wire SPI
*******************************************************************************
*
* Summary:
*   Init Temperature and Humidity sensor Si7013
*
* Parameters:
*   None
*
* Return:
*   Error status
******************************************************************************/

void ST7789V_Init(void)//set up display using predefined command sequence 
{ 
	ST7789V_hard_reset(); 
    
	CyDelay(120); 
	writeCommand(0x11); // Sleep Out
	CyDelay(120); 
    
    //display and color forma setting
	writeCommand(0x36); 
	writeCommand(0x00); 
	writeCommand(0x3A); 
	writeData(0x55);  
    
    //Frame rate seting
    
	writeCommand(0xB2); 
	writeData(0x0C); 
	writeData(0x0C); 
	writeData(0x00); 
	writeData(0x33); 
	writeData(0x33); 
	writeCommand(0xB7); 
	writeData(0x35); 

	//power control 
	writeCommand(0xBB); 
	writeData(0x39);  
	writeCommand(0xC0); 
	writeData(0x2c); 
	writeCommand(0xC2); 
	writeData(0x01); 
	writeCommand(0xC3); 
	writeData(0x10); 
	writeCommand(0xC4); 
	writeData(0x20); 
	writeCommand(0xC6); 
	writeData(0x0F); 
	writeCommand(0xD0); 
	writeData(0xA4); 
	writeData(0xA1);
    
    writeCommand(0xD6); 
	writeData(0xA1); 

	//set gamma correction 
	writeCommand(0xE0); 
	writeData(0x0D); 
	writeData(0x0F); 
	writeData(0x11); 
	writeData(0x07); 
	writeData(0x05); 
	writeData(0x02); 
	writeData(0x28); 
	writeData(0x33); 
	writeData(0x3F); 
	writeData(0x26); 
	writeData(0x14); 
	writeData(0x15); 
	writeData(0x24); 
	writeData(0x28); 

	//set gamma correction 
	writeCommand(0xE1); 
	writeData(0x0D); 
	writeData(0x0E); 
	writeData(0x11); 
	writeData(0x07); 
	writeData(0x05); 
	writeData(0x02); 
	writeData(0x28); 
	writeData(0x22); 
	writeData(0x3F); 
	writeData(0x2A); 
	writeData(0x18); 
	writeData(0x19); 
	writeData(0x26); 
	writeData(0x28);
    
	//exit sleep 
	//writeCommand(0x11); 
    ST7789V_fillrect(0, 0,ST7789V_LCDWIDTH, ST7789V_LCDHEIGHT, colorBack);
	CyDelay(10);
	//display on 
	writeCommand(0x29); 
	CyDelay(10); 
        
} 

//set colour for drawing 
void ST7789V_pushcolour(uint16_t colour) 
{ 
	writeData(colour>>8); 
	writeData(colour); 
} 

//draw colour filled rectangle 
void ST7789V_fillrect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t colour) 
{ 
	if ((x >= ST7789V_LCDWIDTH) || (y >= ST7789V_LCDHEIGHT)) 
	{ 
		return; 
	} 

	if ((x+w-1) >= ST7789V_LCDWIDTH) 
	{ 
		w = ST7789V_LCDWIDTH-x; 
	} 

	if ((y+h-1) >= ST7789V_LCDHEIGHT) 
	{ 
		h = ST7789V_LCDHEIGHT-y; 
	} 

	ST7789V_setaddress(x, y, x+w-1, y+h-1); 

	for(y=h; y>0; y--)  
	{ 
		for(x=w; x>0; x--) 
		{ 
			ST7789V_pushcolour(colour); 
		} 
	} 
} 

////
//void LCD_DrawChar_16(uint16_t Xpos, uint16_t Ypos, const char st)
//{
//    uint32_t index = 0, i = 0;
//    uint16_t xAddress = 0;
//    uint16_t data = 0;
//    
//    xAddress = Xpos;
//
//    //LCD_SetCursor(xAddress, START_Y - Ypos);
//    ST7789V_setaddress(xAddress, Ypos, xAddress + 31, Ypos);
//
//    for( index = 1; index <= 31  ; index++)
//    {
//        data = FontsLib_16x32[ st - 0x20][index*2] << 8;
//        data += FontsLib_16x32[ st - 0x20][index*2+1];
//        for(i = 0; i < 31; i++)
//        {
//            if((data  & (0x8000 >> i)) == 0) 
//            {
//                ST7789V_pushcolour(colorBack); 
//            }else{
//                ST7789V_pushcolour(colorText);   
//            }
//        }
//        ST7789V_setaddress(xAddress, Ypos+index, xAddress + 31, Ypos + index);
//    }
//}

//
void LCD_DrawString(uint16_t Xpos, uint16_t Ypos, uint8 cnt, char *st)
{
    uint8 i;
        
    for( i = 0; i < cnt; i++)
    {
        LCD_DrawChar_16(Xpos + 16 * i , Ypos, *st++);        
    }
    
}

/* [] END OF FILE */
