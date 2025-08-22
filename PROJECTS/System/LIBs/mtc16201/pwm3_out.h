#include "FreeRTOS.h"
#include "task.h"
//
#include <gd32f10x.h>
#include "bit_field.h"
//
void set_sol(uint16_t d);
void set_p_sol(uint16_t d);
void PWM3_Init(void);
void pwm3_test(void* arg);
