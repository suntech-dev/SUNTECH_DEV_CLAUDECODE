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
#ifndef _ANDON_MENU_H_
#define _ANDON_MENU_H_

#include "main.h"
#include "lib/menu.h"
    
MENUNODE * andonMenuCreate(MENUNODE *parent);
int doAndonSet(void *this, uint8 reflash);
#endif
/* [] END OF FILE */
