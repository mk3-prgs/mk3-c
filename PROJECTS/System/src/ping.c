#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#include "uip.h"
#include "uip_arp.h"
#include "stm32f10x.h"
#include "enc28j60.h"

#include "xprintf.h"

#define ICMPBUF ((struct uip_icmpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define ICMP_ECHO       8

volatile uint32_t rsp_time;

void print_ipaddr(uip_ipaddr_t addr)
{
    //
    uint16_t a0 = HTONS(addr[0]);
    uint16_t a1 = HTONS(addr[1]);
    //
    xprintf("%d.%d.%d.%d", 0xff & (a0>>8), 0xff & a0, 0xff & (a1>>8), 0xff & a1);
}

void ping(char *s)
{
int i;
int err=0;
uip_ipaddr_t src_a;
uip_ipaddr_t dst_a;

    if(uip_len != 0) {
        xprintf("uip_buff BUSY!\n");
        return;
        }

    for(i=0; i<4; i++) {
        //
        // Составляем IP заголовок
        ICMPBUF->vhl = 0x45;
        ICMPBUF->tos = 0;
        uint16_t    len = sizeof(struct uip_icmpip_hdr);    // длина будет равна заголовку ip + icmp
        ICMPBUF->len[0] = len >> 8;
        ICMPBUF->len[1] = len & 0xff;
        uint16_t    ipid = 0;
        ICMPBUF->ipid[0] = ipid >> 8;
        ICMPBUF->ipid[1] = ipid & 0xff;
        ICMPBUF->ipoffset[0] = ICMPBUF->ipoffset[1] = 0;
        ICMPBUF->ttl = UIP_TTL;
        ICMPBUF->proto = UIP_PROTO_ICMP;
        // указываем свой ip и ip удаленного устройства.
        uip_gethostaddr(ICMPBUF->srcipaddr);
        uip_ipaddr(&ICMPBUF->destipaddr, 192,168,31,1);

        ICMPBUF->ipchksum = 0;
        ICMPBUF->ipchksum = ~(uip_ipchksum());

        ICMPBUF->type = ICMP_ECHO;
        ICMPBUF->icode = 0;
        ICMPBUF->icmpchksum = 0;
        ICMPBUF->icmpchksum = uip_chksum((uint16_t*)&ICMPBUF->type, 8);
        ICMPBUF->icmpchksum = ~((ICMPBUF->icmpchksum == 0) ? 0xffff : ICMPBUF->icmpchksum);

        //xprintf("type %d, code %d, icmpchksum %d\r\n", ICMPBUF->type, ICMPBUF->icode, ICMPBUF->icmpchksum);

        uip_len = UIP_IPH_LEN + 8;    // длина равна заголовку ip + заголовку icmp
        //
        uip_gethostaddr(src_a);
        uip_ipaddr_copy(dst_a, ICMPBUF->destipaddr);
        //
        //int w=3;
        //
    //loop:;
        rsp_time = xTaskGetTickCount();
        uint32_t t = rsp_time;
        //
        uip_arp_out();

        enc28j60_send_packet((uint8_t *)uip_buf, uip_len);
        //
        while(rsp_time == t) {
            if((xTaskGetTickCount() - t) > 1000) {
                err = 1;
                break;
                }
            vTaskDelay(10);
            }
        //if(w) {w--; goto loop;}
        //
        if(err) { xprintf("Error ping!\n"); break; }
        //
        xprintf("From: "); print_ipaddr(src_a);
        xprintf(" To: ");  print_ipaddr(dst_a);
        xprintf(" Response time: %d.0\n", rsp_time - t);
        //
        vTaskDelay(1000);
        }
        //
        vTaskDelete(NULL);
}

