#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "xprintf.h"
#include "mtc16201/mtc16201.h"
#include "mtc16201/pwm_out.h"
//#include "../shell/dfdata.h"

extern QueueHandle_t kn_queue;

void kn_task(void* arg)
{
uint8_t b;
char str[64];
    //
    PWM_Init();
    lcd_init();
    //set_sat(m_sat);
    //set_led(m_led);
    lcd_print_s("\t");
    marker(32);
    //
    for(;;) {
        xQueueReceive(kn_queue, &b, portMAX_DELAY);
        //xprintf("kn_queue: ret=%d b=%02x\n", ret, b);
        xsprintf(str, "\tНажаты:\nkn = %d", b);
        lcd_print_s(str);
        }
}
