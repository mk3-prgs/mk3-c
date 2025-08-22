#ifndef __SHT3x_PLATFORM_H__
#define __SHT3x_PLATFORM_H__

#include "FreeRTOS.h"
#include "task.h"

#include <gd32f10x.h>

#include <errno.h>
#include "xprintf.h"

//uint64_t get_os_time(void); xTaskGetTickCount();

#define WRITE_BIT                           0   /*!< I2C master write */
#define READ_BIT                            1   /*!< I2C master read */
#define ACK_CHECK_EN                        0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                       0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                             0x0              /*!< I2C ack value */
#define NACK_VAL                            0x1              /*!< I2C nack value */
#define LAST_NACK_VAL                       0x2              /*!< I2C last_nack value */

#endif // __SHT3x_PLATFORM_H__
