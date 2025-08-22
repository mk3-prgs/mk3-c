#ifdef OLD_BOARD

#include <stdint.h>
#include "bit_field.h"

//#define K2 (GPIOA->IDR & GPIO_Pin_0)
//#define K1 (GPIOB->IDR & GPIO_Pin_3)
//#define K3 (GPIOB->IDR & GPIO_Pin_4)

#define K1 GET_BIT_BB(GPIOA+8,0)
#define K2 GET_BIT_BB(GPIOB+8,3)
#define K3 GET_BIT_BB(GPIOB+8,4)

//static unsigned char b_repit;
//static unsigned char cs;

void init_3kn(void)
{
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, 0,  GPIO_PIN_0);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, 0,  GPIO_PIN_3 | GPIO_PIN_4);
}

uint8_t kn_input(void)
{
uint8_t c=0;
    //
	if((K1==0)&&(K2==0)&&(K3==0)) c='\t';
	else if((K1==0)&&(K2==0)) c='\r';
	else if((K1==0)&&(K3==0)) c='\n';
	else if(K1==0) c='P';
	else if(K2==0) c='+';
	else if(K3==0) c='-';
    else c=0;
    /*
	if(c==cs) {
		if((c=='+')||(c=='-')) b_repit++;
		else b_repit = 0;
		//
		if(b_repit >= 100) {b_repit = 97; return(c);}
		return(0);
		}
	else {
		cs=c;
		b_repit = 0;
		}
    */
	return(c);
}
#endif // OLD_BOARD
