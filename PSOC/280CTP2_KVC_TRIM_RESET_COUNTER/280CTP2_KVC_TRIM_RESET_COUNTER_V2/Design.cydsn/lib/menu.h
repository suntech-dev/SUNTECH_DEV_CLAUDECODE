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
#ifndef __MENU_H_
#define __MENU_H_
    
#include "main.h"
      
#define MENU_RETURN_PARENT         INT_MIN
#define MENU_RETURN_THIS           INT_MIN + 1
#define MENU_RETURN_THIS_REFLASH   INT_MIN + 2
#define MAX_NODE_NAME              20
    
typedef  struct menuNode {
  struct menuNode *parent;
  struct menuNode *firstChild;
  struct menuNode *nextSibling;
  int (*func)(void *this, uint8 reflash);
  char nodeName[MAX_NODE_NAME];
} MENUNODE;

// for MENUNODE ////////////////////////////////////////////////////////
MENUNODE * createMENUNODE(MENUNODE *parent, char *nodeName, int (*func)(void *this, uint8 reflash));
void addMENUNODE(MENUNODE *parent, MENUNODE *newChild);
int getNoOfChild(MENUNODE *parent);
MENUNODE * getNthChild(MENUNODE *parent, int index);

MENUNODE * menuCreate();
void reflashMenu();
// ---------------------------------------------------------------------
void initMenu();
void MenuLoop();

extern MENUNODE *g_MenuNode;
extern MENUNODE *g_TopMenuNode;
#endif
/* [] END OF FILE */
