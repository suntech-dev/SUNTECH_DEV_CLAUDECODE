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
#include "menu.h"

MENUNODE *g_MenuNode;
MENUNODE *g_TopMenuNode;

void initMenu()
{    
    g_MenuNode = menuCreate();
}

//=============================================================================================
#define MAX_MENU_NODE 20
MENUNODE g_MENUNODE[MAX_MENU_NODE];

MENUNODE * createMENUNODE(MENUNODE *parent, char *nodeName, int (*func)(void *this, uint8 reflash))
{
    static uint16 index=0;
    
    if(index >= MAX_MENU_NODE)
    {
        printf("Out of Memory : [%d]\r\n", index);
        return NULL;
    }
    
    MENUNODE *node = &g_MENUNODE[index++];
    
    node->parent       = parent;
    node->func         = func;
    node->firstChild   = NULL;
    node->nextSibling  = NULL;
    strcpy(node->nodeName,nodeName);
    
    if(parent != NULL) addMENUNODE(parent,node);
    
    return node;
}

void addMENUNODE(MENUNODE *parent, MENUNODE *newChild)
{
    MENUNODE *temp;
    
    if(parent->firstChild == NULL)
    {
        parent->firstChild = newChild;
    } else {
        temp = parent->firstChild;
        while(temp->nextSibling != NULL)
        {
            temp = temp->nextSibling;
        }
        temp->nextSibling = newChild;

    }
    newChild->parent = parent;
}

int getNoOfChild(MENUNODE *parent)
{
    MENUNODE *temp = parent->firstChild;    
    int count = 0;
    
    while(temp != NULL)
    {
        count++;
        temp = temp->nextSibling;
    }
    return count;
}

MENUNODE * getNthChild(MENUNODE *parent, int index)
{
    MENUNODE *temp = parent->firstChild;    
    int count = 0;
    
    while(temp != NULL)
    {
        if(count == index) return temp;
        count++;
        temp = temp->nextSibling;
    }
    return NULL;
}

void reflashMenu()
{
    if(g_MenuNode) g_MenuNode->func(g_MenuNode,TRUE);
}

void MenuLoop()
{
    static uint8 reflash = TRUE;

    int subMenu = g_MenuNode->func((MENUNODE *) g_MenuNode, reflash);
    if(reflash) reflash = FALSE;

    if(subMenu == MENU_RETURN_THIS_REFLASH)
    {
        reflash = TRUE;
        subMenu = MENU_RETURN_THIS;
    }

    if(subMenu != MENU_RETURN_THIS) 
    {
        reflash = TRUE;

        if(subMenu == MENU_RETURN_PARENT)
        {
            if(g_MenuNode->parent != NULL) g_MenuNode = g_MenuNode->parent;
        }
        else
        {
             MENUNODE * child = getNthChild(g_MenuNode, subMenu);
             if(child != NULL) g_MenuNode = child;
        }   
   }
}
//---------------------------------------------------------------------------------------

/* [] END OF FILE */
