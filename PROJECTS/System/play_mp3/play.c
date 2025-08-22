#include "includes.h"
#include "usart.h"

const char version[4] = "3.3\0";

#include "playerTask.h"
#include "dacTask.h"

void format_cmd(char *cmd, int numb);
cmd_t msg=CMD_NONE;

extern QueueHandle_t cmdEvent;
extern QueueHandle_t dac_Cmd;
extern QueueHandle_t mp3_Cmd;

void play(void* arg)
{
char cmd[64];
int val=0;
    arg=arg;
    //
	usart_init();
	xdev_out(x_putc);
	xdev_in(x_getc);
	//
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
                                                          GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	//
	xprintf("\n\n");
	xprintf("===============================================\r\n");
	xprintf("     STM32-MP3-плейер\n");
	xprintf("     v%s\n", version);
	xprintf("===============================================\r\n");
	//
	xprintf("CPU clock %u\n", SystemCoreClock);

	uint16_t os_version = 0; //CoGetOSVersion();
	uint8_t Major = ((os_version>>12)&0xF) * 10 + ((os_version>>8)&0xF);
	uint8_t Minor = ((os_version>>4)&0xF) * 10 + (os_version&0xF);
	xprintf("CoOS version: %u.%u\r\n", Major, Minor);
    //
	playerCtrlInit();
	//
	DAC_TaskInit();

	for(;;) {
        xprintf("\n# ");
        memset(cmd, 0, 64);
        xgets(cmd, 63);
        //
        //TEST4 = !TEST4;
        //
        xprintf("\n");
        format_cmd(cmd, 64);
        ///xprintf("cmd: [%s]\n", cmd);
        //
        if(!strncmp(cmd, "init", 4)) {
            xprintf("init:\n");
            }
        else if(!strncmp(cmd, "val", 3)) {
            long l;
            char *pt=&cmd[4];
            if(xatoi(&pt, &l)) {
                val=l;
                }
            val &= 0xffff;
            xprintf("val=%u\n", val);
            timer_autoreload_value_config(TIMER5, val);
            timer_autoreload_value_config(TIMER6, val);
            xprintf("\n");
            }
        else if(!strncmp(cmd, "cmd", 3)) {
            long l;
            char *pt=&cmd[4];
            if(xatoi(&pt, &l)) {
                msg=l;
                }
            //
            if(cmdEvent != 0) {
                int res = xQueueSend(cmdEvent, &msg, 10);
                if(res != pdTRUE) xprintf("Failed to send data, Queue full.\n");
                }
            }
        else if(!strncmp(cmd, "smp", 3)) {
            long l;
            char *pt=&cmd[4];
            dac_srate_t smp;
            smp.dac_cmd=DAC_SET_SRATE;
            smp.sample_rate = 48000;
            if(xatoi(&pt, &l)) {
                smp.sample_rate=l;
                }
            //
            if(cmdEvent != 0) {
                int res = xQueueSend(dac_Cmd, &smp, 10);
                if(res != pdTRUE) xprintf("Failed to send data, Queue full.\n");
                }
            }
        else if(!strncmp(cmd, "heap", 4)) {
            xprintf("Heap: %d\n", xPortGetFreeHeapSize());
            }
        else {
            TEST5 = !TEST5;
            dac_data_portion_t cmd;
            cmd.dac_cmd = DAC_START;
            xprintf("dac_DataReqFlag=0\n");
            if(dac_Cmd != 0) {
                int res = xQueueSend(dac_Cmd, &cmd, 10);
                if(res != pdTRUE) xprintf("Failed to send data, Queue full.\n");
                timer_enable(TIMER3);
                }
            int xxx=1;
            xQueueSend(mp3_Cmd, &xxx, 10);
            }
        }
}
