/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "userMenu.h"
#include "lib/widget.h"
#include "lib/image.h"
#include "menuDesign.h"

#include "package.h"

#ifdef PROJECT_NAME1

#define PROJECT_FIRMWARE_VERSION "1.1.1(2023.03.30)"
    
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doTopMenu   ////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayDoTopMenu();

int doTopMenu(uint8 reflash)
{    
    static uint8 bIsFirst=TRUE;
    if(bIsFirst) // 이함수에서 단한번만 호출 된다.
    {
        bIsFirst = FALSE;
    }
    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                DisplayDoTopMenu();
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                
                switch(getIndexOfClickedButtonArray(&tc, &g_btnList))
                {
                    case 0: break;
                    
                    default:
                        switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                        {
                            case BOTTOM_LEFT:  return 0;
                            case BOTTOM_RIGHT: return 1;                            
                        }
                    break;
                }
                
//                uint8 index = getIndexOfClickedButtons(&tc, &g_btnArray);
//                
//                if(index == g_btnArray.size - 2)
//                {
//                    return 0;
//                }
//                switch(index)
//                {                 
//                }
       
                printf("%d\r\n", index);   
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

void DisplayDoTopMenu()
{
    char *listArray[] = {"List1", "List2", "List3", "List4", "List5"};

    SetDrawListButtons(NULL, listArray, 5);
    SetDrawBottomButtons("INFO", "LIST");  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doInfoMenu   ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayInfoMenu();

int doInfoMenu(uint8 reflash)
{    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                DisplayInfoMenu();
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                
                switch(getIndexOfClickedButtonArray(&tc, &g_btnList))
                {
                    case 0: return 0; break;
                    case 1: return 1; break;
                    case 2: return 2; break;                    
                    default:
                        switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                        {
                            case BOTTOM_RIGHT: return MENU_RETURN_PARENT;                            
                        }
                    break;
                }         
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

void DisplayInfoMenu()
{
    char *listArray[] = {"DEVICE INFO", "WIFI INFO", "LED BRIGHT."};
    SetDrawListButtons("INFO", listArray, 3);
    SetDrawBottomButtons(NULL, "EXIT");  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doDeviceInfoMenu   ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayDeviceInfoMenu();

int doDeviceInfoMenu(uint8 reflash)
{        
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                DisplayDeviceInfoMenu();
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                
                switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                {
                    case BOTTOM_LEFT:  return MENU_RETURN_PARENT; 
                    case BOTTOM_RIGHT: return MENU_RETURN_PARENT;                            
                }             
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

void DisplayDeviceInfoMenu()
{
    SetDrawListButtons("DEVICE INFO", NULL, 0);
    SetDrawBottomButtons("REGIST", "EXIT");      

    uint16 xPos = 10;
    uint16 yPos = 100;
    LCD_printf(xPos,yPos,YELLOW,BLACK,Arial_round_16x24,"Line Name");
    
    yPos += 60;
    LCD_printf(xPos,yPos, YELLOW,BLACK,Arial_round_16x24,"Mac Address");

    //LCD_printf(xPos,yPos+30,WHITE,BLACK,SmallFont8x12,g_network.MAC); // WiFi 기능 제거

    yPos += 60;
    LCD_printf(xPos,yPos,YELLOW,BLACK,Arial_round_16x24,"Version");
    LCD_printf(xPos,yPos+30,WHITE,BLACK,SmallFont8x12,PROJECT_FIRMWARE_VERSION);    
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doWifiInfoMenu   ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayWifiInfoMenu();

int doWifiInfoMenu(uint8 reflash)
{        
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                DisplayWifiInfoMenu();
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                                
                switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                {
                    case BOTTOM_RIGHT: return MENU_RETURN_PARENT;                            
                }           
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

void DisplayWifiInfoMenu()
{
    SetDrawListButtons("WIFI INFO", NULL, 0);
    SetDrawBottomButtons(NULL, "EXIT");      

    uint16 xPos = 10;
    uint16 yPos = 100;
    LCD_printf(xPos,yPos,YELLOW,BLACK,Arial_round_16x24,"SSID");
    
    yPos += 45;
    LCD_printf(xPos,yPos,YELLOW,BLACK,Arial_round_16x24,"QUALITY");
    
    yPos += 45;
    LCD_printf(xPos,yPos,YELLOW,BLACK,Arial_round_16x24,"IP");

    //LCD_printf(xPos,yPos+30,WHITE,BLACK,SmallFont8x12,g_network.IPv4); // WiFi 기능 제거
    //LCD_printf(xPos,yPos+50,WHITE,BLACK,SmallFont8x12,g_network.Gateway); // WiFi 기능 제거
    //LCD_printf(xPos,yPos+70,WHITE,BLACK,SmallFont8x12,g_network.SubnetMask); // WiFi 기능 제거             
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doWifiInfoMenu   ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayLedBrightnessMenu(UP_DOWN_MENU *menu, int16 value);

int doLedBrightnessMenu(uint8 reflash)
{        
    static UP_DOWN_MENU menu;
    static int16 brightness = 10;
    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                DisplayLedBrightnessMenu(&menu, brightness);
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                                
                switch(getIndexOfClickedButton(&tc,menu.btn,2))
                {
                    case BOTTOM_LEFT: //return MENU_RETURN_PARENT;  
                        DrawUpDownMessage(&menu, "%d", --brightness);
                        CyDelay(50);
                        break;
                    case BOTTOM_RIGHT: //return MENU_RETURN_PARENT;  
                        DrawUpDownMessage(&menu, "%d", ++brightness);
                        CyDelay(50);                        
                        break;                        
                    break;
                }
                switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                {
                    case BOTTOM_RIGHT: return MENU_RETURN_PARENT;                            
                }                   
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

void DisplayLedBrightnessMenu(UP_DOWN_MENU *menu, int16 value)
{
    DrawUpDownForm(menu, "LED BRIGHT.", "remark", "%d", value);
    SetDrawBottomButtons(NULL, "EXIT");   
}

MENUNODE * menuCreate()
{
//   setDisplayDirection(DISPLAY_DIRECTION_LANDSCAPE);
   setDisplayDirection(DISPLAY_DIRECTION_PORTRAIT);

    // Main Menu  ............................................
   MENUNODE *topMenuNode                  = createMENUNODE(NULL,         &doTopMenu);
   MENUNODE     *infoMenuNode             = createMENUNODE(topMenuNode,  &doInfoMenu);
   MENUNODE         *deviceInfoMenuNode   = createMENUNODE(infoMenuNode, &doDeviceInfoMenu);
   MENUNODE         *wifiInfoMenuNode     = createMENUNODE(infoMenuNode, &doWifiInfoMenu);
   MENUNODE         *ledBrightnessMenuNode= createMENUNODE(infoMenuNode, &doLedBrightnessMenu);

   return topMenuNode;
}

#endif
/* [] END OF FILE */
