/* ========================================
 *
 * Copyright Suntech, 2023.04.01
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Suntech.
 *
 * ========================================
*/
#include "manageMenu.h"
#include "userMenu.h"
#include "lib/widget.h"
#include "lib/image.h"
#include "lib/externalFlash.h"
#include "lib/internalFlash.h"
#include "lib/LEDControl.h"
#include "lib/w25qxx.h"
#include "menuDesign.h"

#include "package.h"
//#include "../userProjectPatternSewing.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayDeviceInfoMenu();
/////////////////////////////////////////////////////////////////////////////////////////////////
int doDeviceInfo(void *this, uint8 reflash) /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;
    static LIST_MENU menu;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                menu.curPage = 0;
                SetDrawListButtons(&menu,thisMenu->nodeName,NULL,0);

                SetDrawBottomButtons("QUIT", NULL, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE); 
                
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
                //  case BOTTOM_RIGHT: return 1;                            
                }      
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

void DisplayDeviceInfoMenu()
{
    uint16 xPos, yPos;
    BUTTON *btn;
    /////////////////////////////////////////////////////////////////////////////////////      
    BUTTON btnLineName;
    /////////////////////////////////////////////////////////////////////////////////////      
    btn = &btnLineName;
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_2_4,FALSE);
    SetDefaultButtonStyle(btn);
    SetButtonStyleColor(btn,BUTTON_STYLE_TEXT);
    SetButtonText(btn,"Line Name");
    btn->font = Arial_round_16x24;
    btn->foregroundColor = YELLOW;
    btn->align = TEXT_ALIGN_LEFT;
    DrawButton(btn);
    xPos = btn->rect.left+ 20;
    yPos = btn->rect.bottom-10;
//    LCD_printf(xPos,yPos,WHITE,BLACK,SmallFont8x12,g_Info.LineName);        
    
    
    /////////////////////////////////////////////////////////////////////////////////////    
    BUTTON btnMac;    
    /////////////////////////////////////////////////////////////////////////////////////    
    btn = &btnLineName;
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_3_4,FALSE);
    SetDefaultButtonStyle(btn);
    SetButtonStyleColor(btn,BUTTON_STYLE_TEXT);
    SetButtonText(btn,"Mac Address");
    btn->font = Arial_round_16x24;
    btn->foregroundColor = YELLOW;
    btn->align = TEXT_ALIGN_LEFT;
    DrawButton(btn);
    xPos = btn->rect.left+ 20;
    yPos = btn->rect.bottom-10;       
//    LCD_printf(xPos,yPos,WHITE,BLACK,SmallFont8x12,g_network.MAC);    
    
    /////////////////////////////////////////////////////////////////////////////////////
    BUTTON btnVersion; 
    /////////////////////////////////////////////////////////////////////////////////////    
    btn = &btnLineName;
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_4_4,FALSE);
    SetDefaultButtonStyle(btn);
    SetButtonStyleColor(btn,BUTTON_STYLE_TEXT);
    SetButtonText(btn,"Version");
    btn->font = Arial_round_16x24;
    btn->foregroundColor = YELLOW;
    btn->align = TEXT_ALIGN_LEFT;
    DrawButton(btn);
    
    xPos = btn->rect.left+ 20;
    yPos = btn->rect.bottom-10;       
    LCD_printf(xPos,yPos,WHITE,BLACK,SmallFont8x12,PROJECT_FIRMWARE_VERSION);
     
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayWifiInfoMenu();
/////////////////////////////////////////////////////////////////////////////////////////////////
int doWifiInfo(void *this, uint8 reflash) /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{ 
    MENUNODE *thisMenu = (MENUNODE *) this;    
    static LIST_MENU menu;
    static int16 oldRSSI=0;
    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                menu.curPage = 0;
//                oldRSSI = g_network.RSSI;
                SetDrawListButtons(&menu,thisMenu->nodeName,NULL,0);

                SetDrawBottomButtons("QUIT", NULL, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE); 
                
                DisplayWifiInfoMenu();
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                
                switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                {
                    case BOTTOM_LEFT:  return MENU_RETURN_PARENT;
                //  case BOTTOM_RIGHT: return 1;                            
                }      
            }
            break;
    }
    
////    printf("%d %d\r\n",oldRSSI,g_network.RSSI);
//    CyDelay(500);
//    if(oldRSSI != g_network.RSSI)
    {
        DisplayWifiInfoMenu();
//        oldRSSI = g_network.RSSI;
    }
    
    return MENU_RETURN_THIS;    
}

    
void DisplayWifiInfoMenu()
{
    uint16 xPos, yPos;
    BUTTON *btn;
    /////////////////////////////////////////////////////////////////////////////////////      
    BUTTON btnSSID;
    /////////////////////////////////////////////////////////////////////////////////////      
    btn = &btnSSID;
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_1_2, BUTTON_Y_2_4,FALSE);
    SetDefaultButtonStyle(btn);
    SetButtonStyleColor(btn,BUTTON_STYLE_TEXT);
    SetButtonText(btn,"SSID");
    btn->font = Arial_round_16x24;
    btn->foregroundColor = YELLOW;
    btn->align = TEXT_ALIGN_LEFT;
    DrawButton(btn);
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
    {
        btn->rect.top += 30;
        btn->rect.bottom += 20;        
    }
    else
    {
        btn->rect.left = 100;
    }
    btn->rect.right = g_SCREEN_WIDTH-1;    
    
//    SetButtonText(btn,"%13.13s",g_network.SSID);        
    btn->foregroundColor = WHITE;
    btn->align = TEXT_ALIGN_RIGHT;    
    DrawButton(btn);   
    
    /////////////////////////////////////////////////////////////////////////////////////    
    BUTTON btnQuality;    
    /////////////////////////////////////////////////////////////////////////////////////    
    btn = &btnQuality;
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_3_4,FALSE);
    SetDefaultButtonStyle(btn);
    SetButtonStyleColor(btn,BUTTON_STYLE_TEXT);
    SetButtonText(btn,"QUALITY");
    btn->font = Arial_round_16x24;
    btn->foregroundColor = YELLOW;
    btn->align = TEXT_ALIGN_LEFT;
    DrawButton(btn);
    
    if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT)
    {
        btn->rect.top += 30;
        btn->rect.bottom += 20;        
    }
    else
    {
        btn->rect.left = 130;
    }
    btn->rect.right = g_SCREEN_WIDTH-1; 
        
    btn->foregroundColor = WHITE;
    btn->align = TEXT_ALIGN_RIGHT;

//    if(g_network.RSSI == 0 || g_network.RSSI < -30000)
//        SetButtonText(btn,"disconnect");
//    else
//        SetButtonText(btn,"   RSSI:%d",g_network.RSSI);

    DrawButton(btn);    
    
    /////////////////////////////////////////////////////////////////////////////////////
    BUTTON btnIP; 
    /////////////////////////////////////////////////////////////////////////////////////    
    btn = &btnIP;
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_4_4,FALSE);
//    SetDefaultButtonStyle(btn);
//    SetButtonStyleColor(btn,BUTTON_STYLE_TEXT);
//    SetButtonText(btn,"IP");
//    btn->font = Arial_round_16x24;
//    btn->foregroundColor = YELLOW;
//    btn->align = TEXT_ALIGN_LEFT;
//    DrawButton(btn);
    
    xPos = btn->rect.left+ 15;
    yPos = btn->rect.top + 5;       
    LCD_printf(xPos,    yPos+ 0,YELLOW,BLACK,SmallFont8x12,"IP         :");
//    LCD_printf(xPos+100,yPos+ 0,WHITE,BLACK,SmallFont8x12,"%s",g_network.IPv4);    
    LCD_printf(xPos,    yPos+20,YELLOW,BLACK,SmallFont8x12,"Subnet Mask:");
//    LCD_printf(xPos+100,yPos+20,WHITE,BLACK,SmallFont8x12,"%s",g_network.SubnetMask);    
    LCD_printf(xPos,    yPos+10,YELLOW,BLACK,SmallFont8x12,"Gatgeway   :");    
//    LCD_printf(xPos+100,yPos+10,WHITE,BLACK,SmallFont8x12,"%s",g_network.Gateway);  
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int doDisplayDir(void *this, uint8 reflash) ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;    
    static LIST_MENU menu;
    static char *listText[] = {"PORTRAIT", "LANDSCAPE"};

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                menu.curPage = 0;
                if(getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT) 
                    SetDrawListButtons(&menu,thisMenu->nodeName,&listText[1],1);
                else 
                    SetDrawListButtons(&menu,thisMenu->nodeName,&listText[0],1);
                    
                UpdateDrawListButtons(&menu,NO_SELECT);
                SetDrawBottomButtons("QUIT", NULL, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE); 
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                
                uint16 idx = getIndexOfClickedListMenu(&tc, &menu);

                switch(idx)
                {
                    case NO_CLICK :
                        {
                            switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                            {
                                case BOTTOM_LEFT:  return MENU_RETURN_PARENT;
                            //  case BOTTOM_RIGHT: return 1;                            
                            }
                            break;
                        }
                    case 0:
                        
                        setDisplayDirection((getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT) ? DISPLAY_DIRECTION_LANDSCAPE : DISPLAY_DIRECTION_PORTRAIT);
                        ShowWaitMessage();
                        SaveExternalFlashConfig();
                        return MENU_RETURN_THIS_REFLASH; break;
                }                
            }
            break;
    }
    return MENU_RETURN_THIS;    
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doBrightness   //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayDoBrigtness(char *title, UP_DOWN_ONE_MENU *menu);
void DisplayDoBrigtnessSelectValue(UP_DOWN_ONE_MENU *menu, uint8 selectIncre, uint16 value);
void DisplayDoBrigtnessValueChange(UP_DOWN_ONE_MENU *menu, uint16 value);

int doBrightness(void *this, uint8 reflash)
{
    MENUNODE *thisMenu = (MENUNODE *) this;
    static UP_DOWN_ONE_MENU menu;   
    
    static uint16 selectIncre = 0;
    static uint16 value = 50;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {                
                value = getMiscConfig()->uBrightness;
                
                DisplayDoBrigtness(thisMenu->nodeName,&menu);
                DisplayDoBrigtnessSelectValue(&menu, selectIncre, value);
            }
            break;  
        
        case FALSE: // Cliking Check
            {

                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                uint16 indexClick = getIndexOfClickedButton(&tc, menu.btn,8);
                switch(indexClick)
                {                  
                    case 3: // for up
                        if      (value+menu.inc[selectIncre] < 100)
                        {
                            value+=menu.inc[selectIncre];
                            LED_Brightness(value);
                            DisplayDoBrigtnessValueChange(&menu,value);
                        }
                        break;                       
                    case 4: // for down
                        if      (value-menu.inc[selectIncre] > 0)
                        {
                            value-=menu.inc[selectIncre];
                            LED_Brightness(value);
                            DisplayDoBrigtnessValueChange(&menu,value);
                        }
                        break;               
                        break;                        
                    case 5:
                        selectIncre = 0;
                        DisplayDoBrigtnessSelectValue(&menu,selectIncre, value);                         
                        break;
                    case 6:
                        selectIncre = 1;
                        DisplayDoBrigtnessSelectValue(&menu,selectIncre, value);
                        break;
                    case 7:
                        selectIncre = 2;
                        DisplayDoBrigtnessSelectValue(&menu,selectIncre, value);
                        break;                        
                    default:
                        switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                        {
                            case BOTTOM_LEFT:
                                LED_Brightness(getMiscConfig()->uBrightness);
                                return MENU_RETURN_PARENT;
                            case BOTTOM_RIGHT: 
                            {
                                getMiscConfig()->uBrightness = value;
                                LED_Brightness(getMiscConfig()->uBrightness);      
                                ShowWaitMessage();
                                SaveExternalFlashConfig();
                                return MENU_RETURN_PARENT;                                
                            }                            
                        }
                    break;
                }                                  
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

void DisplayDoBrigtness(char *title, UP_DOWN_ONE_MENU *menu)
{
    menu->inc[0] = 1;
    menu->inc[1] = 10;
    menu->inc[2] = 30;
    
    SetDrawUpDownOneMenu(menu, "Bright.", title);
    
    menu->btn[0].disable = TRUE; // for no click title
    menu->btn[1].disable = TRUE; // for no click title
    menu->btn[2].disable = TRUE; // for no click title
    
    SetDrawBottomButtons("QUIT", "SAVE", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
}

void DisplayDoBrigtnessSelectValue(UP_DOWN_ONE_MENU *menu, uint8 selectIncre, uint16 value)
{
    menu->btn[5].foregroundColor = ((selectIncre==0) ? ORANGE : DARKGREY);  // 1 incre
    menu->btn[6].foregroundColor = ((selectIncre==1) ? ORANGE : DARKGREY);  // 2 incre  
    menu->btn[7].foregroundColor = ((selectIncre==2) ? ORANGE : DARKGREY);  // 3 incre  

    DrawButton(&menu->btn[5]);
    DrawButton(&menu->btn[6]);
    DrawButton(&menu->btn[7]);
    
    DisplayDoBrigtnessValueChange(menu,value);    
}

void DisplayDoBrigtnessValueChange(UP_DOWN_ONE_MENU *menu, uint16 value)
{
    SetButtonText(&menu->btn[2],"%d",value);
    DrawButton(&menu->btn[2]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doResetData   //////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void DisplayDoBrigtness(char *title, UP_DOWN_ONE_MENU *menu);
//void DisplayDoBrigtnessSelectValue(UP_DOWN_ONE_MENU *menu, uint8 selectIncre, uint16 value);
//void DisplayDoBrigtnessValueChange(UP_DOWN_ONE_MENU *menu, uint16 value);

int doResetData(void *this, uint8 reflash)
{
    MENUNODE *thisMenu = (MENUNODE *) this;
    static YES_NO_MENU menu;   
    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {                
                strcpy(menu.title,thisMenu->nodeName);
                SetDrawYesNoButtons(&menu, "Do you want ?");
                
                SetDrawBottomButtons("QUIT", "YES", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
            }
            break;  
        
        case FALSE: // Cliking Check
            {

                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;

                switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                {
                    case BOTTOM_LEFT:
                    {
                        return MENU_RETURN_PARENT;
                    }
                    case BOTTOM_RIGHT: 
                    {
                        ResetInternalFlash();                        
                        return MENU_RETURN_PARENT;                                
                    }                            
                }
                               
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

int doFactoryReset(void *this, uint8 reflash)
{
    MENUNODE *thisMenu = (MENUNODE *) this;
    static YES_NO_MENU menu;   
    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {                
                strcpy(menu.title,thisMenu->nodeName);
                SetDrawYesNoButtons(&menu, "Do you want ?");
                
                SetDrawBottomButtons("QUIT", "YES", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
            }
            break;  
        
        case FALSE: // Cliking Check
            {

                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;

                switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                {
                    case BOTTOM_LEFT:
                    {
                        return MENU_RETURN_PARENT;
                    }
                    case BOTTOM_RIGHT: 
                    {
                        ShowMessage("Reset..");
                        W25qxx_EraseChip();
                        ResetInternalFlash();
                        CySoftwareReset();
                        return MENU_RETURN_PARENT;                                
                    }                            
                }
                               
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

int doMonitoring(void *this, uint8 reflash)
{
    MENUNODE *thisMenu = (MENUNODE *) this;
    static uint16 x, y=0;
    static char message[200];
    static int16 indexString;
    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {                
                //strcpy(menu.title,thisMenu->nodeName);
                EraseBlankArea(GetBodyArea().top,FALSE);                  
                SetDrawBottomButtons("QUIT", NULL, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
                x = 0;
                y = GetBodyArea().top;
                indexString = 0;
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                while(UART_SpiUartGetRxBufferSize() > 0 )
                {
                    char c = UART_UartGetChar();
                    if(c == '\0') continue;
                    message[indexString++] = c;
                }
                
                if(message[indexString-1] == '}')
                {
                    for(uint16 i=0; i < indexString; i++)
                    {
                        char c =  message[i];
                        printf("%c",c);
                        if(c == ' ' || c == '\r' || c == '\n') continue;

                        if(x >= g_SCREEN_WIDTH - 10)
                        {
                            x = 0;
                            y += 20;
                        }
                        if(y > GetBodyArea().bottom + 20) y = GetBodyArea().top;
                        
                        LCD_DrawFont(x,y,WHITE,BLACK,SmallFont8x12,c);
                        x += 10;
                    }
                    indexString = 0;
                }
                
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;

                switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
                {
                    case BOTTOM_LEFT:
                    {
                        return MENU_RETURN_PARENT;
                    }          
                }
                               
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

MENUNODE * manageMenuCreate(MENUNODE *parent)
{
    // Main Menu  ............................................
   MENUNODE *root                      = createMENUNODE(parent,    "INFO",          &doListMenu);
         MENUNODE *deviceInfo          = createMENUNODE(root,      "DEVICE INFO",   &doDeviceInfo);
         MENUNODE *wifiInfo            = createMENUNODE(root,      "WIFI INFO",     &doWifiInfo);        
         MENUNODE *settings            = createMENUNODE(root,      "SETTINGS",      &doListMenu);    
             MENUNODE *displayDir      = createMENUNODE(settings,  "DISPLAY DIR",  &doDisplayDir);
             MENUNODE *brightness      = createMENUNODE(settings,  "LED BRIGHT.",   &doBrightness);
             MENUNODE *factoryReset    = createMENUNODE(settings,  "FACT. RESET", &doFactoryReset);
//             MENUNODE *monitoring      = createMENUNODE(settings,  "Monitoring",    &doMonitoring);  
        MENUNODE *resetData            = createMENUNODE(root,      "RESET DATA",    &doResetData);                
        //     MENUNODE *machine         = createMENUNODE(settings,  "MACHINE",       &doMachine);            
   return root;
}
/* [] END OF FILE */
