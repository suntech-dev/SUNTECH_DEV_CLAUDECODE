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
#include "UI.h"
#include "FT5x46.h"
#include "widget.h"
#include "sysTick.h"
#include "userTimer.h"
#include "externalFlash.h"

//uint8 g_displayDirection = DISPLAY_DIRECTION_PORTRAIT;
uint16 g_DEFAULT_BUTTON_HEIGHT = DEFAULT_BUTTON_HEIGHT_PORTRAIT;

uint16 g_SCREEN_WIDTH  = LCD_WIDTH;
uint16 g_SCREEN_HEIGHT = LCD_HEIGHT;

uint8 g_index_touch_counter;

void setDisplayDirection(uint8 dir)
{
    getMiscConfig()->bDisplayDir = dir;
    
    if(dir == DISPLAY_DIRECTION_PORTRAIT)
    {
        g_SCREEN_WIDTH  = LCD_WIDTH;
        g_SCREEN_HEIGHT = LCD_HEIGHT;
        g_DEFAULT_BUTTON_HEIGHT = DEFAULT_BUTTON_HEIGHT_PORTRAIT;
    }
    else
    {
        g_SCREEN_WIDTH  = LCD_HEIGHT;
        g_SCREEN_HEIGHT = LCD_WIDTH;
        g_DEFAULT_BUTTON_HEIGHT = DEFAULT_BUTTON_HEIGHT_LANDSCAPE;        
    }
    
    initWidget();
}

POINT MapToDeviceCoordinate(uint16 x, uint16 y)
{
    static POINT point;

    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
    {
        point.x = x;
        point.y = y;
    }
    else 
    {
        point.x = LCD_WIDTH - y - 1;
        point.y = x;        
    }

    return point;
}

POINT MapToScreenCoordinate(uint16 x, uint16 y)
{
    static POINT point;

    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
    {
        point.x = x;
        point.y = y;
    }
    else 
    {
        point.x = y;
        point.y = LCD_WIDTH - x - 1; 
    }
    
    return point;
}

RECT ST7789V_setaddress_MapScreenCoordinate(uint16 x1,uint16 y1,uint16 x2,uint16 y2)
{    
    POINT p1 = MapToDeviceCoordinate(x1, y1);
    POINT p2 = MapToDeviceCoordinate(x2, y2);
    
    static RECT rect;
    rect.left   = MIN(p1.x,p2.x);
    rect.right  = MAX(p1.x,p2.x); 
    rect.top    = MIN(p1.y,p2.y);
    rect.bottom = MAX(p1.y,p2.y); 
  
    ST7789V_setaddress(rect.left, rect.top, rect.right, rect.bottom); 
    return rect;
}

void FillAllArea(COLOR color)
{
    FillRectangle(0, 0, g_SCREEN_WIDTH-1, g_SCREEN_HEIGHT-1, color);
}

void FillRectangle(uint16 x1, uint16 y1, uint16 x2, uint16 y2, COLOR color)
{        
    RECT rect = ST7789V_setaddress_MapScreenCoordinate(x1, y1, x2, y2);
    
    for(int i=x1; i <= x2; i++)
       for(int j=y1; j<= y2; j++)
          ST7789V_pushcolour(color); 
}

void FillRect(RECT *r, COLOR color)
{        
    FillRectangle(r->left, r->top, r->right, r->bottom, color);
}


void DrawHorizontalLine(uint16 x1, uint16 x2, uint16 y, COLOR color)
{
    uint16 minX = MIN(x1,x2);
    uint16 maxX = MAX(x1,x2);
    
    ST7789V_setaddress_MapScreenCoordinate(x1, y, x2, y);
    
    for(uint16 i=minX; i <= maxX; i++)
        ST7789V_pushcolour(color);    
}

void DrawRectangle     (uint16 x1, uint16 y1, uint16 x2, uint16 y2, COLOR color)
{
    DrawVerticalLine(x1,y1,y2,color);    
    DrawHorizontalLine(x1+1, x2-1, y1, color);
    DrawVerticalLine(x2,y1,y2,color);        
    DrawHorizontalLine(x1+1, x2-1, y2, color);

}
void DrawRect          (RECT rect, COLOR color)
{
    DrawRectangle(rect.left, rect.top, rect.right, rect.bottom, color);
}

void DrawVerticalLine(uint16 x, uint16 y1, uint16 y2, COLOR color)
{    
    uint16 minY = MIN(y1,y2);
    uint16 maxY = MAX(y1,y2);
    
    ST7789V_setaddress_MapScreenCoordinate(x, minY, x, maxY);
    
    for(uint16 i=minY; i <= maxY; i++)
        ST7789V_pushcolour(color);
}

void DrawImage(uint16 xPos, uint16 yPos, uint16 width, uint16 height, uint16 *image)
{
    uint32_t index = 0, i = 0, j=0;
 
    ST7789V_setaddress_MapScreenCoordinate(xPos, yPos, xPos+width-1, yPos);
    
    for( index = 0; index < height  ; index++)
    {
        for(i = 0; i < width ; i++)
        {
            ST7789V_pushcolour(image[j++]);
        }
        ST7789V_setaddress_MapScreenCoordinate(xPos, yPos+index+1, xPos+width-1, yPos+index+1);      
    }    
}

void DrawImageBackColorReplace(uint16 xPos, uint16 yPos, uint16 width, uint16 height, uint16 *image, uint16 blackReplaceColor)
{
    uint32_t index = 0, i = 0, j=0;
 
    ST7789V_setaddress_MapScreenCoordinate(xPos, yPos, xPos+width-1, yPos);
    
    for( index = 0; index < height  ; index++)
    {
        for(i = 0; i < width ; i++)
        {
            uint16 c = image[j++];
            
            if(c == 0) c = blackReplaceColor;
            ST7789V_pushcolour(c);
        }
        ST7789V_setaddress_MapScreenCoordinate(xPos, yPos+index+1, xPos+width-1, yPos+index+1);      
    }    
}

void LCD_DrawFont(uint16 xPos, uint16 yPos, uint16 foregroundColor, uint16 backgroundColor, fontdatatype *font, const char st)
{
    uint16 width = font[0];
    uint16 height = font[1];
    uint16 ascii_start = font[2];
    uint16 noFont = font[3];

    float n = width / 8.0;
    uint16 r = (uint16) n;
    if(n != (float) r) r++;

    uint16 DataSize = r * height;  /* bytes/char = ceil(width/8) * height — non-8-aligned 폰트 지원 */
    uint32 p = DataSize * (st-ascii_start) + 4;
    
//  printf("width  = %d \r\n",width);
//  printf("height = %d \r\n",height);
//  printf("start  = %c \r\n",ascii_start);
//  printf("No     = %d \r\n",noFont);

    for(uint16 index = 0; index < height  ; index++)
    {
        uint16 y = yPos+index;
           
        ST7789V_setaddress_MapScreenCoordinate(xPos, y, xPos+width-1, y);   

        for(int j=0; j < r; j++)
        {
             uint8 data = font[p];

            for(int i = 0; i < 8; i++)
            {
                if(j * 8 + i >= width) break;  /* 패딩 비트 제외 (non-8-aligned 폰트 지원) */
                if((data  & (0x80 >> i)) == 0)
                {
                    ST7789V_pushcolour(backgroundColor);
                }else{
                    ST7789V_pushcolour(foregroundColor);
                }
            }
            p++;
        }        
    }
}

void LCD_printf(uint16 xPos, uint16 yPos, uint16 foregroundColor, uint16 backgroundColor, fontdatatype *font, const char *fmt, ...)
{
    va_list ap;
    char buff[128];
    
    va_start(ap, fmt);

    vsprintf(buff, fmt, ap);
    va_end(ap);
        
    uint16 xInterval = font[FONT_WIDTH];
    uint16 nLength = strlen(buff);

    for(uint16 i=0; i < nLength; i++)
    {
        LCD_DrawFont(xPos + xInterval * i, yPos, foregroundColor, backgroundColor, font, buff[i]);
    }     
}

POINT CalculationOfLocationForString(RECT boundingBox, uint16 font_width, uint16 font_height, uint16 marginLeftRight, uint16 marginTopBottom, uint16 strlength, uint8 horizontalAlign, uint8 verticallAlign)
{
    POINT startPos;
    switch(horizontalAlign)
    {
        case TEXT_ALIGN_LEFT:   startPos.x = boundingBox.left + marginLeftRight;                         break;
        case TEXT_ALIGN_RIGHT:  startPos.x = boundingBox.right - font_width*strlength - marginLeftRight; break;
        case TEXT_ALIGN_CENTER: startPos.x = ((boundingBox.right + boundingBox.left) - font_width*strlength) * 0.5; break;     //((boundingBox.right + boundingBox.left)/2 - font[FONT_WIDTH]*strlength/2
    }
    switch(verticallAlign)
    {
        case TEXT_ALIGN_TOP:     startPos.y = boundingBox.top + marginTopBottom;                  break;
        case TEXT_ALIGN_BOTTOM:  startPos.y = boundingBox.bottom - font_height - marginTopBottom; break;
        case TEXT_ALIGN_MIDDLE:  startPos.y = ((boundingBox.bottom + boundingBox.top) - font_height) * 0.5; break;        
    }
    return startPos;
}


TOUCH GetTouch()
{
    TOUCH touch = {
        .isClick = FALSE,
        .point.x       = 0,
        .point.y       = 0,
    };
    
    static uint8 isUpStat = TRUE;
    static uint8 oldRead = 0, read;

    read = ~TC_INT_Read() & 0x01;
    
    if(oldRead == 0 && read == 1)
    {
        resetCounter_1ms(g_index_touch_counter);
        //tc_no_touch_count = 100;
        
        if(isUpStat)
        {
            TouchGetPosition();
            if(touchActive)
            {
                touch.point = MapToScreenCoordinate(ParamTouch.x[0], ParamTouch.y[0]);
                
                touch.isClick = TRUE;
                isUpStat = FALSE;  
                
                touchActive = 0;                
            }
        }
        
    }
    else
    {
        if(isFinishCounter_1ms(g_index_touch_counter))
        {
            //tc_no_touch_count = 0;
            isUpStat = TRUE;
        }   
    }
        
    oldRead = read;
    return touch;
}


uint8 isPointInRect(RECT *r, POINT pt)
{
    if(pt.x < r->left)   return FALSE;
    if(pt.x > r->right)  return FALSE;
    if(pt.y < r->top)    return FALSE;
    if(pt.y > r->bottom) return FALSE;
    return TRUE;
}



BUZZER_CONTROL g_BuzzerControl = {
    .uDownCount = 0,
    .uUpCount   = 0,
    .uInterate  = 0,
    .type       = BUZZER_STOP,
};

void Buzzer(uint8 type, uint16 noOfInterate)
{
    switch(g_BuzzerControl.type = type)
    {
        case BUZZER_STOP:        
            g_BuzzerControl.uDownCount = 0;
            g_BuzzerControl.uUpCount = 0;
            g_BuzzerControl.uInterate = 0;
            BUZZER_Write(FALSE);
            break;
        case BUZZER_CLICK:
            g_BuzzerControl.uDownCount = 50;
            break;
        case BUZZER_WARNING:
            g_BuzzerControl.uUpCount = 0;
            g_BuzzerControl.uInterate = noOfInterate;
            break;
        case BUZZER_WARNING_CONTINUOUS:
            g_BuzzerControl.uUpCount = 0;
            g_BuzzerControl.uInterate = UINT16_MAX;
            g_BuzzerControl.type = BUZZER_WARNING;
            break;            
        case BUZZER_SHORT_WARNING:
            g_BuzzerControl.uUpCount = 0;
            g_BuzzerControl.uInterate = noOfInterate;              
            break;
        case BUZZER_SHORT_WARNING_CONTINUOUS:
            g_BuzzerControl.uUpCount = 0;
            g_BuzzerControl.uInterate = UINT16_MAX;  
            g_BuzzerControl.type = BUZZER_SHORT_WARNING;            
            break;   
        case BUZZER_CONTINUOUS_BEEP:
            BUZZER_Write(TRUE);            
            break;
    }
}

void BuzzerTimer()
{
    switch(g_BuzzerControl.type)
    {
        case BUZZER_STOP:
        case BUZZER_CONTINUOUS_BEEP:
            return;
        case BUZZER_WARNING:
            if(g_BuzzerControl.uInterate==0)
            {
                g_BuzzerControl.type = BUZZER_STOP;
            } else {
                switch(g_BuzzerControl.uUpCount++)
                {
                    case    0: g_BuzzerControl.uDownCount = 500; break;                 
                    case 1000: g_BuzzerControl.uUpCount = 0;
                               g_BuzzerControl.uInterate--;  
                }                
            }
            break;    
        case BUZZER_SHORT_WARNING:
            if(g_BuzzerControl.uInterate==0)
            {
                g_BuzzerControl.type = BUZZER_STOP;
            } else {
                switch(g_BuzzerControl.uUpCount++)
                {
                    case    0: g_BuzzerControl.uDownCount = 50; break;
                    case  100: g_BuzzerControl.uDownCount = 50; break;
                    case  200: g_BuzzerControl.uDownCount = 50; break;                         
                    case 1000: g_BuzzerControl.uUpCount = 0;
                               g_BuzzerControl.uInterate--;                    
                    break;
                }                
            }
            break;              
    }
    
    if(g_BuzzerControl.uDownCount)
    {
        BUZZER_Write(TRUE);
        g_BuzzerControl.uDownCount--;
    } else {
        BUZZER_Write(FALSE);
    }        
}

//void SetIconButton(ICON_BUTTON *btn, uint16 x, uint16 y, uint16 width, uint16 height, uint16 *image)
//{
//    btn->image = image;
//    btn->rect.left = x;
//    btn->rect.top = y;
//    btn->rect.right = x + width - 1;
//    btn->rect.bottom = y + height - 1;
//}

RECT GetBodyArea()
{
    RECT body;
    body.top = (DEFAULT_TOP_TITLE_HEIGHT + 1);
    body.left = 0;
    body.right = g_SCREEN_WIDTH;
    body.bottom = g_SCREEN_HEIGHT-g_DEFAULT_BUTTON_HEIGHT-1;
    return body;
}

void RectangleSizeShrink(RECT *r, int16 left, int16 right, int16 up, int16 down) // width, height은 증분을 의미 한다.
{
    r->left   +=  left;
    r->right  += right;
    r->top    +=    up;
    r->bottom +=  down;    
}
void initUI()
{
    g_index_touch_counter = registerCounter_1ms(50); // 50ms동안 연속적으로  touch가 되어 있어야 만 touch로 인정한다.  double touch 방지용
}
/* [] END OF FILE */
