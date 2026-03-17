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
#include "resetMenu.h"
#include "userMenu.h"
#include "lib/widget.h"
#include "lib/image.h"
#include "lib/externalFlash.h"
#include "lib/internalFlash.h"
#include "count.h"

int doReset(void *this, uint8 reflash)
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
                        ResetCount();
                        return MENU_RETURN_PARENT;                                
                    }                            
                }
                               
            }
            break;
    }
    return MENU_RETURN_THIS;    
}

/* [] END OF FILE */
