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
#include "andonMenu.h"
#include "userMenu.h"
#include "lib/widget.h"
#include "lib/image.h"
#include "menuDesign.h"

#include "package.h"
#include "WIFI.h"

void DisplayAndonMenu()
{
//    char *listArray[] = {"list1", "list2", "list3"};
//    SetDrawListButtons(NULL, listArray, 3);
//    SetDrawBottomButtons("QUIT", NULL, BUTTON_STYLE_R_GREEN, BUTTON_STYLE_R_BLUE);  
}

int doAndonMenu(void *this, uint8 reflash)
{    
    switch(reflash)
    {
        case TRUE: // 화면이 바뀔때 실행
            {
                DisplayAndonMenu();
            }
            break;  
        
        case FALSE: // Cliking Check
            {
                TOUCH tc = GetTouch();
                if(tc.isClick == FALSE) break;
                
//                switch(getIndexOfClickedButtonArray(&tc, &g_btnList))
//                {
//                    case 0: return 0; break;
//                    case 1: return 1; break;
//                    case 2: return 2; break;                    
//                    default:
//                        switch(getIndexOfClickedButton(&tc,g_btnBottom,2))
//                        {
//                            case BOTTOM_LEFT: return MENU_RETURN_PARENT;                            
//                        }
//                    break;
//                }         
            }
            break;
    }
    return MENU_RETURN_PARENT;    
}

MENUNODE * andonMenuCreate(MENUNODE *parent)
{
    // Main Menu  ............................................
   MENUNODE *menuMain                  = createMENUNODE(parent,    "ANDON",     &doAndonMenu);

   return menuMain;
}
/* [] END OF FILE */
