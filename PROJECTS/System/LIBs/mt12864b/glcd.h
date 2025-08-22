#ifndef _GLCD_DEFINED
#define _GLCD_DEFINED
//
#include "font.h"
extern font_t font_L;
extern font_t font_M;
extern font_t font_H;
#include "mt12864b.h"
//
void gsat(char *s);
void glcd_init(char *s);
void glcd(char *s);
void img(char *s);

void gclr(char *s);
void gecho(char *s);
void gfont(char *s);

#endif
