#include <hwtimers.h>
#include <time.h>
#include <processor.h>

#include <l4/re/env.h>
#include <l4/sys/kip.h>
#include <l4/util/util.h>

// Definitions

//--------------//
// init_icp_rtc //
//--------------//

#define MICROSECONDS_PER_SECOND 1000000

static l4_kernel_info_t *kip;

static int init_l4_rtc (void) {

  kip = l4re_kip();
  if(!kip) {
    printf("Can't get KIP in %d of file %s.\n", __LINE__, __func__);
    l4_sleep_forever();
  }

  return 0;
}

//--------------//
// read_icp_rtc //
//--------------//

static hwtime_t read_l4_rtc (void) {
  printf("%s\n", __func__);
  return kip->clock;
}

//---------------//
// read_l4_rtc_nsec //
//---------------//

static duration_t read_l4_rtc_nsec (void) {
  printf("%s\n", __func__);
  return read_l4_rtc () * 1000;		//microseconds to nanoseconds
}

//---------------------//
// hwtime2duration_icp_rtc //
//---------------------//

static duration_t hwtime2duration_l4_rtc (hwtime_t hwtime) {
  printf("%s\n", __func__);
  return hwtime * 1000;
}

//------------//
// getres_icp_rtc //
//------------//

static duration_t getres_l4_rtc (void) {
  return 1/MICROSECONDS_PER_SECOND;
}

//-----------------//
// shutdown_l4_rtc //
//-----------------//

static void shutdown_l4_rtc (void) {
}

system_clock_t l4_rtc_clock = {
  init_clock: init_l4_rtc,
  gettime_hwt: read_l4_rtc,
  gettime_nsec: read_l4_rtc_nsec,
  hwtime2duration: hwtime2duration_l4_rtc,
  getres: getres_l4_rtc,
  shutdown_clock: shutdown_l4_rtc,
};

void rtclink(void);
void
rtclink(void){
        init_l4_rtc();
}
