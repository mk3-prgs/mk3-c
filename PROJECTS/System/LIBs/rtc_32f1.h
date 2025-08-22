#ifndef _RTC_DEFINED
#define _RTC_DEFINED

#include <gd32f10x.h>
//#define _RTC_TDIF	3	/* JST = UTC+3.0 */
#define _RTC_TDIF	0	/* JST = UTC+0.0 */

//typedef uint32_t time_t;

typedef struct {
	uint16_t	year;	/* 1970..2106 */
	uint8_t		month;	/* 1..12 */
	uint8_t		mday;	/* 1..31 */
	uint8_t		hour;	/* 0..23 */
	uint8_t		min;	/* 0..59 */
	uint8_t		sec;	/* 0..59 */
	uint8_t		wday;	/* 0..6 (Sun..Sat) */
} RTCTIME;
//
int rtc_gettime(RTCTIME* rtc);			/* Get time */
int rtc_settime (const RTCTIME* rtc);	/* Set time */
//
void rtc_configuration(rcu_osci_type_enum s_sin);
void rtc_sync(void);
int initRTC(RTCTIME* prtc, rcu_osci_type_enum s_sin);
//
uint32_t g_time(uint32_t* t);
void set_time(RTCTIME* rtc);
void time_update(RTCTIME* rtc);
//
int time_to_tm(uint32_t utc, RTCTIME* p_rtc);
uint32_t tm_to_time(const RTCTIME* p_rtc);
//
void s_time(uint32_t u);
//
#endif
