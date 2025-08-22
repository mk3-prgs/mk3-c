#ifndef __TFTP_H__
#define __TFTP_H__

#define TFTP_SERVER_PORT 69
#define TFTP_CLIENT_PORT 69

#define STATE_INITIAL         0
#define STATE_SENDING         1
#define STATE_OFFER_RECEIVED  2
#define STATE_CONFIG_RECEIVED 3
#define STATE_CLOSE 4
#define STATE_ACK 5

enum packet_type {RRQ=1, WRQ, DATA, ACK, ERR};

enum filemode {NETASCII, OCTET, MAIL};
/*
static char *filemode[] = {
    "netascii",
    "octet",
    "mail"
};
*/

enum error_codes {NOTDEFERR, NOTFOUNDERR, ACCESSERR, DISKFULLERR,
		  ILLEGALOPERR, UNKNOWNIDERR, FILEEXISTSERR,
		  UNKNOWNUSERERR};
/*
static char *errormsg[] = {
    "Not defined, see error message",
    "File not found",
    "Access violation",
    "Disk full or allocation exceeded",
    "Illegal TFTP operation",
    "Unknown transfer ID",
    "File already exists",
    "No such user"
};
*/
typedef
    struct generic_packet {
         short unsigned opcode;
         char info[514];
     } generic_packet;

typedef
     struct data_packet {
         short unsigned opcode;  /* 3 */
         short unsigned block_number;
         char data[512];
     } data_packet;

typedef
     struct ack_packet {
         short unsigned opcode;  /* 4 */
         short unsigned block_number;
     } ack_packet;

typedef
     struct error_packet {
         short unsigned opcode;  /* 5 */
         short unsigned error_code;
         char error_msg[512];
     } error_packet;


struct tftpc_state {
    char state;
    struct uip_udp_conn *conn;
//    uint8_t buff[256];
};

void tftpc_init(const void *mac_addr, int mac_len);
void tftpc_appcall(void);

typedef struct tftpc_state uip_udp_appstate_t;

#endif // __TFTP_H__
