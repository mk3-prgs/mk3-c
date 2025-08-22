#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <termios.h>

#include <sys/ioctl.h>
#include <linux/serial.h>

#define MODBUS_RTU_RS232 1
#define MODBUS_RTU_RS485 0
#define HAVE_DECL_TIOCSRS485 1

#define ACK 0x79
#define NACK 0x1f
//
#include "io.h"

/*
#include "at91gpio.h"
#define _BOOT0  (1 << 12)
AT91S_PIO* gpio;

AT91S_PIO*  gpio_init(unsigned long mask)
{
    gpio = pio_map(AT91_PIOC);
    pio_enable(gpio, mask);
    pio_disable_irq(gpio, mask);
    pio_disable_multiple_driver(gpio, mask);
    pio_enable_pull_ups(gpio, mask);
    pio_synchronous_data_output(gpio, mask);
    pio_output_enable(gpio, mask);
    //pio_input_enable(pio, mask);
    //gpio->PIO_OWER = mask;
	return(gpio);
}
*/

void p_ms(port_t *ctx, int b)
{
struct termios tios;
    //
    tcgetattr(ctx->file, &(tios));
    //
    if (b) {
        /* Mark */
        tios.c_cflag |= CMSPAR;
        tios.c_cflag |= PARENB;
        tios.c_cflag |= PARODD;
        }
    else {
        /* Space */
        tios.c_cflag |= CMSPAR;
        tios.c_cflag |= PARENB;
        tios.c_cflag &=~ PARODD;
        }
    //
    tcsetattr(ctx->file, TCSANOW, &tios);
}

/* Sets up a serial port for RTU communications */
int p_connect(port_t *ctx)
{
    struct termios tios;
    speed_t speed;

    //port_attr_t *ctx = ctx->backend_data;

    if(ctx->debug) {
        printf("Opening %s at %d bauds (%c, %d, %d)\n",
               ctx->device, ctx->baud, ctx->parity,
               ctx->data_bit, ctx->stop_bit);
    }

    /* The O_NOCTTY flag tells UNIX that this program doesn't want
       to be the "controlling terminal" for that port. If you
       don't specify this then any input (such as keyboard abort
       signals and so forth) will affect your process

       Timeouts are ignored in canonical input mode or when the
       NDELAY option is set on the file via open or fcntl */
    ctx->file = open(ctx->device, O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL | O_NONBLOCK);
    //ctx->file = open(ctx->device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (ctx->file == -1) {
        fprintf(stderr, "ERROR Can't open the device %s (%s)\n",
                ctx->device, strerror(errno));
        return -1;
    }

    /* Save */
    tcgetattr(ctx->file, &(ctx->old_tios));

    memset(&tios, 0, sizeof(struct termios));

    /* C_ISPEED     Input baud (new interface)
       C_OSPEED     Output baud (new interface)
    */
    switch (ctx->baud) {
    case 110:
        speed = B110;
        break;
    case 300:
        speed = B300;
        break;
    case 600:
        speed = B600;
        break;
    case 1200:
        speed = B1200;
        break;
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 57600:
        speed = B57600;
        break;
    case 115200:
        speed = B115200;
        break;
    default:
        speed = B9600;
        if (ctx->debug) {
            fprintf(stderr,
                    "WARNING Unknown baud rate %d for %s (B9600 used)\n",
                    ctx->baud, ctx->device);
        }
    }

    /* Set the baud rate */
    if ((cfsetispeed(&tios, speed) < 0) ||
        (cfsetospeed(&tios, speed) < 0)) {
        close(ctx->file);
        ctx->file = -1;
        return -1;
    }

    /* C_CFLAG      Control options
       CLOCAL       Local line - do not change "owner" of port
       CREAD        Enable receiver
    */
    tios.c_cflag |= (CREAD | CLOCAL);

    /* CSIZE, HUPCL, CRTSCTS (hardware flow control) */
    tios.c_cflag &= ~CRTSCTS;

    /* Set data bits (5, 6, 7, 8 bits)
       CSIZE        Bit mask for data bits
    */
    tios.c_cflag &= ~CSIZE;
    switch (ctx->data_bit) {
    case 5:
        tios.c_cflag |= CS5;
        break;
    case 6:
        tios.c_cflag |= CS6;
        break;
    case 7:
        tios.c_cflag |= CS7;
        break;
    case 8:
    default:
        tios.c_cflag |= CS8;
        break;
    }

    /* Stop bit (1 or 2) */
    if (ctx->stop_bit == 1) tios.c_cflag &=~ CSTOPB; // 1
    else tios.c_cflag |= CSTOPB; // 2

    /* CMSPAR
       PARENB       Enable parity bit
       PARODD       Use odd parity instead of even */
    if (ctx->parity == 'N') {
        /* None */
        tios.c_cflag &=~ PARENB;
        }
    else if (ctx->parity == 'E') {
        /* Even */
        tios.c_cflag &= ~CMSPAR;
        tios.c_cflag |= PARENB;
        tios.c_cflag &=~ PARODD;
        }
    else if (ctx->parity == 'O'){
        /* Odd */
        tios.c_cflag &= ~CMSPAR;
        tios.c_cflag |= PARENB;
        tios.c_cflag |= PARODD;
        }
    else if (ctx->parity == 'M'){
        /* Mark */
        tios.c_cflag |= CMSPAR;
        tios.c_cflag |= PARENB;
        tios.c_cflag |= PARODD;
        }
    else if (ctx->parity == 'S'){
        /* Space */
        tios.c_cflag |= CMSPAR;
        tios.c_cflag |= PARENB;
        tios.c_cflag &=~ PARODD;
        }

    /* Read the man page of termios if you need more information. */

    /* This field isn't used on POSIX systems
       tios.c_line = 0;
    */

    /* C_LFLAG      Line options

       ISIG         Enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
       ICANON       Enable canonical input (else raw)
       XCASE        Map uppercase \lowercase (obsolete)
       ECHO         Enable echoing of input characters
       ECHOE        Echo erase character as BS-SP-BS
       ECHOK        Echo NL after kill character
       ECHONL       Echo NL
       NOFLSH       Disable flushing of input buffers after
                        interrupt or quit characters
       IEXTEN       Enable extended functions
       ECHOCTL      Echo control characters as ^char and delete as ~?
       ECHOPRT      Echo erased character as character erased
       ECHOKE       BS-SP-BS entire line on line kill
       FLUSHO       Output being flushed
       PENDIN       Retype pending input at next read or input char
       TOSTOP       Send SIGTTOU for background output

       Canonical input is line-oriented. Input characters are put
       into a buffer which can be edited interactively by the user
       until a CR (carriage return) or LF (line feed) character is
       received.

       Raw input is unprocessed. Input characters are passed
       through exactly as they are received, when they are
       received. Generally you'll deselect the ICANON, ECHO,
       ECHOE, and ISIG options when using raw input
    */

    /* Raw input */
    tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* C_IFLAG      Input options

       Constant     Description
       INPCK        Enable parity check
       IGNPAR       Ignore parity errors
       PARMRK       Mark parity errors
       ISTRIP       Strip parity bits
       IXON         Enable software flow control (outgoing)
       IXOFF        Enable software flow control (incoming)
       IXANY        Allow any character to start flow again
       IGNBRK       Ignore break condition
       BRKINT       Send a SIGINT when a break condition is detected
       INLCR        Map NL to CR
       IGNCR        Ignore CR
       ICRNL        Map CR to NL
       IUCLC        Map uppercase to lowercase
       IMAXBEL      Echo BEL on input line too long
    */
    if (ctx->parity == 'N') {
        /* None */
        tios.c_iflag &= ~INPCK;
    } else {
        tios.c_iflag |= INPCK;
    }

    /* Software flow control is disabled */
    tios.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* C_OFLAG      Output options
       OPOST        Postprocess output (not set = raw output)
       ONLCR        Map NL to CR-NL

       ONCLR ant others needs OPOST to be enabled
    */

    /* Raw ouput */
    tios.c_oflag &=~ OPOST;

    /* C_CC         Control characters
       VMIN         Minimum number of characters to read
       VTIME        Time to wait for data (tenths of seconds)

       UNIX serial interface drivers provide the ability to
       specify character and packet timeouts. Two elements of the
       c_cc array are used for timeouts: VMIN and VTIME. Timeouts
       are ignored in canonical input mode or when the NDELAY
       option is set on the file via open or fcntl.

       VMIN specifies the minimum number of characters to read. If
       it is set to 0, then the VTIME value specifies the time to
       wait for every character read. Note that this does not mean
       that a read call for N bytes will wait for N characters to
       come in. Rather, the timeout will apply to the first
       character and the read call will return the number of
       characters immediately available (up to the number you
       request).

       If VMIN is non-zero, VTIME specifies the time to wait for
       the first character read. If a character is read within the
       time given, any read will block (wait) until all VMIN
       characters are read. That is, once the first character is
       read, the serial interface driver expects to receive an
       entire packet of characters (VMIN bytes total). If no
       character is read within the time allowed, then the call to
       read returns 0. This method allows you to tell the serial
       driver you need exactly N bytes and any read call will
       return 0 or N bytes. However, the timeout only applies to
       the first character read, so if for some reason the driver
       misses one character inside the N byte packet then the read
       call could block forever waiting for additional input
       characters.

       VTIME specifies the amount of time to wait for incoming
       characters in tenths of seconds. If VTIME is set to 0 (the
       default), reads will block (wait) indefinitely unless the
       NDELAY option is set on the port with open or fcntl.
    */
    /* Unused because we use open with the NDELAY option */
    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 0;

    if (tcsetattr(ctx->file, TCSANOW, &tios) < 0) {
        close(ctx->file);
        ctx->file = -1;
        return -1;
    }

    /* The RS232 mode has been set by default */
    //ctx->fileerial_mode = MODBUS_RTU_RS232;
//    gpio = gpio_init(_BOOT0);

    return 0;
}

int p_set_serial_mode(port_t *ctx, int mode)
{
    //if (ctx->backend->backend_type == _MODBUS_BACKEND_TYPE_RTU) {
#if HAVE_DECL_TIOCSRS485
        //port_attr_t *ctx = ctx->backend_data;
        struct serial_rs485 rs485conf;
        memset(&rs485conf, 0x0, sizeof(struct serial_rs485));

        if (mode == MODBUS_RTU_RS485) {
            rs485conf.flags = SER_RS485_ENABLED;
            if (ioctl(ctx->file, TIOCSRS485, &rs485conf) < 0) {
                return -1;
            	}

            ctx->serial_mode |= MODBUS_RTU_RS485;
            return 0;
        } else if (mode == MODBUS_RTU_RS232) {
            if (ioctl(ctx->file, TIOCSRS485, &rs485conf) < 0) {
                return -1;
            	}

            ctx->serial_mode = MODBUS_RTU_RS232;
            return 0;
        }
#else
        if (ctx->debug) {
            fprintf(stderr, "This function isn't supported on your platform\n");
        }
        errno = ENOTSUP;
        return -1;
#endif
    //}

    /* Wrong backend and invalid mode specified */
    errno = EINVAL;
    return -1;
}

int p_get_serial_mode(port_t *ctx) {
    //if (ctx->backend->backend_type == _MODBUS_BACKEND_TYPE_RTU) {
#if HAVE_DECL_TIOCSRS485
        //port_attr_t *ctx = ctx->backend_data;
        return ctx->serial_mode;
#else
        if (ctx->debug) {
            fprintf(stderr, "This function isn't supported on your platform\n");
        }
        errno = ENOTSUP;
        return -1;
#endif
    //} else {
    //   errno = EINVAL;
    //    return -1;
    //}
}

void p_close(port_t *ctx)
{
    /* Closes the file descriptor in RTU mode */
    //port_attr_t *ctx = ctx->backend_data;

    tcsetattr(ctx->file, TCSANOW, &(ctx->old_tios));
    close(ctx->file);
}

int p_flush(port_t *ctx)
{
    return tcflush(ctx->file, TCIOFLUSH);
}

ssize_t p_send(port_t *ctx, const uint8_t *buff, int length)
{
    return write(ctx->file, buff, length);
}

ssize_t p_recv(port_t *ctx, uint8_t *buff, int length)
{
    return (read(ctx->file, buff, length));
}

int p_set_parity(port_t *ctx, char par)
{
    tcdrain(ctx->file);
    p_flush(ctx);
    p_close(ctx);
    //
    //ctx->debug = 0;
    ctx->parity = par;
    return(p_connect(ctx));
}


int p_reset(port_t *ctx)
{
 int flags;
 int ret=0;
 int i;
 uint8_t cmd[8];
 //
//    pio_out(gpio, _BOOT0, 1);
    //
    ioctl(ctx->file, TIOCMGET, &flags);
    //
    flags &= ~TIOCM_RTS; //BOOT0
    //
    flags &=  ~TIOCM_DTR; //RESET
    ioctl(ctx->file, TIOCMSET, &flags);
    usleep(10000);
    flags |=  TIOCM_DTR;
    ioctl(ctx->file, TIOCMSET, &flags);
    usleep(10000);
    //
    do {
        ret = read(ctx->file, cmd, 8); //
        } while(ret);
    //
    cmd[0] = 0x7f;
    //for(i=0;i<1000;i++)
    write(ctx->file, cmd, 1);
    usleep(100000);
    cmd[0] = 0xff;
    ret = read(ctx->file, cmd, 8);

    if(ret>0) {
        printf("Connect = ");
        for(i=0; i<ret; i++) printf("%02x ", cmd[i]);
        printf("\n");
        //printf("\n");
        }
    else printf("ret=%d cmd[0]=%02x\n\n", ret, cmd[0]);

//    pio_out(gpio, _BOOT0, 0);
    //
    return(ret);
}

int p_start(port_t *ctx)
{
int flags;
int ret=0;
//char c;
//uint8_t cmd[8];
    //
//    pio_out(gpio, _BOOT0, 0);
    //
    ioctl(ctx->file, TIOCMGET, &flags);
    //
    flags |=   TIOCM_RTS;
    //
    // Reset
    flags &=  ~TIOCM_DTR; //RESET
    ioctl(ctx->file, TIOCMSET, &flags);
    usleep(100000);
    flags |=  TIOCM_DTR;
    ioctl(ctx->file, TIOCMSET, &flags);
    usleep(10000);
    //
    /*
    printf("Продолжить...:\n");
    while(1) {
        c = getchar();
        if(c == 'q') break;
        //ret = read(ctx->file, cmd, 1);

        //putchar(cmd[0]);
        }
    //
    */
    //system("/usr/bin/picocom -b 9600 -d 8 -p n -f n -l /dev/ttyUSB0");
    return(ret);
}

//===========================================================================//
int  IsHex(char c)
{
	return(((c>='0')&&(c<='9')) ||
	(c=='a') || (c=='b') || (c=='c') || (c=='d') || (c=='e') || (c=='f') ||
	(c=='A') || (c=='B') || (c=='C') || (c=='D') || (c=='E') || (c=='F'));
}

//===========================================================================//
void  byte_copy(char* dst, char* src, int n)
{
char c;
int i;
	for(i=0;i<n;i++) {
		c = (char)src[i];
		dst[i] = c;
		}
	dst[i] = '\0';
}

//===========================================================================//
int  HexToBin(uint8_t* dst, char* src, unsigned int lendst)
{
//:10 0000 00 020CB6120CC28F30E530220210130201 2E
unsigned int kol=0;
unsigned int adr=0;
char str[5]={0};
unsigned int d;
int ret = 0;
unsigned int i;
//
	// п÷я─п╬п╡п╣я─я▐п╣п╪ я└п╬я─п╪п╟я┌
	if((src[0]==':')&&(src[7]=='0')&&(src[8]=='0')) {
		byte_copy(str,&src[1],2);
		// п≤п╥п╡п╩п╣п╨п╟п╣п╪ п╨п╬п╩п╦я┤п╣я│я┌п╡п╬ п╠п╟п╧я┌ п╡ я│я┌я─п╬п╨п╣
		sscanf(str,"%x",&kol);
		if((kol==0)||(kol>0x100)) {
			//s.Format(_T("п■п╩п╦п╫п╟ я│я┌я─п╬п╨п╦ (0x%02x) п╠п╬п╩я▄я┬п╣ 0x20"),kol);
			//AfxMessageBox(s);
			ret = -3;
			goto brk;
			}
        //printf("%02x ", kol);
		//
		byte_copy(str,&src[3],4);
		// п≤п╥п╡п╩п╣п╨п╟п╣п╪ п╟п╢я─п╣я│я│
		sscanf(str,"%x",&adr);
		if(adr > lendst-kol) {
			//s.Format(_T("п░п╢я─п╣я│я│=0x%04x п╡п╫п╣ п╢п╬п©я┐я│я┌п╦п╪п╬пЁп╬: 0x0000-0x%04x"),adr+kol,lendst);
			//AfxMessageBox(s);
			ret = -4;
			goto brk;
			}
        //printf("%04x ", adr);
		//
		for(i=0;i<kol;i++) {
			byte_copy(str,&src[(2*i)+9],2);
			if((adr+i) < lendst) {
				sscanf(str,"%x",&d);
				dst[adr+i] = (uint8_t)d;
				}
			else {
				//s.Format(_T("п░п╢я─п╣я│я│=0x%04x п╥п╟ п©я─п╣п╢п╣п╩п╬п╪ п╠я┐я└п╣я─п╟: 0x%04x"),adr+i,lendst);
				//AfxMessageBox(s);
				ret = -5;
				goto brk;
				}
            //printf("%02x", (uint8_t)d);
			}
        //printf("\n");
		}
	else if((src[0]==':')&&(src[7]=='0')&&(src[8]=='1')) {
	    // п п╬п╫п╣я├ я└п╟п╧п╩п╟
	    ret = 1;
        }
	else if((src[0]==':')&&(src[7]=='0')&&(src[8]=='5')) {
	    // п║я┌п╟я─я┌п╬п╡я▀п╧ п╟п╢я─п╣я│
	    ret = 0;
        }
	else if((src[0]==':')&&(src[7]=='0')&&(src[8]=='4')) {
	    // п═п╟я│я┬п╦я─п╣п╫п╦п╣ п╟п╢я─п╣я│п╟
	    // :02 0000 04 0800 F2
	    byte_copy(str,&src[1],2);
		// п≤п╥п╡п╩п╣п╨п╟п╣п╪ п╨п╬п╩п╦я┤п╣я│я┌п╡п╬ п╠п╟п╧я┌ п╡ я│я┌я─п╬п╨п╣
		sscanf(str,"%x",&kol);
		if((kol==0)||(kol>0x40)) {
			//s.Format(_T("п■п╩п╦п╫п╟ я│я┌я─п╬п╨п╦ (0x%02x) п╠п╬п╩я▄я┬п╣ 0x20"),kol);
			//AfxMessageBox(s);
			ret = -3;
			goto brk;
			}
		//
		byte_copy(str,&src[9],4);
		// п≤п╥п╡п╩п╣п╨п╟п╣п╪ п╟п╢я─п╣я│
		sscanf(str,"%x",&adr);
		//printf("Adres = %04x\n", adr);
	    ret = adr;
        }
	else {
		//s.Format(_T("п²п╣ п╡п╣я─п╫я▀п╧ я└п╬я─п╪п╟я┌ я└п╟п╧п╩п╟:\r\n"));
		//for(int i=0;i<9;i++) s += src[i];
		//AfxMessageBox(s);
		ret = -6;
		}
	//
brk:
	return(ret);
}

void BinToHex(uint8_t n, uint16_t addr, uint8_t *src, char *dst)
{
uint8_t  ks = 0;
int i;
    sprintf(dst, ":%02x%04x00 ", n, (uint16_t)addr); // 9-я┌я▄ п╠п╟п╧я┌ (0...8)
    //
    ks =  (uint8_t)n;
    ks += (uint8_t)(addr>>8);
    ks += (uint8_t)addr;
    //
    for(i=0; i<n; i++) {
        sprintf(&dst[9+2*i], "%02x ", src[i]);
        ks += src[i];
        }
    ks = 0x100 - ks;
    sprintf(&dst[9+2*n], "%02x\n", ks);
    //
    //printf("%s", dst);
    //
}
