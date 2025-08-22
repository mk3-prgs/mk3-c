#ifndef _FONT_DEFINED
#define _FONT_DEFINED

#include <stdint.h>

typedef struct _tf_ {
int w;
int h;
int n_x;
int n_y;
int w_f;
int s_x;
int sz;
uint8_t* gl;
} font_t;

#endif
