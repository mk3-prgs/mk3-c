#include "mk2-c.h"
#include <stdint.h>
#include "gd32f10x.h"
#include  "lib.h"

/*
#define SCK_SET  GPIO_SetBits(GPIOB, GPIO_Pin_6)
#define SCK_CLR  GPIO_ResetBits(GPIOB, GPIO_Pin_6)
//
#define DATA_SET  GPIO_SetBits(GPIOB, GPIO_Pin_7)
#define DATA_CLR  GPIO_ResetBits(GPIOB, GPIO_Pin_7)
#define DATA_INP  GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7)
*/
#define SCK_SET  gpio_bit_set(GPIOB, GPIO_PIN_6)
#define SCK_CLR  gpio_bit_reset(GPIOB, GPIO_PIN_6)
//
#define DATA_SET  gpio_bit_set(GPIOB, GPIO_PIN_7)
#define DATA_CLR  gpio_bit_reset(GPIOB, GPIO_PIN_7)
#define DATA_INP  gpio_output_bit_get(GPIOB, GPIO_PIN_7)
//
//
typedef union
{
    unsigned int i;
    float f;
} value;

//----------------------------------------------------------------------------------
// modul-var
//----------------------------------------------------------------------------------
enum {TEMP,HUMI};

#define noACK 0
#define ACK   1
                            //adr  command  r/w
#define STATUS_REG_W 0x06   //000   0011    0
#define STATUS_REG_R 0x07   //000   0011    1
#define MEASURE_TEMP 0x03   //000   0001    1
#define MEASURE_HUMI 0x05   //000   0010    1
#define RESET        0x1e   //000   1111    0

#define _nop_() usleep(30)
//
void s_connectionreset(void);
//
static int sht_inited = 0;
//----------------------------------------------------------------------------------
void s_init(int rst)
{
///GPIO_InitTypeDef  gpio;

    if((sht_inited==0)||(rst != 0)) {
        // I2C1 SDA and SCL configuration
        // SCK
        ///gpio.GPIO_Pin = GPIO_Pin_6;
        ///gpio.GPIO_Speed = GPIO_Speed_50MHz;
        ///gpio.GPIO_Mode = GPIO_Mode_Out_PP;
        ///GPIO_Init(GPIOB, &gpio);
        gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
        //
        // SDA
        ///gpio.GPIO_Pin = GPIO_Pin_7;
        ///gpio.GPIO_Speed = GPIO_Speed_50MHz;
        ///gpio.GPIO_Mode = GPIO_Mode_Out_OD;
        ///GPIO_Init(GPIOB, &gpio);
        gpio_init(GPIOB, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
        //
        s_connectionreset();
        sht_inited = 1;
        }
}
//----------------------------------------------------------------------------------
char s_write_byte(unsigned char data)
//----------------------------------------------------------------------------------
// writes a byte on the Sensibus and checks the acknowledge
{
unsigned char i,error=0;

    for(i=0x80; i>0; i/=2) {     //shift bit for masking
        if(i & data) DATA_SET; //masking value with i , write to SENSI-BUS
        else DATA_CLR;
        _nop_();                //observe setup time
        SCK_SET;                //clk for SENSI-BUS
        _nop_();         //pulswith approx. 5 us
        SCK_CLR;
        _nop_();                //observe hold time
        }

    DATA_SET;
    _nop_();                    //observe setup time
    SCK_SET;
    _nop_();               //clk #9 for ack
    error = DATA_INP;           //check ack (DATA will be pulled down by SHT11)
    SCK_CLR;
    _nop_();
    //
    return error;               //error=1 in case of no acknowledge
}

//----------------------------------------------------------------------------------
char s_read_byte(unsigned char ack)
//----------------------------------------------------------------------------------
// reads a byte form the Sensibus and gives an acknowledge in case of "ack=1"
{
uint8_t i;
uint8_t val;
    DATA_SET;                     //release DATA-line
    //
    val = 0;
    //
    for(i=0x80; i>0; i/=2) {      //shift bit for masking
        SCK_SET;
        _nop_();         //clk for SENSI-BUS
        if(DATA_INP) val |= i;        //read bit
        else val &= ~i;
        _nop_();
        SCK_CLR;
        }
    //
    _nop_();
    if(ack) DATA_CLR;
    else DATA_SET;
    //DATA_PORT = !ack;                        //in case of "ack==1" pull down DATA-Line
    _nop_();                          //observe setup time
    SCK_SET;                            //clk #9 for ack
    _nop_();          //pulswith approx. 5 us
    SCK_CLR;
    _nop_();                          //observe hold time
    DATA_SET;                           //release DATA-line
    return val;
}

//----------------------------------------------------------------------------------
void s_transstart(void)
//----------------------------------------------------------------------------------
// generates a transmission start
//       _____         ________
// DATA:      |_______|
//           ___     ___
// SCK : ___|   |___|   |______
{
   DATA_SET;        //Initial state
   _nop_();
   SCK_SET;
   _nop_();
   DATA_CLR;
   _nop_();
   SCK_CLR;
   _nop_();
   SCK_SET;
   _nop_();
   DATA_SET;
   _nop_();
   SCK_CLR;
}

//----------------------------------------------------------------------------------
void s_connectionreset(void)
//----------------------------------------------------------------------------------
// communication reset: DATA-line=1 and at least 9 SCK cycles followed by transstart
//       _____________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
{
unsigned char i;
    //
    DATA_SET;
    SCK_CLR;            //Initial state
    //
    _nop_(); _nop_(); _nop_();
    for(i=0;i<9;i++) {  //9 SCK cycles
        SCK_SET;
        _nop_(); _nop_();
        SCK_CLR;
        _nop_(); _nop_();
        }
    //
    DATA_SET;
    _nop_();_nop_(); _nop_();
    s_transstart();     //transmission start
}

//----------------------------------------------------------------------------------
char s_softreset(void)
//----------------------------------------------------------------------------------
// resets the sensor by a softreset
{
  unsigned char error=0;
  s_connectionreset();              //reset communication
  error+=s_write_byte(RESET);       //send RESET-command to sensor
  return error;                     //error=1 in case of no response form the sensor
}
//
//----------------------------------------------------------------------------------
char s_read_statusreg(unsigned char *p_value, unsigned char *p_checksum)
//----------------------------------------------------------------------------------
// reads the status register with checksum (8-bit)
{
  unsigned char error=0;
  s_transstart();                   //transmission start
  error=s_write_byte(STATUS_REG_R); //send command to sensor
  *p_value=s_read_byte(ACK);        //read status register (8-bit)
  *p_checksum=s_read_byte(noACK);   //read checksum (8-bit)
  return error;                     //error=1 in case of no response form the sensor
}

//----------------------------------------------------------------------------------
char s_write_statusreg(unsigned char *p_value)
//----------------------------------------------------------------------------------
// writes the status register with checksum (8-bit)
{
  unsigned char error=0;
  s_transstart();                   //transmission start
  error+=s_write_byte(STATUS_REG_W);//send command to sensor
  error+=s_write_byte(*p_value);    //send value of status register
  return error;                     //error>=1 in case of no response form the sensor
}
//
//----------------------------------------------------------------------------------
//char s_measure_beg(int16_t *p_val, uint8_t *p_chk, uint8_t mode)
char s_measure_beg(uint8_t mode)
//----------------------------------------------------------------------------------
// makes a measurement (humidity/temperature) with checksum
{
char error=0;
//
    s_transstart();                   //transmission start
    //
    switch(mode){                     //send command to sensor
        case TEMP	: error += s_write_byte(MEASURE_TEMP); break;
        case HUMI	: error += s_write_byte(MEASURE_HUMI); break;
        default     : break;
        }
    //
    return error;
}

//----------------------------------------------------------------------------------
//char s_measure_end(int16_t *p_val, uint8_t *p_chk, uint8_t mode)
char s_measure_end(int16_t *p_val, uint8_t *p_chk)
//----------------------------------------------------------------------------------
// makes a measurement (humidity/temperature) with checksum
{
char error=0;
    //
    *p_val = ((uint16_t)s_read_byte(ACK))<<8;    //read the first byte (MSB)
    *p_val |= ((uint16_t)s_read_byte(ACK));    //read the second byte (LSB)
    *p_chk = s_read_byte(noACK);  //read checksum
    //
    return error;
}

//----------------------------------------------------------------------------------------
void calc_sth11(float *p_humi ,float *p_temp)
//----------------------------------------------------------------------------------------
// calculates temperature [°C] and humidity [%RH]
// input :  humi [Ticks] (12 bit)
//          temp [Ticks] (14 bit)
// output:  humi [%RH]
//          temp [°C]
{ const float C1 = -2.0468;           // for 12 Bit RH
  const float C2 = +0.0367;           // for 12 Bit RH
  const float C3 = -0.0000015955;     // for 12 Bit RH
  const float T1 = +0.01;             // for 12 Bit RH
  const float T2 = +0.00008;          // for 12 Bit RH

  float rh = *p_humi;             // rh:      Humidity [Ticks] 12 Bit
  float t  = *p_temp;           // t:       Temperature [Ticks] 14 Bit
  float rh_lin;                     // rh_lin:  Humidity linear
  float rh_true;                    // rh_true: Temperature compensated humidity
  float t_C;                        // t_C   :  Temperature [°C]

  t_C = t * 0.01 - 40.1;                //calc. temperature [°C] from 14 bit temp. ticks @ 5V
  //
  rh_lin = C3*rh*rh + C2*rh + C1;     //calc. humidity from ticks to [%RH]
  rh_true = (t_C - 25.0) * (T1 +T2*rh) + rh_lin; //calc. temperature compensated humidity [%RH]
  if(rh_true>100) rh_true=100;       //cut if the value is outside of
  if(rh_true<0.1) rh_true=0.1;       //the physical possible range

  *p_temp = t_C;               //return temperature [°C]
  *p_humi = rh_true;           //return humidity[%RH]
}

//--------------------------------------------------------------------
//float calc_dewpoint(float h,float t)
//--------------------------------------------------------------------
// calculates dew point
// input:   humidity [%RH], temperature [°C]
// output:  dew point [°C]
//{
//float k,dew_point ;
    //
//    k = (log10(h)-2)/0.4343 + (17.62*t)/(243.12+t);
//    dew_point = 243.12*k/(17.62-k);
//    return dew_point;
//}

/*
//----------------------------------------------------------------------------------
extern uint16_t hr_bf[256];

void sht_pool(void *pvParameters)
{
//----------------------------------------------------------------------------------
// sample program that shows how to use SHT11 functions
// 1. connection reset
// 2. measure humidity [ticks](12 bit) and temperature [ticks](14 bit)
// 3. calculate humidity [%RH] and temperature [°C]
// 4. calculate dew point [°C]
// 5. print temperature, humidity, dew point

uint8_t error;
float x,y;
uint8_t status;
uint8_t chk_s;
uint8_t chk_h;
uint8_t chk_t;
int16_t humi_i;
int16_t temp_i;
float humi_f;
float temp_f;

    s_init();
    //
    s_connectionreset();
    //
    while(1) {
        error  = 0;
        //s_connectionreset();
        //error = s_softreset();
        //error = s_write_byte(0);
        //error = s_write_byte(i++);
        error += s_read_statusreg(&status, &chk_s);
        //
        error += s_measure(&humi_i, &chk_h, HUMI);  //measure humidity
        error += s_measure(&temp_i, &chk_t, TEMP);  //measure temperature
        //
        if(error != 0) {
            s_connectionreset();               //in case of an error: connection reset
            hr_bf[a_Ttek] = 0; //-5000;
            hr_bf[a_Vtek] = 0; //-5000;
            }
        else {
            humi_f = (float)humi_i;               //converts integer to float
            temp_f = (float)temp_i;                   //converts integer to float
            calc_sth11(&humi_f, &temp_f);             //calculate humidity, temperature
            //dew_point=calc_dewpoint(humi_f, temp_f);   //calculate dew point
            //printf("temp:%5.1fC humi:%5.1f%% dew point:%5.1fC\n", (double)temp_val.f, (double)humi_val.f, (double)dew_point);
            x = temp_f;
            y = humi_f;
            y = y+x;
            //
            hr_bf[a_Ttek] = (int16_t)(temp_f*100.0);
            hr_bf[a_Vtek] = (int16_t)(humi_f*100.0);
            }
        //
        //xprintf("> stat=%02x %02x  %04x %02x  %04x %02x\r\n", status, chk_s, temp_i, chk_t, humi_i, chk_h);
        //
        if(status) {
            status = 0;
            s_write_statusreg(&status);
            }
        //vTaskDelay(800); //(be sure that the compiler doesn't eliminate this line!)
    //-----------------------------------------------------------------------------------
    }
    //vTaskDelete(NULL);
}
*/

