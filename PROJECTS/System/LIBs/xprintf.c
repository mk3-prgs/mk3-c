/*------------------------------------------------------------------------/
/  Universal string handler for user console interface
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#include <string.h>
#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"

#include "xprintf.h"

#define _USE_XFUNC_IN  1
#define _USE_XFUNC_OUT 1

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#if _USE_XFUNC_OUT
#include <stdarg.h>

SemaphoreHandle_t x_io;

void* xdev_out(void* func)
{
void* pf=xfunc_out;
    xSemaphoreTake(x_io, portMAX_DELAY);
    xfunc_out = (void(*)(unsigned char))(func);
    xSemaphoreGive(x_io);
    return(pf);
}

void (*xfunc_out)(unsigned char);	/* Pointer to the output stream */
static char *outptr;

/*----------------------------------------------*/
/* Put a character                              */
/*----------------------------------------------*/

void xputc (char c)
{
	if (_CR_CRLF && c == '\n') xputc('\r');		/* CR -> CRLF */

	if(outptr) {
		*outptr++ = (unsigned char)c;
		return;
        }

	xSemaphoreTake(x_io, portMAX_DELAY);
	if(xfunc_out) xfunc_out((unsigned char)c);
	xSemaphoreGive(x_io);
}



/*----------------------------------------------*/
/* Put a null-terminated string                 */
/*----------------------------------------------*/

void xputs (					/* Put a string to the default device */
	const char* str				/* Pointer to the string */
)
{
	while (*str)
		xputc(*str++);
}


void xfputs (					/* Put a string to the specified device */
	void(*func)(unsigned char),	/* Pointer to the output function */
	const char*	str				/* Pointer to the string */
)
{
	void (*pf)(unsigned char);


	pf = xfunc_out;		/* Save current output device */
	xfunc_out = func;	/* Switch output to specified device */
	while (*str)		/* Put the string */
		xputc(*str++);
	xfunc_out = pf;		/* Restore output device */
}



/*----------------------------------------------*/
/* Formatted string output                      */
/*----------------------------------------------*/
/*  xprintf("%d", 1234);			"1234"
    xprintf("%6d,%3d%%", -200, 5);	"  -200,  5%"
    xprintf("%-6u", 100);			"100   "
    xprintf("%ld", 12345678L);		"12345678"
    xprintf("%04x", 0xA3);			"00a3"
    xprintf("%08LX", 0x123ABC);		"00123ABC"
    xprintf("%016b", 0x550F);		"0101010100001111"
    xprintf("%s", "String");		"String"
    xprintf("%-4s", "abc");			"abc "
    xprintf("%4s", "abc");			" abc"
    xprintf("%c", 'a');				"a"
    xprintf("%f", 10.0);            <xprintf lacks floating point support>
*/

static
void xvprintf (
	const char*	fmt,	/* Pointer to the format string */
	va_list arp			/* Pointer to arguments */
)
{
	unsigned int r, i, j, w, f;
	unsigned long v;
	char s[16], c, d, *p;


	for (;;) {
		c = *fmt++;					/* Get a char */
		if (!c) break;				/* End of format? */
		if (c != '%') {				/* Pass through it if not a % sequense */
			xputc(c); continue;
            }
		f = 0;
		c = *fmt++;					/* Get first char of the sequense */
		if (c == '0') {				/* Flag: '0' padded */
			f = 1; c = *fmt++;
		} else {
			if (c == '-') {			/* Flag: left justified */
				f = 2; c = *fmt++;
			}
		}
		for (w = 0; c >= '0' && c <= '9'; c = *fmt++)	/* Minimum width */
			w = w * 10 + c - '0';
		if (c == 'l' || c == 'L') {	/* Prefix: Size is long int */
			f |= 4; c = *fmt++;
            }
		if (!c) break;				/* End of format? */
		d = c;
		if (d >= 'a') d -= 0x20;
		switch (d) {				/* Type is... */
		case 'S' :					/* String */
			p = va_arg(arp, char*);
			for (j = 0; p[j]; j++) ;
			while (!(f & 2) && j++ < w) xputc(' ');
			xputs(p);
			while (j++ < w) xputc(' ');
			continue;
		case 'C' :					/* Character */
			xputc((char)va_arg(arp, int)); continue;
		case 'B' :					/* Binary */
			r = 2; break;
		case 'O' :					/* Octal */
			r = 8; break;
		case 'D' :					/* Signed decimal */
		case 'U' :					/* Unsigned decimal */
			r = 10; break;
		case 'X' :					/* Hexdecimal */
        case 'P' :
			r = 16; break;
		default:					/* Unknown type (passthrough) */
			xputc(c); continue;
		}

		/* Get an argument and put it in numeral */
		v = (f & 4) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));
		if (d == 'D' && (v & 0x80000000)) {
			v = 0 - v;
			f |= 8;
		}
		i = 0;
		do {
			d = (char)(v % r); v /= r;
			if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
			s[i++] = d + '0';
		} while (v && i < sizeof(s));
		if (f & 8) s[i++] = '-';
		j = i; d = (f & 1) ? '0' : ' ';
		while (!(f & 2) && j++ < w) xputc(d);
		do xputc(s[--i]); while(i);
		while (j++ < w) xputc(' ');
	}
}

void x_out_done(void);

void xprintf (			/* Put a formatted string to the default device */
	const char*	fmt,	/* Pointer to the format string */
	...					/* Optional arguments */
)
{
	va_list arp;


	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);
	//
	x_out_done();
}


int xsprintf (			/* Put a formatted string to the memory */
	char* buff,			/* Pointer to the output buffer */
	const char*	fmt,	/* Pointer to the format string */
	...					/* Optional arguments */
)
{
int len=0;

	va_list arp;


	outptr = buff;		/* Switch destination for memory */

	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);

	len = outptr - buff + 1;

	*outptr = 0;		/* Terminate output string with a \0 */
	outptr = 0;			/* Switch destination for device */
	//
	return(len);
}


void xfprintf (					/* Put a formatted string to the specified device */
	void(*func)(unsigned char),	/* Pointer to the output function */
	const char*	fmt,			/* Pointer to the format string */
	...							/* Optional arguments */
)
{
	va_list arp;
	void (*pf)(unsigned char);


	pf = xfunc_out;		/* Save current output device */
	xfunc_out = func;	/* Switch output to specified device */

	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);

	xfunc_out = pf;		/* Restore output device */
}



/*----------------------------------------------*/
/* Dump a line of binary dump                   */
/*----------------------------------------------*/

void put_dump (
	const void* buff,		/* Pointer to the array to be dumped */
	unsigned long addr,		/* Heading address value */
	int len,				/* Number of items to be dumped */
	int width				/* Size of the items (DW_CHAR, DW_SHORT, DW_LONG) */
)
{
	int i;
	const unsigned char *bp;
	const unsigned short *sp;
	const unsigned long *lp;


	xprintf("%08lX:", addr);		/* address */

	switch (width) {
	case DW_CHAR:
		bp = buff;
		for (i = 0; i < len; i++)		/* Hexdecimal dump */
			xprintf(" %02X", bp[i]);
		xputc(' ');
		for (i = 0; i < len; i++)		/* ASCII dump */
			xputc((bp[i] >= ' ' && bp[i] <= '~') ? bp[i] : '.');
		break;
	case DW_SHORT:
		sp = buff;
		do								/* Hexdecimal dump */
			xprintf(" %04X", *sp++);
		while (--len);
		break;
	case DW_LONG:
		lp = buff;
		do								/* Hexdecimal dump */
			xprintf(" %08LX", *lp++);
		while (--len);
		break;
	}

#if !_LF_CRLF
	xputc('\r');
#endif
	xputc('\n');
}

#endif /* _USE_XFUNC_OUT */



#if _USE_XFUNC_IN
unsigned char (*xfunc_in)(void);	/* Pointer to the input stream */

void* xdev_in(void* func)
{
void* pf=xfunc_in;
    xSemaphoreTake(x_io, portMAX_DELAY);
    outptr = 0;
    xfunc_in = func;
    xSemaphoreGive(x_io);
    //
    return(pf);
}
/*----------------------------------------------*/
/* Get a line from the input                    */
/*----------------------------------------------*/

static char hist[8][64]={0,};
static uint8_t p_hist=0;
extern char cur_path[];

/* 0:End of stream, 1:A line arrived */
int xgets(char* buff, int len)
{
TickType_t l_t;
const TickType_t d_ticks = 10;
//
unsigned int c;
int i;
int esc=0;
int n_hist=p_hist;

	if(!xfunc_in) {
        ///xprintf("No input function specified\n");;
        return 0;		/* No input function specified */
        }
    //
    //xprintf("xgets: Delay ... ");
    l_t = xTaskGetTickCount();
    vTaskDelayUntil(&l_t, d_ticks);
    //xprintf("done!\n");
    //
    i = 0;
    //
	for (;;) {
        xSemaphoreTake(x_io, portMAX_DELAY);
        c = xfunc_in();
        xSemaphoreGive(x_io);
		//xputc(c);
        //buff[i] = 0;
		//
		if(c == 0) {
            vTaskDelayUntil(&l_t, d_ticks);
            //xputc('*');
            continue;
            }
        //
        //xprintf("%02x ", c);
		//
		if(esc == 2) {
            esc=0;
            if(c==0x44) {
                if(i) {
                    i--; // \033[#D
                    xputc(0x1b);
                    xputc('[');
                    xputc('1');
                    xputc('D');
                    //xputc(0x44);
                    }
                }
            else if(c==0x43) {
                if(buff[i]) {
                    i++;
                    xputc(0x1b);
                    xputc('[');
                    xputc('1');
                    xputc('C');
                    //xputc(0x43);
                    }
                }
            else if(c==0x41) {
                //xputc('\r');
                //xputc('>');
                //xputc(0x1b);
                //xputc('[');
                //xputc('J');
                //
                xprintf("\r%s >\033[J", cur_path);
                //
                n_hist--; n_hist &= 0x07;
                memcpy(buff, hist[n_hist], 64);
                xputs(buff);
                i=strlen(buff);
                }
            else if(c==0x42) {
                //xputc('\r');
                //xputc(0x1b);
                //xputc('[');
                //xputc('J');
                //xputc('>');
                xprintf("\r\033[J%s >", cur_path);
                //
                n_hist++; n_hist &= 0x07;
                memcpy(buff, hist[n_hist], 64);
                xputs(buff);
                i=strlen(buff);
                }
            //
            continue;
            }
		//
		if((esc == 0)&&(c == 0x1b)) {
            esc=1;
            continue;
            }
        else if((esc==1)&&(c == 0x5b)) {
            esc=2;
            continue;
            }
        //
		//
		//if(!c) return 0;			/* End of stream? */
		if(c == '\r') {
            xputc(c);
            buff[i++] = 0;
            break;		/* End of line? */
            }
		if(c == '\003') {
            buff[0] = 3;
            buff[1] = 0;
            return(1);
            }
		if((c == 0x7f)||(c == 0x08)) {
            if(i) {
                i--;
                xputc(0x08);
                xputc(' ');
                xputc(0x08);
                }
			continue;
            }
		//
		//if(c >= ' ') {	/* Visible chars */
        xputc(c);
        buff[i++] = c;
        //
        if(i > (len - 1)) {
		    break;
            }
        }
    //
	buff[i] = 0;	/* Terminate with a \0 */

	if((i>0) && (buff[0] != '\0')) {
        memcpy(hist[p_hist], buff, 64);
        p_hist++;
        p_hist &= 0x07;
        /*
        char* p=buff;
        xputc('\r');
        xputc('\n');
        xputc('[');
        while(*p) xprintf("%02x ", *p++);
        xputc(']');
        xputc('\r');
        xputc('\n');
        */
        }
    //for(k=0; k<i; k++) {
    //    xprintf(" %02x", buff[k]);
    //    }

	//xputc('\r');
	//xputc('\n');

	return(i);
}


int xfgets (	/* 0:End of stream, 1:A line arrived */
	unsigned char (*func)(void),	/* Pointer to the input stream function */
	char* buff,	/* Pointer to the buffer */
	int len		/* Buffer length */
)
{
	unsigned char (*pf)(void);
	int n;


	pf = xfunc_in;			/* Save current input device */
	xfunc_in = func;		/* Switch input to specified device */
	n = xgets(buff, len);	/* Get a line */
	xfunc_in = pf;			/* Restore input device */

	return n;
}


/*----------------------------------------------*/
/* Get a value of the string                    */
/*----------------------------------------------*/
/*	"123 -5   0x3ff 0b1111 0377  w "
	    ^                           1st call returns 123 and next ptr
	       ^                        2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                  ^ 6th call fails and returns 0
*/

int xatoi (			/* 0:Failed, 1:Successful */
	char **str,		/* Pointer to pointer to the string */
	long *res		/* Pointer to the valiable to store the value */
)
{
	unsigned long val;
	unsigned char c, r, s = 0;


	*res = 0;

	while ((c = **str) == ' ') (*str)++;	/* Skip leading spaces */

	if (c == '-') {		/* negative? */
		s = 1;
		c = *(++(*str));
        }

	if(c == '0') { // 0x...; 0b...;
		c = *(++(*str));
		switch (c) {
		case 'x':		/* hexdecimal */
			r = 16; c = *(++(*str));
			break;
		case 'b':		/* binary */
			r = 2; c = *(++(*str));
			break;
		default:
			if (c <= ' ') return 1;	/* single zero */
			if (c < '0' || c > '9') return 0;	/* invalid char */
			r = 8;		/* octal */
            }
        }
    else {
		if (c < '0' || c > '9') return 0;	/* EOL or invalid char */
		r = 10;			/* decimal */
        }

	val = 0;
	while(c > ' ') {
		if(c >= 'a') c -= 0x20;
		c -= '0';
		if(c >= 17) {
			c -= 7;
			if (c <= 9) return 0;	/* invalid char */
            }
		if(c >= r) return 0;		/* invalid char for current radix */
		val = val * r + c;
		c = *(++(*str));
        }
    //
	if(s) val = 0 - val;			/* apply sign if needed */

	*res = val;
	return 1;
}

int xatof (char **str, float *res)
{
uint32_t val;
unsigned char c, s = 0;
int del;
int coma=0;

	*res = 0.0;

	while ((c = **str) == ' ') (*str)++;	/* Skip leading spaces */
	//while ((c = **str) == '0') (*str)++;	/* Skip leading 0 */

	if (c == '-') {		/* negative? */
		s = 1;
		c = *(++(*str));
        }

	if((c < '0')||(c > '9')) return 0;	/* EOL or invalid char */

	val = 0; del =1;
	//printf("val=%d del=%d\n", val, del);
	//
	while(c > ' ') {
        if((c == '.')&&(del==1)) {
            coma = 1;
            c = *(++(*str));
            continue;
            }
		c -= '0';
		if(c >= 10) break;		/* invalid char for current radix */
		//
		val = (val * 10) + c;
		if(coma) del *= 10;
		//
		//printf("val=%d\n", val);
		//
		c = *(++(*str));
        }
    //
	//printf("val=%d\n", val);
    //
    *res = ((float)val)/del;
    if(s) *res = 0 - *res;
    //
    //printf("*res=%.3f del=%d\n", *res, del);
    //
	return 1;
}

#endif /* _USE_XFUNC_IN */
