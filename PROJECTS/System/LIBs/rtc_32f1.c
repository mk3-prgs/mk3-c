#include "main.h"
#include "rtc_32f1.h"

#define F_LSE	32768	/* LSE oscillator frequency */

static
const uint8_t samurai[] = {31,28,31,30,31,30,31,31,30,31,30,31};

uint16_t set_rtc[8];

/*------------------------------------------*/
/* Get time in calendar form                */
/*------------------------------------------*/
/*
    rtc.mday = 4;
    rtc.month = 11;
    rtc.year = 2014;
    rtc.hour = 2;
    rtc.min = 59;
    rtc.sec = 0;
    //rtc_settime(&rtc);
*/


volatile uint32_t utc_time;

void rtc_sync(void)
{
    rtc_register_sync_wait();
    rtc_interrupt_enable(RTC_INT_SECOND);
    utc_time = rtc_counter_get();
}

void RTC_IRQHandler(void)
{
    if(rtc_flag_get(RTC_INT_FLAG_SECOND) != RESET) {
        rtc_flag_clear(RTC_INT_FLAG_SECOND);
        utc_time = rtc_counter_get();
        }
}

uint32_t g_time(uint32_t* t)
 {
     if(t != NULL) *t = utc_time;
     return(utc_time);
 }

void rtc_configuration(rcu_osci_type_enum s_sin)
{
uint32_t p_div=32768;
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    //
    //allow access to BKP domain
    pmu_backup_write_enable();
    // reset backup domain
    bkp_deinit();
    //
    //
    if(s_sin == RCU_LXTAL) {
        /* enable LXTAL */
        rcu_osci_on(RCU_LXTAL);
        /* wait till LXTAL is ready */
        rcu_osci_stab_wait(RCU_LXTAL);
        /* select RCU_LXTAL as RTC clock source */
        rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
        //
        p_div=32768;
        }
    else if(s_sin == RCU_IRC40K) {
        rcu_osci_on(RCU_IRC40K);
        rcu_osci_stab_wait(RCU_IRC40K);
        rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);
        p_div=40000;
        }
    else if(s_sin == RCU_HXTAL) {
        rcu_osci_on(RCU_HXTAL);
        rcu_osci_stab_wait(RCU_HXTAL);
        rcu_rtc_clock_config(RCU_RTCSRC_HXTAL_DIV_128);
        p_div=8000000/128;
        }

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_SECOND);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set RTC prescaler: set RTC period to 1s */
    rtc_prescaler_set(p_div);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

int initRTC(RTCTIME* prtc, rcu_osci_type_enum s_sin)
{
    //
    if(prtc != NULL) {
        rtc_configuration(s_sin);
        rtc_settime(prtc);
        }
    else {
        /* enable PMU and BKPI clocks */
        rcu_periph_clock_enable(RCU_BKPI);
        rcu_periph_clock_enable(RCU_PMU);
        /* allow access to BKP domain */
        pmu_backup_write_enable();
        //
        }
    rtc_register_sync_wait();
    rtc_interrupt_enable(RTC_INT_SECOND);
    rtc_lwoff_wait();
    //
    utc_time = rtc_counter_get();
    //
    return(0);
}

int rtc_gettime (RTCTIME* p_rtc)
{
	uint32_t utc, n, i, d;

    utc = utc_time;
    //
	utc += (long)(_RTC_TDIF * 3600);

	p_rtc->sec = (uint8_t)(utc % 60); utc /= 60;
	p_rtc->min = (uint8_t)(utc % 60); utc /= 60;
	p_rtc->hour = (uint8_t)(utc % 24); utc /= 24;
	p_rtc->wday = (uint8_t)((utc + 4) % 7);
	p_rtc->year = (uint16_t)(1970 + utc / 1461 * 4); utc %= 1461;
	n = ((utc >= 1096) ? utc - 1 : utc) / 365;
	p_rtc->year += n;
	utc -= n * 365 + (n > 2 ? 1 : 0);
	for (i = 0; i < 12; i++) {
		d = samurai[i];
		if (i == 1 && n == 2) d++;
		if (utc < d) break;
		utc -= d;
	}
	p_rtc->month = (uint8_t)(1 + i);
	p_rtc->mday = (uint8_t)(1 + utc);

	return(1);
}

/*------------------------------------------*/
/* Set time in calendar form                */
/*------------------------------------------*/

int rtc_settime(const RTCTIME* p_rtc)
{
	uint32_t utc, i, y;

	y = p_rtc->year - 1970;
	if (y > 2106 || !p_rtc->month || !p_rtc->mday) return 0;

	utc = y / 4 * 1461; y %= 4;
	utc += y * 365 + (y > 2 ? 1 : 0);
	for (i = 0; i < 12 && i + 1 < p_rtc->month; i++) {
		utc += samurai[i];
		if (i == 1 && y == 2) utc++;
	}
	utc += p_rtc->mday - 1;
	utc *= 86400;
	utc += p_rtc->hour * 3600 + p_rtc->min * 60 + p_rtc->sec;

	utc -= (long)(_RTC_TDIF * 3600);
    //
    pmu_backup_write_enable();
    //
    rtc_lwoff_wait();
    rtc_configuration_mode_enter();
    rtc_counter_set(utc);
    rtc_configuration_mode_exit();
    rtc_lwoff_wait();
    //
    utc_time = utc;
    //
	return(1);
}

uint32_t tm_to_time(const RTCTIME* p_rtc)
{
	uint32_t utc, i, y;

	y = p_rtc->year - 1970;
	if(y > 2106 || !p_rtc->month || !p_rtc->mday) return 0;

	utc = y / 4 * 1461; y %= 4;
	utc += y * 365 + (y > 2 ? 1 : 0);
	for (i = 0; i < 12 && i + 1 < p_rtc->month; i++) {
		utc += samurai[i];
		if (i == 1 && y == 2) utc++;
	}
	utc += p_rtc->mday - 1;
	utc *= 86400;
	utc += p_rtc->hour * 3600 + p_rtc->min * 60 + p_rtc->sec;
    //
	return(utc);
}

int time_to_tm(uint32_t utc, RTCTIME* p_rtc)
{
	uint32_t n, i, d;

	p_rtc->sec = (uint8_t)(utc % 60); utc /= 60;
	p_rtc->min = (uint8_t)(utc % 60); utc /= 60;
	p_rtc->hour = (uint8_t)(utc % 24); utc /= 24;
	p_rtc->wday = (uint8_t)((utc + 4) % 7);
	p_rtc->year = (uint16_t)(1970 + utc / 1461 * 4); utc %= 1461;
	n = ((utc >= 1096) ? utc - 1 : utc) / 365;
	p_rtc->year += n;
	utc -= n * 365 + (n > 2 ? 1 : 0);
	for (i = 0; i < 12; i++) {
		d = samurai[i];
		if (i == 1 && n == 2) d++;
		if (utc < d) break;
		utc -= d;
	}
	p_rtc->month = (uint8_t)(1 + i);
	p_rtc->mday = (uint8_t)(1 + utc);

	return(1);
}

void s_time(uint32_t u)
{
    utc_time = u;
    ///RTC_SetCounter(utc);
    rtc_lwoff_wait();
    rtc_counter_set(u);
    rtc_lwoff_wait();
}

uint32_t get_fattime(void)
{
	RTCTIME rtc;
	// Get local time
	if (!rtc_gettime(&rtc)) return 0;
//
	// Pack date and time into a DWORD variable
	return	  ((uint32_t)(rtc.year - 1980) << 25)
			| ((uint32_t)rtc.month << 21)
			| ((uint32_t)rtc.mday << 16)
			| ((uint32_t)rtc.hour << 11)
			| ((uint32_t)rtc.min << 5)
			| ((uint32_t)rtc.sec >> 1);

    return(0);
}
