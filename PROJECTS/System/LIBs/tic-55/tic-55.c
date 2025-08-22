#include "xprintf.h"
#include "tic-55.h"
#include "bit_field.h"
//
volatile uint8_t b10_mc;
//
//=======================================================================================================
static char hex_tab[]=
{
	'0','1','2','3','4','5','6','7',
 	'8','9','a','b','c','d','e','f',
	' '
};

static const unsigned char cg_tab[]=
{
0x7b, //0
0x60, //1
0x37, //2
0x75, //3
0x6c, //4
0x5d, //5
0x5f, //6
0x70, //7
0x7f, //8
0x7d, //9
0x7e, //a
0x4f, //b
0x1b, //c
0x67, //d
0x1f, //e
0x1e, //f
0x00  //
};

char toHex(char c)
{
	return(hex_tab[(int)c]);
}

#define TIC55_CLK GPIO_PIN_7
#define CLK     GET_BIT_BB(GPIOB+12,7)

#define TIC55_OUT GPIO_PIN_11
#define OUT     GET_BIT_BB(GPIOB+12,11)

#define TIC55_LD GPIO_PIN_6
#define LD       GET_BIT_BB(GPIOB+12,6)

void tic55_init(void)
{
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, TIC55_CLK | TIC55_OUT | TIC55_LD);

	CLK=1;
	OUT=0;
	LD=1;
}

void tic55_outbyte(unsigned int c)
{
char n;
	for(n=0;n<9;n++) {
		if(0x100&c) OUT=1;
		else OUT=0;
		c *= 2;
		//
		_delay_us(5);
		CLK=0;
		_delay_us(5);
		CLK=1;
		}
}

void print_d(char n, unsigned int d)
{
int i;
	for(i=0; i<n; i++) {
		tic55_outbyte(0x80 | cg_tab[d%10]);
		d /= 10;
		}
	//
	for(i=n; i<8; i++) tic55_outbyte(0x80);
	//
	LD=0;
	_delay_us(2);
	LD=1;
}

void print_f(int x)
{
int i;
    tic55_outbyte(0);
	tic55_outbyte(0);
	//
    for(i=0; i<4; i++) {
        if(i==1) tic55_outbyte(0x100 | cg_tab[x%10]);
        else tic55_outbyte(cg_tab[x%10]);
        x /= 10;
        }
	//
	tic55_outbyte(0);
	tic55_outbyte(0);
	//
	LD=0;
	_delay_us(2);
	LD=1;
}

void print_tv(int t, int v)
{
int i;
    for(i=0; i<3; i++) {
        if(i==1) tic55_outbyte(0x100 | cg_tab[v%10]);
        else tic55_outbyte(cg_tab[v%10]);
        v /= 10;
        }
	//
	tic55_outbyte(0);
	//
	if(t>=0) {
        for(i=0; i<4; i++) {
            if(i==2) tic55_outbyte(0x100 | cg_tab[t%10]);
            else tic55_outbyte(cg_tab[t%10]);
            t /= 10;
            }
        }
    else {
        t=-t/10;
        for(i=0; i<3; i++) {
            if(i==1) tic55_outbyte(0x100 | cg_tab[t%10]);
            else tic55_outbyte(cg_tab[t%10]);
            t /= 10;
            }
        tic55_outbyte(0x04);
        }
	//
	LD=0;
	_delay_us(2);
	LD=1;
}

/*
void adc_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // разрешение тактирование АЦП

    RCC->CFGR &= ~RCC_CFGR_ADCPRE_0;  // предделитель АЦП = 10 (/6)
    RCC->CFGR |=  RCC_CFGR_ADCPRE_1;

    ADC1->CR1 = 0;      // запрещаем все в управляющих регистрах
    ADC1->CR2 = 0;

    ADC1->CR2 |= ADC_CR2_ADON; // разрешить АЦП

    ADC1->SMPR2 |= ADC_SMPR2_SMP0_0 | ADC_SMPR2_SMP0_1 ; // время выборки 28,5 циклов
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP0_2 ;
    ADC1->SMPR2 |= ADC_SMPR2_SMP1_0 | ADC_SMPR2_SMP1_1 ; // время выборки 28,5 циклов
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP1_2 ;
    ADC1->SMPR2 |= ADC_SMPR2_SMP2_0 | ADC_SMPR2_SMP2_1 ; // время выборки 28,5 циклов
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP2_2 ;
    ADC1->SMPR2 |= ADC_SMPR2_SMP3_0 | ADC_SMPR2_SMP3_1 ; // время выборки 28,5 циклов
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP3_2 ;

    // выбор каналов
    ADC1->SQR1 =0; // 1 регулярный канал
    ADC1->SQR3 =0; // 1е преобразование - канал 0
    ADC1->JSQR =0; // 1 инжектированный канал
    ADC1->JSQR |= ADC_JSQR_JSQ4_0; // 1е преобразование - канал 1

    // калибровка
    ///delayMks(10); // задержка 10 мкс
    ///ADC1->CR2 |= ADC_CR2_CAL; // запуск калибровки
    ///while ((ADC1->CR2 & ADC_CR2_CAL) != 0) ; // ожидание окончания калибровки

    ADC1->CR2 |= ADC_CR2_EXTSEL; // источник запуска - SWSTART
    ADC1->CR2 |= ADC_CR2_EXTTRIG; // разрешение внешнего запуска для регулярных каналов
}

float VREF=3.3;

float adc(void)
{
float VREF=3.3;
    //--------------- Режим один регулярный канал, однократное преобразование
    ADC1->CR2 &= ~ADC_CR2_ADON; // запретить АЦП

    // выбор каналов
    ADC1->SQR1 =0; // 1 регулярный канал
    ADC1->SQR3 =0; // 1 преобразование - канал 0

    ADC1->CR2 &= ~ADC_CR2_CONT; // запрет непрерывного режима
    ADC1->CR1 &= ~ADC_CR1_SCAN; // запрет режима сканирования

    ADC1->CR2 |= ADC_CR2_ADON; // разрешить АЦП

    // измерение сигнала
    ADC1->CR2 |= ADC_CR2_SWSTART; // запуск АЦП
    while(!(ADC1->SR & ADC_SR_EOC)) ; // ожидание завершения преобразования

    float res= (float)ADC1->DR * VREF / 4096. ; // пересчет в напряжение

    return(res);
}

void adc_0(void)
{
    //--------------- Режим один регулярный канал, непрерывное преобразование

    ADC1->CR2 &= ~ADC_CR2_ADON; // запретить АЦП

    // выбор каналов
    ADC1->SQR1 =0; // 1 регулярный канал
    ADC1->SQR3 =0; // 1 преобразование - канал 0

    ADC1->CR2 |= ADC_CR2_CONT; // разрешение непрерывного режима
    ADC1->CR1 &= ~ADC_CR1_SCAN; // запрет режима сканирования

    ADC1->CR2 |= ADC_CR2_ADON; // разрешить АЦП

    float res; // переменная для результата
    uint8_t str[32]; // строка

    ADC1->CR2 |= ADC_CR2_SWSTART; // запуск АЦП

    while(1) {
        // основной цикл
        res= (float)ADC1->DR * VREF / 4096. ; // пересчет в напряжение

        // вывод на LCD
        xprintf("%d.%03d V", (uint16_t)res, ((uint16_t)((res - (uint16_t)res)*1000.)) );
        }
}
*/
