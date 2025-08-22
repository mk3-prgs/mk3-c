#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdint.h>
#include <string.h>
#include "xprintf.h"

void net_init(void);

void net(char *s)
{
    net_init();
}
