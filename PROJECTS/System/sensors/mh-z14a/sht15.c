#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

float hum_sht15(void)
{
char buf[256];
FILE *in = NULL;
float ret=-50.0;
//
    in = fopen("/sys/devices/platform/sht15/humidity1_input", "r");
    if (in != NULL) {
        //
        fgets(buf, 256, in);
        ret = (float)atoi(buf);
        //
        fclose(in);
        //
        }
	return(ret);
}

float themp_sht15(void)
{
char buf[256];
FILE *in = NULL;
float ret=-50.0;
//
    in = fopen("/sys/devices/platform/sht15/temp1_input", "r");
    if (in != NULL) {
        //
        fgets(buf, 256, in);
        ret = (float)atoi(buf);
        //
        fclose(in);
        //
        }
	return(ret);
}
