#ifndef _SHELL_DEFINED
#define _SHELL_DEFINED

#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"
//
#include <string.h>
#include "ff.h"
#include "rtc_32f1.h"
//
#include "xprintf.h"
#include "usart.h"
#include "diskio.h"
//
#include "mtc16201/mtc16201.h"
#include "mtc16201/pwm_out.h"
#include "mtc16201/pwm3_out.h"
//
#include "spi_io.h"

typedef struct __ptentry__
{
    char *commandstr;
    void (* pfunc)(char *str);
    char *help;
    uint8_t task;
} ptentry;

/*---------------------------------------------------------------------------*/
int load(char* name);
///=============================================================================
#include "shell_config.h"

#ifdef CONFIG_SYS
    #include "sys.h"
#endif

#ifdef CONFIG_FILE_OP
    #include "file_op.h"
#endif

#ifdef CONFIG_DISK_OP
    #include "disk_op.h"
#endif

#ifdef CONFIG_OTHER
    #include "other.h"
#endif

#ifdef CONFIG_SD_CARD
    #include "sd_card.h"
#endif

#ifdef CONFIG_ADC
    #include "adc.h"
#endif

#ifdef CONFIG_NET
    #include "net.h"
#endif

#ifdef CONFIG_DIO
    #include "d_io.h"
#endif

#ifdef CONFIG_IO_TASK
    #include "io_task.h"
#endif

#ifdef CONFIG_SENSORS
    #include "sens.h"
#endif

#ifdef CONFIG_DAC
    #include "dac.h"
#endif

#ifdef CONFIG_GLCD
    #include "glcd.h"
#endif
///=============================================================================

#endif

