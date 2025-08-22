#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include <errno.h>
#include <modbus.h>

// Контекст Modbus.
static modbus_t* modbus = NULL;

int fmb_open(char* dev, int nom, int speed)
{
    ///modbus_new_rtu(const char *device, int baud, char parity, int data_bit, int stop_bit, int rts);
    modbus = modbus_new_rtu(dev, speed, 'N', 8, 1, 0);
    if(!modbus){
        printf("Error creating modbus context!\n");
        return(-1);
        }
    modbus_set_debug(modbus, 0);
    // Режим - RS-232.
    modbus_rtu_set_serial_mode(modbus, MODBUS_RTU_RS232);
    // Установим адрес ведомого.
    modbus_set_slave(modbus, nom);
    //
    //modbus_get_response_timeout(modbus, &response_timeout);
    //response_timeout.tv_sec = 0;
    //response_timeout.tv_usec = 1000000;
    //
    modbus_set_response_timeout(modbus, 1, 500000); //&response_timeout);
    //
    // Соединимся.
    if(modbus_connect(modbus) == -1){
        printf("Error connecting to slave!: %d\n", errno);
        printf(modbus_strerror(errno));
        modbus_close(modbus);
        modbus_free(modbus);
        return(-1);
        }
    return(0);
}

int fmb_tx(int addr, uint16_t *bf, int len)
{
    //printf("addr=%u len=%d\n", addr, len);
    if(modbus_write_registers(modbus, addr, len, bf) == -1) {
        printf("Error writing reg hold register: %d!\n", errno);
        printf(modbus_strerror(errno));
        printf("\n");
        return(-1);
        }
    return(0);
}

int fmb_rx(int addr, uint16_t *bf, int len)
{
    //printf("addr=%u d=%d\n", addr, len);
    if(modbus_read_registers(modbus, addr, len, bf) == -1) {
        printf("Error reading reg hold register: %d!\n", errno);
        printf(modbus_strerror(errno));
        printf("\n");
        return(-1);
        }
    return(0);
}

void fmb_close(void)
{
    //modbus_flush(modbus);
    //modbus_close(modbus);
    //modbus_free(modbus);
}
