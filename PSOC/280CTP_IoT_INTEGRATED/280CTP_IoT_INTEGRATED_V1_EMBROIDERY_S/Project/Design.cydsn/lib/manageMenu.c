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
#include "andonApi.h"
#include "../downtime.h"
#include "../defective.h"
#include "../resetMenu.h"
#include "../uartTestMenu.h"

#include "package.h"
#include "WIFI.h"
#include "server.h"
//#include "../userProjectPatternSewing.h"


/////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayDeviceInfoMenu();
/////////////////////////////////////////////////////////////////////////////////////////////////
int doDeviceInfo(void *this, uint8 reflash) /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;
    static uint16 page = 0;
  //  static LIST_MENU menu;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                g_ListMenu.curPage = page;
                SetDrawListButtons(&g_ListMenu,thisMenu->nodeName,NULL,0, BUTTON_STYLE_LIST);

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
                    case BOTTOM_LEFT:
                    {
                        page = g_ListMenu.curPage;
                        return MENU_RETURN_PARENT;
                    }
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
    ///SetButtonText(btn,"Line Name");
    SetButtonText(btn,"Version");
    btn->font = Arial_round_16x24;
    btn->foregroundColor = YELLOW;
    btn->align = TEXT_ALIGN_LEFT;
    DrawButton(btn);
    xPos = btn->rect.left+ 20;
    yPos = btn->rect.bottom-10;       
    ///LCD_printf(xPos,yPos,WHITE,BLACK,SmallFont8x12,g_Info.LineName);      
    LCD_printf(xPos,yPos,WHITE,BLACK,SmallFont8x12,PROJECT_FIRMWARE_VERSION);       
    
    
    /////////////////////////////////////////////////////////////////////////////////////    
    BUTTON btnMac;    
    /////////////////////////////////////////////////////////////////////////////////////    
    btn = &btnLineName;
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_3_4,FALSE);
    SetDefaultButtonStyle(btn);
    SetButtonStyleColor(btn,BUTTON_STYLE_TEXT);
    SetButtonText(btn,"Mac Address");
    ///SetButtonText(btn,"Ver: KVC3-1.0");
    ///SetButtonText(btn,"Ver: "PROJECT_FIRMWARE_VERSION);
    btn->font = Arial_round_16x24;
    btn->foregroundColor = YELLOW;
    btn->align = TEXT_ALIGN_LEFT;
    DrawButton(btn);
    xPos = btn->rect.left+ 20;
    yPos = btn->rect.bottom-10;       
    LCD_printf(xPos,yPos,WHITE,BLACK,SmallFont8x12,g_network.MAC);    
    
    /////////////////////////////////////////////////////////////////////////////////////
    BUTTON btnVersion; 
    /////////////////////////////////////////////////////////////////////////////////////    
    btn = &btnLineName;
    SetButtonSize(btn,0,g_DEFAULT_BUTTON_HEIGHT, BUTTON_X_FULL, BUTTON_Y_4_4,FALSE);
    SetDefaultButtonStyle(btn);
    SetButtonStyleColor(btn,BUTTON_STYLE_TEXT);
    SetButtonText(btn,"Server IP");
    btn->font = Arial_round_16x24;
    btn->foregroundColor = YELLOW;
    ///btn->foregroundColor = WHITE;
    btn->align = TEXT_ALIGN_LEFT;
    DrawButton(btn);
    
    xPos = btn->rect.left+ 20;
    yPos = btn->rect.bottom-10;       
    ///LCD_printf(xPos,yPos,WHITE,BLACK,SmallFont8x12,PROJECT_FIRMWARE_VERSION);
    LCD_printf(xPos,yPos,WHITE,BLACK,SmallFont8x12,"%s",g_ptrServer->host);
    //wifi_printf("AT*ICT*HTTPGET=http://%s:%d%s%s\r\n",g_ptrServer->IP,g_ptrServer->port,g_ptrServer->path,url_encode(url));
     
}




/////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayWifiInfoMenu();
/////////////////////////////////////////////////////////////////////////////////////////////////
int doWifiInfo(void *this, uint8 reflash) /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{ 
    MENUNODE *thisMenu = (MENUNODE *) this;    
 //   static LIST_MENU menu;
    static uint16 page = 0;    
    static int16 oldRSSI=0;
    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                g_ListMenu.curPage = page;
                oldRSSI = g_network.RSSI;
                SetDrawListButtons(&g_ListMenu,thisMenu->nodeName,NULL,0, BUTTON_STYLE_LIST);

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
                    case BOTTOM_LEFT:
                    {
                        page = g_ListMenu.curPage;
                        return MENU_RETURN_PARENT;
                    }
                //  case BOTTOM_RIGHT: return 1;                            
                }      
            }
            break;
    }
    
//    printf("%d %d\r\n",oldRSSI,g_network.RSSI);
//    CyDelay(500);
    if(oldRSSI != g_network.RSSI)
    {
        DisplayWifiInfoMenu();
        oldRSSI = g_network.RSSI;
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
    
    SetButtonText(btn,"%13.13s",g_network.SSID);        
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
    
    if(g_network.RSSI == 0 || g_network.RSSI < -30000)
        SetButtonText(btn,"disconnect");    
    else
        SetButtonText(btn,"   RSSI:%d",g_network.RSSI);
        
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
    LCD_printf(xPos+100,yPos+ 0,WHITE,BLACK,SmallFont8x12,"%s",g_network.IPv4);    
    LCD_printf(xPos,    yPos+20,YELLOW,BLACK,SmallFont8x12,"Subnet Mask:");
    LCD_printf(xPos+100,yPos+20,WHITE,BLACK,SmallFont8x12,"%s",g_network.SubnetMask);    
    LCD_printf(xPos,    yPos+10,YELLOW,BLACK,SmallFont8x12,"Gatgeway   :");    
    LCD_printf(xPos+100,yPos+10,WHITE,BLACK,SmallFont8x12,"%s",g_network.Gateway);  
}


int doDisplayDir(void *this, uint8 reflash) ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{    
    MENUNODE *thisMenu = (MENUNODE *) this;    

    static char *listText[] = {"PORTRAIT", "LANDSCAPE"};
    static uint16 indexSelect = 0;
    static uint16 page = 0, oldIndexSelect;
    
    if(reflash)
    {         
        oldIndexSelect = indexSelect = (getMiscConfig()->bDisplayDir == DISPLAY_DIRECTION_PORTRAIT) ? 1 : 0;
    }
    
    switch(doSelectList(this, reflash, listText, 2, &indexSelect, &page))
    {
        case 0:
        case 1:
            setDisplayDirection((indexSelect==1) ? DISPLAY_DIRECTION_PORTRAIT : DISPLAY_DIRECTION_LANDSCAPE);
            doSelectList(this, TRUE, listText, 2, &indexSelect, &page);
            break; 
        case MENU_BOTTOM_LEFT: // QUIT
        {            
            setDisplayDirection((oldIndexSelect==1) ? DISPLAY_DIRECTION_PORTRAIT : DISPLAY_DIRECTION_LANDSCAPE);            
            return MENU_RETURN_PARENT;
        }
        case MENU_BOTTOM_RIGHT: // SAVE
        {
            ShowWaitMessage();                                    
            SaveExternalFlashConfig();
            return MENU_RETURN_PARENT;
        }            
    }
    return MENU_RETURN_THIS;    
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// doBrightness   //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
int doBrightness(void *this, uint8 reflash)
{    
    MENUNODE *thisMenu = (MENUNODE *) this;  
 //   static char *strTitle = "LED BRIGHT.";
    static char *strValue = "Bright";
    static int32 minValue = 0, maxValue = 100;
    static int32 incr[] ={1, 10, 30};
    
    static uint16 selectIncre = 0;
    static int32 value = 0;
    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                value = getMiscConfig()->uBrightness;             
            }
            break;  
    }
  
    switch(doIncreseNumberMenu(reflash, thisMenu->nodeName, strValue, incr, &value, minValue, maxValue, &selectIncre))
    {
    case MENU_BOTTOM_LEFT:
        return MENU_RETURN_PARENT;
        break;
    case MENU_BOTTOM_RIGHT:
        {
            getMiscConfig()->uBrightness = value;
            LED_Brightness(getMiscConfig()->uBrightness);      
            ShowWaitMessage();
            SaveExternalFlashConfig();
            return MENU_RETURN_PARENT;           
            return MENU_RETURN_PARENT;
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
                      //  printf("%c",c);
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

int doListMenuRoot(void *this, uint8 reflash)
{
    static uint16 page = 0;
    return doListMenuPage(this, reflash, &page) ;
}

int doListMenuSetting(void *this, uint8 reflash)
{
    static uint16 page = 0;
    return doListMenuPage(this, reflash, &page) ;
}

int doRestart(void *this, uint8 reflash) /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
{
    MENUNODE *thisMenu = (MENUNODE *) this;
    static YES_NO_MENU menu;

    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                strcpy(menu.title, thisMenu->nodeName);
                SetDrawYesNoButtons(&menu, "Do you want ?");
                SetDrawBottomButtons("QUIT", "OK", BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);
            }
            break;

        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;

                switch(getIndexOfClickedButton(&tc, g_btnBottom, 2))
                {
                    case BOTTOM_LEFT:
                    {
                        return MENU_RETURN_PARENT;
                    }
                    case BOTTOM_RIGHT:
                    {
                        ShowWaitMessage();
                        CySoftwareReset();
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
   MENUNODE *root                     = createMENUNODE(parent,    "-- MENU --",    &doListMenuRoot);
        MENUNODE *settings            = createMENUNODE(root,      "SETTINGS",      &doListMenuSetting); 
            //  MENUNODE *displayDir     = createMENUNODE(settings,  "DISPLAY DIR",   &doDisplayDir);
             MENUNODE *brightness     = createMENUNODE(settings,  "LED BRIGHT",    &doBrightness);
        MENUNODE *downtime            = createMENUNODE(root,      "DOWNTIME",      &doDowntime);
            MENUNODE *downtimeRequest = createMENUNODE(downtime,  "REQUEST",       &doDowntimeRequest);
        MENUNODE *defective           = createMENUNODE(root,      "DEFECTIVE",     &doDefective);
            MENUNODE *defectiveRequest= createMENUNODE(defective, "REQUEST",       &doDefectiveRequest);        
        MENUNODE *reset               = createMENUNODE(root,      "RESET",         &doReset);
        MENUNODE *deviceInfo          = createMENUNODE(root,      "DEVICE INFO",   &doDeviceInfo);
        MENUNODE *wifiInfo            = createMENUNODE(root,      "WIFI INFO",     &doWifiInfo);
        MENUNODE *restart             = createMENUNODE(root,      "RESTART",       &doRestart);
        MENUNODE *uartTest            = createMENUNODE(root,      "UART TEST",     &doUartTestMenu);
   return root;
}
/* [] END OF FILE */
