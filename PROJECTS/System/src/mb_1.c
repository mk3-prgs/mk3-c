#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

#include <gd32f10x.h>

#include "usart/usart_bus.h"
#include "modbus/modbus_rtu.h"
#include "utils/net.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "bit_field.h"
#include "xprintf.h"
#include "mb_1.h"

uint16_t uhr[64]; // Регистры user

void init_usart1(modbus_rtu_t* p_mb, usart_bus_t* p_ub, int speed);

static q_rsp mb_rsp;
extern mb_sys mb;

static QueueHandle_t mb_queue=0;
//! Шина USART.
static usart_bus_t usart_bus;
//! Modbus.
static modbus_rtu_t modbus;

/**
 * Сообщения для приёма и передачи Modbus.
 * Можно использовать и одно сообщение, если не
 * планируется обращаться к полученному сообщению
 * при формировании ответа.
 */
static modbus_rtu_message_t modbus_rx_msg;
static modbus_rtu_message_t modbus_tx_msg;

/**
 * Обработчик получения сообщения Modbus.
 */
static void modbus_on_msg_recv(void)
{
static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
static uint8_t status=0;
    // Обработаем сообщение и перенаправим его в зависимости от функции.
    if(modbus.mode == MODBUS_RTU_MODE_MASTER) {
        //int i;
        uint8_t* ptbf = (uint8_t*)modbus_rx_msg.adu.data_and_crc;
        int      len  = modbus_rx_msg.data_size;

        mb_rsp.len  = len + 2;
        mb_rsp.func = modbus_rx_msg.adu.func;
        mb_rsp.addr = modbus_rx_msg.adu.address;

        memcpy(mb_rsp.buff, ptbf, mb_rsp.len);
        //
        status++;
        if(mb_queue != 0) xQueueSendFromISR(mb_queue, &status, &xHigherPriorityTaskWoken);
        //
        modbus.mode = MODBUS_RTU_MODE_SLAVE;
        //
        /* Now the buffer is empty we can switch context if necessary. */
        if( xHigherPriorityTaskWoken ) {
            /* Actual macro used here is port specific. */
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            }
        }
    else {
        modbus_rtu_dispatch(&modbus);
        }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Обработчик чтения регистра флагов.
 * @param address Адрес.
 * @param value Значение.
 * @return Код ошибки Modbus.
 */
static modbus_rtu_error_t modbus_on_read_coil(uint16_t address, modbus_rtu_coil_value_t* value)
{
    // Если не адрес светодиода - возврат ошибки.
    if(address != 0) return MODBUS_RTU_ERROR_INVALID_ADDRESS;

    // Передадим состояние светодиода.
    *value = 1;

    return MODBUS_RTU_ERROR_NONE;
}

/**
 * Обработчик Записи регистра флагов.
 * @param address Адрес.
 * @param value Значение.
 * @return Код ошибки Modbus.
 */
static modbus_rtu_error_t modbus_on_write_coil(uint16_t address, modbus_rtu_coil_value_t value)
{
    // Если не адрес светодиода - возврат ошибки.
    if(address != 0) return MODBUS_RTU_ERROR_INVALID_ADDRESS;

    // Зажжём или погасим светодиод
    // в зависимости от значения.
    //if(value) led_on(LED0);
    //else led_off(LED0);

    return MODBUS_RTU_ERROR_NONE;
}

/**
 * Обработчик чтения регистра хранения.
 * @param address Адрес.
 * @param value Значение.
 * @return Код ошибки Modbus.
 */
static modbus_rtu_error_t modbus_on_read_hold_reg(uint16_t mb_addr, uint16_t* value)
{
    uint32_t a_max = sizeof(mb_sys)-2;
    if(mb_addr <= a_max) {
        /// file_io
        uint16_t* pt = (uint16_t*)&mb;
        *value = pt[mb_addr];
        //xprintf("r: a=%04x d=%04x\n", mb_addr, *value);
        }
    else if((mb_addr >= 0x1000)&&(mb_addr <= 0x1040)) {
        /// доступ к регистрам user
        *value = uhr[mb_addr - 0x1000];
        }
    else {
        return MODBUS_RTU_ERROR_INVALID_ADDRESS;
        }
//
    return(MODBUS_RTU_ERROR_NONE);
}

/**
 * Обработчик записи регистра хранения.
 * @param address Адрес.
 * @param value Значение.
 * @return Код ошибки Modbus.
 */
static modbus_rtu_error_t modbus_on_write_hold_reg(uint16_t mb_addr, uint16_t value)
{
    //  addr указывает на uint8_t
    //
    uint32_t a_max = sizeof(mb_sys)-2;
    //
    if(mb_addr < a_max) {
        /// file_io
        uint16_t* pt = (uint16_t*)&mb;
        pt[mb_addr] = value;
        //xprintf("w: a=%04x d=%04x\n", mb_addr, value);
        }
    else if((mb_addr >= 0x1000)&&(mb_addr <= 0x1040)) {
        uhr[mb_addr - 0x1000] = value;
        /// доступ к регистрам user
        }
    else {
        return MODBUS_RTU_ERROR_INVALID_ADDRESS;
        }
//
    return(MODBUS_RTU_ERROR_NONE);
}

/**
 * Обработчик передачи идентификатора ведомого устройства.
 * @param slave_id Идентификатор ведомого устройства.
 * @return Код ошибки Modbus.
 */
static modbus_rtu_error_t modbus_on_report_slave_id(modbus_rtu_slave_id_t* slave_id)
{
    // Состояние - работаем.
    slave_id->status = MODBUS_RTU_RUN_STATUS_ON;
    // Идентификатор - для пример возьмём 0xaa.
    slave_id->id = 0xaa;
    // В дополнительных данных передадим наше имя.
    slave_id->data = "GD32F1x MCU Modbus v1.0";
    // Длина имени.
    slave_id->data_size = strlen(slave_id->data);

    return MODBUS_RTU_ERROR_NONE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Инициализация Modbus.
 */
static void init_modbus(uint8_t saddr)
{
    // Структура инициализации Modbus.
    modbus_rtu_init_t modbus_is;

    modbus_is.usart = &usart_bus; // Шина USART.
    modbus_is.mode = MODBUS_RTU_MODE_SLAVE; // Режим - ведомый.
    modbus_is.address = saddr; // Адрес.
    modbus_is.rx_message = &modbus_rx_msg; // Сообщение для приёма.
    modbus_is.tx_message = &modbus_tx_msg; // Сообщение для передачи.

    // Инициализируем Modbus.
    modbus_rtu_init(&modbus, &modbus_is);
    // Установка каллбэка получения сообщения.
    modbus_rtu_set_msg_recv_callback(&modbus,          modbus_on_msg_recv);
    // Установка каллбэков доступа к данным.
    modbus_rtu_set_read_coil_callback(&modbus,         modbus_on_read_coil);
    modbus_rtu_set_write_coil_callback(&modbus,        modbus_on_write_coil);
    modbus_rtu_set_report_slave_id_callback(&modbus,   modbus_on_report_slave_id);
    modbus_rtu_set_read_holding_reg_callback(&modbus,  modbus_on_read_hold_reg);
    modbus_rtu_set_write_holding_reg_callback(&modbus, modbus_on_write_hold_reg);
}

void mb1_init(uint8_t slave, int speed)
{
    // Инициализация.
    //
    init_usart1(&modbus, &usart_bus, speed);
    init_modbus(slave);
    //
    mb_queue = xQueueCreate(4, sizeof(uint8_t));
	///if(mb_queue == 0 ) {
	///	xprintf("Queue was not created and must not be used.\r\n");
    ///    }
    ///xprintf("\r\nCreate mb_queue=0x%x\r\n", mb_queue);
}

/// //////////////////////////////////////////////////////////////////////////////////
///
/// Master functions
///

int mb1_read_holding_regs(uint8_t slave, uint16_t addr, uint16_t* buff, size_t size)
{
int k;
//int i=0;
//char c=0;
char stat=0;
int ret=0;
int len=0;

uint8_t bf[256];
    //
    if(mb_queue == 0) return(0);
    //
    bf[0] = addr>>8;
    bf[1] = addr;
    bf[2] = size>>8;
    bf[3] = size;
    //bf[4] = 2*size;
//
    //bf[5] = buff[0]>>8;
    //bf[6] = buff[0];
    //bf[7] = buff[1]>>8;
    //bf[8] = buff[1];
//
    modbus.mode = MODBUS_RTU_MODE_MASTER;
//
    modbus_rtu_message_set_address(&modbus_tx_msg, slave);
    modbus_rtu_message_set_func(&modbus_tx_msg, MODBUS_RTU_FUNC_READ_HOLDING_REGS);
    modbus_rtu_message_copy_data(&modbus_tx_msg, (const void*)bf, 4);
    modbus_rtu_message_calc_crc(&modbus_tx_msg);
    //
    modbus_rtu_send_message(&modbus);
    //
    //for(i=0; i<5; i++) {
        ret = xQueueReceive(mb_queue, &stat, 20);
        //xprintf("mb_queue: ret=%d\r\n", ret); //uxQueueMessagesWaiting(mb_queue));
        //
        if(ret == pdPASS) { // pdPASS
            //marker(0);
            //xprintf("stat=%d func=%02x address=%02x: [ ", stat, mb_rsp.func, mb_rsp.addr);
            //for(k=0; k<mb_rsp.len; k++)  xprintf("%02x ", mb_rsp.buff[k]);
            //xprintf("]\r\n");
            //
            len = (mb_rsp.buff[0]) / 2;
            for(k=0; k<len; k++) {
                buff[k] = mb_rsp.buff[2*k+1];
                buff[k] <<= 8;
                buff[k] += mb_rsp.buff[2*k+2];
                }
            //
            ret=len;
            //break;
            }
        else { // errQUEUE_EMPTY
            //
            //if(c == ':') c = '*';
            //else c = ':';
            //
            //char p = marker(15);
            //putc_lcd(c);
            //marker(p);
            //break;
            //xprintf("Wait mb Rx! slave=%d ret=%d\r\n",  slave, ret);
            ret=-1;
            }
    //    }
    //
    return(ret);
}

int mb1_write_holding_regs(uint8_t slave, uint16_t addr, uint16_t* buff, size_t size)
{
int i;
//int k;
//char c=0;

char stat=0;
uint8_t bf[256];
err_t ret;
    //
    if(mb_queue == 0) return(-1);
    if(size == 0) return(-1);
    //
    bf[0] = addr>>8;
    bf[1] = addr;
    bf[2] = size>>8;
    bf[3] = size;
    bf[4] = 2*size;
    //
    for(i=0; i<(2*size); i++) {
        bf[(2*i)+5] = buff[i]>>8;
        bf[(2*i)+6] = buff[i];
        }
    //
    modbus.mode = MODBUS_RTU_MODE_MASTER;
    //
    modbus_rtu_message_set_address(&modbus_tx_msg, slave);
    modbus_rtu_message_set_func(&modbus_tx_msg, MODBUS_RTU_FUNC_WRITE_MULTIPLE_REGS);
    modbus_rtu_message_copy_data(&modbus_tx_msg, (const void*)bf, 5 + 2*size);
    modbus_rtu_message_calc_crc(&modbus_tx_msg);
    //
    ret = modbus_rtu_send_message(&modbus);
    if(ret) {
        //xprintf("Errot mb Tx!\r\n");
        return(-1);
        }
    //
    ret=-1;
    //
    //for(i=0; i<5; i++) {
        ret = xQueueReceive(mb_queue, &stat, 20);
        //xprintf("mb_queue: ret=%d\r\n", ret); //uxQueueMessagesWaiting(mb_queue));
        //
        if(ret == pdPASS) { // pdPASS
            //marker(0);
            //xprintf("stat=%d func=%02x address=%02x ", stat, mb_rsp.func, mb_rsp.addr);
            //for(k=0; k<mb_rsp.len; k++)  xprintf("%02x ", mb_rsp.buff[k]);
            //xprintf("Slave=%d is Ok! ret=%d\r\n", slave, ret);
            //lcd_print_s(str);
            //marker(16);
            ret = size;
            //break;
            }
        else { // errQUEUE_EMPTY
            //
            //if(c == ':') c = '*';
            //else c = ':';
            //
            //char p = marker(15);
            //putc_lcd(c);
            //marker(p);
            //break;
            //xprintf("Wait mb Tx! slave=%d ret=%d\r\n",  slave, ret);
            ret=-1;
            }
        //}
    //
    return(ret);
}
