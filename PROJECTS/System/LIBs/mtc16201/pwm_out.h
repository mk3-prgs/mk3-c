#include "FreeRTOS.h"
#include "task.h"
//
#include <gd32f10x.h>
//
int d_sat(int d);
int d_led(int d);
//
void set_sat(uint16_t d);
void set_led(uint16_t d);
void PWM_Init(void);
void pwm_test(void* arg);
