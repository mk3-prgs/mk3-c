#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

float cnv(char *buf)
{
int d;
	sscanf(buf,"%d",&d);
	return(((float)d + 50)/ 1000.0);
}


float therm(void)
{
char buf[256];
FILE *in = NULL;
float ret=-50.0;
//
    in = fopen("/sys/devices/w1_bus_master1/28-000001894829/w1_slave", "r");
    if (in != NULL) {
        //
        fgets(buf, 256, in);
        fgets(buf, 256, in);
        buf[34] = '\0';
        ret = cnv(&buf[29]);
        //printf("T0 = %4.1f\n", ret);
        fclose(in);
        //
        }
	return(ret);
}
