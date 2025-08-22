#include "uip.h"
#include "tftp.h"
#include "xprintf.h"

void tftpc_appcall(void)
{
int i;
struct uip_udp_conn *c;

    xprintf("tftpc_appcall()\n");
    //uip_udp_conn->rport: %d\n", HTONS(uip_udp_conn->rport));

    for(i=0; i<UIP_UDP_CONNS; i++) {
        c = &uip_udp_conns[i];
        xprintf("udp: %d %x\n", i, c->lport);
        }
}

/*---------------------------------------------------------------------------*/
//void tftpc_init(const void *mac_addr, int mac_len)
//{
/*
    s.mac_addr = mac_addr;
    s.mac_len  = mac_len;

    s.state = 0;

    uip_ipaddr(addr, 0,0,0,0);
    s.conn = uip_udp_new(&addr, HTONS(TFTP_SERVER_PORT));

    if(s.conn != NULL) {
        uip_udp_bind(s.conn, HTONS(TFTP_CLIENT_PORT));
        }
*/
/*
    uip_ipaddr(&addr, 192,168,31,1);
    c = uip_udp_new(&addr, HTONS(69));
    //
    if(c != NULL) {
        uip_udp_bind(c, HTONS(7777));
        }
*/
    //uip_send("123456", 6);
//}
