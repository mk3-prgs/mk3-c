#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "uip.h"
#include "uip_arp.h"
#include "enc28j60.h"

#include "xprintf.h"

void hex_dump(uint8_t* bf, int len);

//--------------------------------------------------------------
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
//--------------------------------------------------------------
void uip_log(char *msg)
{
    xprintf("%s\n", msg);
}
//--------------------------------------------------------------
static uint32_t delay_arp = 100;

//void vTask_uIP_periodic(void *pvParameters)
void vTask_uIP_periodic(void)
{
uint32_t i;
//uint8_t delay_arp = 0;
//	for (;;) {
//		vTaskDelay(configTICK_RATE_HZ/4); // полсекунды
		for(i = 0; i < UIP_CONNS; i++) {
			uip_periodic(i);
            uip_conn->nc = i;
			if(uip_len > 0) {
                //for(i=0;i<uip_len;i++) xprintf("%02x", uip_buf[i]);
                //xprintf("\n");
				uip_arp_out();
				enc28j60_send_packet((uint8_t *) uip_buf, uip_len);
				//network_send();
                }
            }

#if UIP_UDP
		for(i = 0; i < UIP_UDP_CONNS; i++) {
			uip_udp_periodic(i);
			if(uip_len > 0) {
				uip_arp_out();
				enc28j60_send_packet((uint8_t *) uip_buf, uip_len);
				//network_send();
			}
		}
#endif /* UIP_UDP */

        delay_arp++;
		if(delay_arp >= 500) { // один раз за 500 проходов цикла, около 5 сек.
			delay_arp = 0;
			uip_arp_timer();
			//xprintf("uip_arp_timer()\n");
            }
//	}
}

extern uint8_t tst_bf[];
extern int tst_st;
extern int tst_ok;

struct uip_eth_addr mac = { { 0x00, 0x01, 0x02, 0x03, 0x04, 0x00 } };

void vTask_uIP(void *pvParameters)
{
TickType_t xLastWakeTime;
const TickType_t xFrequency = 2;
int st=0;
    //
    enc28j60_init(mac.addr);
    //
	uip_init();
	uip_arp_init();
	uip_setethaddr(mac);

	uip_ipaddr_t ipaddr;

	uip_ipaddr(ipaddr, 192, 168, 31, 55);
	uip_sethostaddr(ipaddr);

	uip_ipaddr(ipaddr, 192, 168, 31, 1);
	uip_setdraddr(ipaddr);

	uip_ipaddr(ipaddr, 255, 255, 255, 0);
	uip_setnetmask(ipaddr);
	//
	httpd_init();
	//
    xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		uip_len = enc28j60_recv_packet((uint8_t *) uip_buf, UIP_BUFSIZE);

		if(uip_len > 0) {
			if(BUF->type == htons(UIP_ETHTYPE_IP)) {
				uip_arp_ipin();
				uip_input();
				if (uip_len > 0) {
					uip_arp_out();
					enc28j60_send_packet((uint8_t *) uip_buf, uip_len);
                    }
                }
			else if (BUF->type == htons(UIP_ETHTYPE_ARP)) {
				uip_arp_arpin();
				if (uip_len > 0) {
					enc28j60_send_packet((uint8_t *) uip_buf, uip_len);
                    }
                }
            }
		//taskYIELD();
        //
		if(st <= 0) {
            vTask_uIP_periodic();
            st = 5; //(100 / xFrequency);
            }
        else st--;
        //
        if(tst_st) {
            if((tst_bf[0] == 0x19) && (tst_bf[1] == 0xff));
            else hex_dump(tst_bf, tst_st);
            //
            tst_st = 0;
            }
        //
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
        }
}
