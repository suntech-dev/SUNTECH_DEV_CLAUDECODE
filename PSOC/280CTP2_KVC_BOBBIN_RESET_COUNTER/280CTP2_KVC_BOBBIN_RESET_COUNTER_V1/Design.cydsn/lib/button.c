#include "button.h"


void SetDefaultButtonStyle(BUTTON *btn)
{
    btn->image           = NULL;    
    btn->font            = Grotesk16x32;
    btn->backgroundColor = BLACK;
    btn->foregroundColor = WHITE;
    btn->outlineColor    = DARKGREY;        
    btn->align           = TEXT_ALIGN_CENTER;
    btn->marginLeftRight = 16;
    btn->marginTopBottom =  0;
    btn->show            = TRUE;
    btn->disable         = FALSE;
    btn->ghost           = FALSE;
}
void SetDefaultButtonStyleArray(BUTTON *btn, uint16 size)
{
    for(int i=0; i < size; i++)
        SetDefaultButtonStyle(&btn[i]);
}
void SetHiddenButtonStyle(BUTTON *btn)
{
    btn->image           = NULL;
    btn->backgroundColor = BLACK;
    btn->foregroundColor = BLACK;
    btn->outlineColor    = BLACK;
    btn->show            = FALSE;
    btn->disable         = TRUE;
}

void DrawButton(BUTTON *btn)
{
    if(btn == NULL) return;
    if(btn->ghost == TRUE) return;
    
    DrawRect(btn->rect, btn->outlineColor);  
    FillRectangle(btn->rect.left+1,btn->rect.top+1,btn->rect.right-1, btn->rect.bottom-1, btn->backgroundColor);
   
    if(btn->show == TRUE)
    {
        if(btn->image != NULL)
        {
            DrawImageRect(&btn->rect, btn->image,btn->backgroundColor);
        }
        DrawButtonText(btn);
    }
}

void DrawButtons(BUTTON *btn, uint16 size)
{
    for(int i=0; i < size; i++)
    {
        DrawButton(&btn[i]);
    }  
}
void DrawButtonArray(BUTTON_ARRAY *array)
{
    for(int i=0; i < array->size; i++)
    {
        BUTTON *btn = array->btn;
        DrawButton(&array->btn[i]);
    } 
}
    
void DrawButtonText(BUTTON *btn)
{
    #define MAX_NUM_BUTTON_STRING 16
    
    uint16 nLength =  (uint16) strlen(btn->text);

    POINT location;
    
    if(nLength==0) return;
    
    uint16 marginLeftRight;
    uint16 marginTopBottom;
    
    if(btn->align == TEXT_TWO_LINE || nLength > MAX_NUM_BUTTON_STRING)
    {
  
    }
    else
    {
        location = CalculationOfLocationForString(btn->rect,
            btn->font[FONT_WIDTH],  btn->font[FONT_HEIGHT],
            btn->marginLeftRight, btn->marginTopBottom,
            nLength, btn->align, TEXT_ALIGN_MIDDLE);

        LCD_printf(location.x, location.y, btn->foregroundColor, btn->backgroundColor, btn->font, btn->text);        
    }    
}

void SetButtonSize(BUTTON *btn, uint16 y, uint16 height, uint8 x_size_type, uint8 y_size_type, uint8 withScroll)
{
    RECT r = GetBodyArea();
    uint16 width = g_SCREEN_WIDTH;
    
    if(withScroll == TRUE) width -= 40;
    
    // Left-Right
    switch(x_size_type)
    {
        case BUTTON_X_TITLE_BAR:
            btn->rect.left = 0;
            btn->rect.right = g_SCREEN_WIDTH - 32;        
            break;
        case BUTTON_X_FULL:
            btn->rect.left = 0;
            btn->rect.right = width-1;     
            break;        
        case BUTTON_X_1_2:
            btn->rect.left = 0;
            btn->rect.right = width / 2 - 1;  
            break;
        case BUTTON_X_2_2:
            btn->rect.left  = width / 2;
            btn->rect.right = width-1;
            break;
        case BUTTON_X_1_3:
            btn->rect.left  = 0;
            btn->rect.right = width/3 - 1;
            break;
        case BUTTON_X_2_3:
            btn->rect.left  = width/3;
            btn->rect.right = width*2/3 - 1;
            break;
        case BUTTON_X_3_3:
            btn->rect.left  = width*2/3;
            btn->rect.right = width     - 1;
            break;
        case BUTTON_X_1_4:
            btn->rect.left  = 0;
            btn->rect.right = width/4 - 1;
            break;
        case BUTTON_X_2_4:
            btn->rect.left  = width/4;
            btn->rect.right = width/2 - 1;
            break;
        case BUTTON_X_3_4:
            btn->rect.left  = width/2;
            btn->rect.right = width*3/4 - 1;
            break;
        case BUTTON_X_4_4:
            btn->rect.left  = width*3/4;
            btn->rect.right = width - 1;
            break;        
    }

    // Top-Bottom
    switch(y_size_type)
    {
        case BUTTON_Y_TOP:
            btn->rect.top = 0;  
            break;
        case BUTTON_Y_FIRST:
            btn->rect.top = r.top;
            break;        
        case BUTTON_Y_HALF:
            btn->rect.top = r.top+(r.bottom-r.top)*0.5; 
            break;
        case BUTTON_Y_2_3:
            btn->rect.top = r.top+(r.bottom-r.top)*0.333;  
            break;
        case BUTTON_Y_3_3:
            btn->rect.top = r.top+(r.bottom-r.top)*0.666;   
            break;
        case BUTTON_Y_2_4:
            btn->rect.top = r.top+(r.bottom-r.top)*0.25;  
            break;
        case BUTTON_Y_3_4:
            btn->rect.top = r.top+(r.bottom-r.top)*0.5;   
            break;
        case BUTTON_Y_4_4:
            btn->rect.top = r.top+(r.bottom-r.top)*0.75;   
            break;             
        case BUTTON_Y_BOTTOM:
            btn->rect.top = r.bottom+1;
            break;
        case BUTTON_Y:
        default:
            btn->rect.top = y;          
            break;
    }
    btn->rect.bottom = btn->rect.top + height - 1;    
}

void SetButtonStyleColor(BUTTON *btn, uint16 styleColor)
{
    SetDefaultButtonStyle(btn);  

    switch(styleColor)
    {                   
        case BUTTON_STYLE_LIST     :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =     BLUE;
            break;
        case BUTTON_STYLE_TITLE    :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =     RED;
            break;        
        case BUTTON_STYLE_EXIT     :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =     BLUE;
            break;             
        case BUTTON_STYLE_SAVE     :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =    GREEN;
            break;
        case BUTTON_STYLE_TEXT     :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =    BLACK;
            btn->outlineColor    =    BLACK;           
            break;
        case BUTTON_STYLE_WHITE    :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =    BLACK;
            btn->outlineColor    =    BLACK;
            break;
        case BUTTON_STYLE_RED      :
            btn->foregroundColor =      RED;
            btn->backgroundColor =    BLACK;
            btn->outlineColor    =    BLACK;
            break;  
        case BUTTON_STYLE_ORANGE   :
            btn->foregroundColor =   ORANGE;
            btn->backgroundColor =    BLACK;
            btn->outlineColor    =    BLACK;
            break;                
        case BUTTON_STYLE_YELLOW   :
            btn->foregroundColor =   YELLOW;
            btn->backgroundColor =    BLACK;
            btn->outlineColor    =    BLACK;
            break;              
        case BUTTON_STYLE_GREEN   :
            btn->foregroundColor =   GREEN;
            btn->backgroundColor =    BLACK;
            btn->outlineColor    =    BLACK;
            break;         
        case BUTTON_STYLE_BLUE     :
            btn->foregroundColor =     BLUE;
            btn->backgroundColor =    BLACK;
            btn->outlineColor    =    BLACK;
            break;          
        case BUTTON_STYLE_LIGHTGREY:
            btn->foregroundColor =LIGHTGREY;
            btn->backgroundColor =    BLACK;
            btn->outlineColor    =    BLACK;
            break;
        case BUTTON_STYLE_DARKGREY:
            btn->foregroundColor = DARKGREY;
            btn->backgroundColor =    BLACK;
            btn->outlineColor    =    BLACK;
            break;          
        case BUTTON_STYLE_R_WHITE  :
            btn->foregroundColor =    BLACK;
            btn->backgroundColor =    WHITE;
            break;        
        case BUTTON_STYLE_R_RED    :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =      RED;
            break;   
        case BUTTON_STYLE_R_ORANGE :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =   ORANGE;
            break;               
        case BUTTON_STYLE_R_YELLOW :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =   YELLOW;
            break;
        case BUTTON_STYLE_R_GREEN :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =    GREEN;
            break;                       
        case BUTTON_STYLE_R_BLUE   :
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =     BLUE;
            break;
        case BUTTON_STYLE_R_LIGHTGREY:
            btn->foregroundColor =    WHITE;
            btn->backgroundColor = LIGHTGREY;
            break;
        case BUTTON_STYLE_R_DARKGREY:
            btn->foregroundColor =    WHITE;
            btn->backgroundColor = DARKGREY;
            break;        
        default:
            btn->foregroundColor =    WHITE;
            btn->backgroundColor =    GREEN;
            btn->outlineColor    = DARKGREY;        
    }
}

void SetButtonText(BUTTON *btn, const char *fmt, ...)
{
    va_list ap;
    char buff[32];
    
    va_start(ap, fmt);

    vsprintf(buff, fmt, ap);
    va_end(ap);
        
    strcpy(btn->text, buff);    
}

void DrawImageRect(RECT *rect, IMAGE *image, uint16 replaceBlackColor)
{
    uint16 xPos = (rect->left+rect->right) * 0.5 - image->width  * 0.5;
    uint16 yPos = (rect->top+rect->bottom) * 0.5 - image->height * 0.5;
    
    DrawImageBackColorReplace(xPos,yPos,image->width,image->height,image->image,replaceBlackColor);
}

uint16 getClickedButton(TOUCH *tc, BUTTON *btn)
{
    if(btn->disable == TRUE) return NO_CLICK;
    
    if(isPointInRect(&btn->rect,tc->point))
    {
        Buzzer(BUZZER_CLICK, 0);
        return TRUE;
    }
    return NO_CLICK;
}

uint16 getIndexOfClickedButtonArray(TOUCH *tc, BUTTON_ARRAY *array)
{
    for(int i=0; i < array->size; i++)
    {
        if(getClickedButton(tc, &array->btn[i]) == TRUE) return i;
    }
    return NO_CLICK;
}

uint16 getIndexOfClickedButton     (TOUCH *tc, BUTTON *btns, uint16 size)
{
    for(int i=0; i < size; i++)
    {
        if(getClickedButton(tc, &btns[i]) == TRUE) return i;
    }
    return NO_CLICK;
}


/*
uint16 getIndexOfClickedButtonArray(TOUCH *tc, BUTTON_ARRAY *array)
{
    for(int i=0; i < array->size; i++)
    {
        BUTTON *btn = &array->btn[i];
        if(btn->disable == TRUE) continue;
        if(isPointInRect(&btn->rect,tc->point))
        {
            Buzzer(BUZZER_CLICK, 0);
            return i;
        }
    }
    return NO_CLICK;
}

uint16 getIndexOfClickedButton     (TOUCH *tc, BUTTON *btns, uint16 size)
{
    for(int i=0; i < size; i++)
    {
        BUTTON *btn = &btns[i];
        if(btn->disable == TRUE) continue;
        if(isPointInRect(&btn->rect,tc->point))
        {
            Buzzer(BUZZER_CLICK, 0);
            return i;
        }
    }
    return NO_CLICK;
}
*/
//void DrawIconButton(ICON_BUTTON *btn)
//{
//    uint16 width = 32;
//    uint16 height = 32;
//    
//    uint16 x_center = (btn->rect.left + btn->rect.right) * 0.5;    
//    uint16 y_center = (btn->rect.top + btn->rect.bottom) * 0.5;
//    
//    FillRect(&btn->rect, btn->backgroundColor);
//    
//    DrawImageBackColorReplace(x_center-width/2, y_center-height/2, width, height, btn->image, btn->backgroundColor);
//}
