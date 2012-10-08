#include <l4/sys/kdebug.h>
#include <l4/sys/debugger.h>
#include <l4/sys/thread.h>
#include <l4/re/env.h>
#include <l4/io/io.h>
#include <stdio.h>
#include <stddef.h>

#include <hwtimers.h>
#include <irqs.h>
#include <l4/irqs.h>
#include <time.h>
#include <processor.h>

#define VERSATILE_TIMER_RELOAD 		(0x80000000UL)

#define VERSATILE_TIMER_ENABLE          (1UL << 7) /* Timer Enable */
#define VERSATILE_TIMER_PERIODIC        (1UL << 6) /* Timer Mode */
#define VERSATILE_TIMER_IE              (1UL << 5) /* Timer INT Enable */
#define VERSATILE_TIMER_32BIT           (1UL << 1) /* 32 bit counter */
#define VERSATILE_TIMER_ONESHOT         (1UL << 0) /* One shot mode */
#define VERSATILE_TIMER_DIV1            (0UL << 2) /* Divided by 1 */
#define VERSATILE_TIMER_DIV16           (1UL << 2) /* Divided by 16 */
#define VERSATILE_TIMER_DIV256          (2UL << 2) /* Divided by 256 */

#define REALVIEW_TIMER23_IRQ 	34
#define TIMER2_PHYS_ADDR	0x10011000
#define TIMER_TICK_FREQ		1000000ULL

#define PERIODIC_INTERVAL_MAX	0xFFFFFFFFUL
#define PERIODIC_INTERVAL_MIN	1000		//1 microseconds

// Definitions
static timer_handler_t l4_timer_user_handler = NULL;

static unsigned long current_time_lo, current_time_hi;

typedef struct Timer {
  unsigned long load;
  unsigned long val;
  unsigned long ctrl;
  unsigned long clear;
  unsigned long raw_irq_status;
  unsigned long irq_status;
  unsigned long bgload;
} Timer_t;

volatile Timer_t *timer2;
volatile Timer_t *timer3;

//static duration_t clock_ctime = 0;

//-----------------------//
// l4_timer_irq_handler //
//-----------------------//

static int l4_timer_irq_handler (context_t *context) {

  printf("timer2 %d\n", timer2->irq_status);
  printf("timer3 %d\n", timer3->irq_status);

  if (l4_timer_user_handler)
    (*l4_timer_user_handler) ();

  return 0;
}

//------------------------//
// init_hwtimer_l4_timer //
//------------------------//

static int init_hwtimer_l4_timer (void) {
  int ret;
  l4_addr_t timer2_virt_addr = 0;

  ret = l4io_request_iomem(TIMER2_PHYS_ADDR, 0x1000, L4IO_MEM_NONCACHED, &timer2_virt_addr);
  if (ret)
  {
    printf("[Realview Timer2] Error: Could not map device memory\n");
    return 1;
  }

  timer3 = (volatile Timer_t *)(timer2_virt_addr + 0x20);

  /* One-Shot Mode */
  timer3->clear = 1;
  timer3->ctrl &= (~VERSATILE_TIMER_ENABLE);		//disable
  timer3->ctrl = VERSATILE_TIMER_IE | VERSATILE_TIMER_32BIT | VERSATILE_TIMER_ONESHOT;
  timer3->clear = ~0UL;

  //install_irq_handler_sys (REALVIEW_TIMER23_IRQ, l4_timer_irq_handler);

  //hw_enable_irq(REALVIEW_TIMER23_IRQ);
  return 0;
}

//-----------------------//
// set_hwtimer_l4_timer //
//-----------------------//
static void set_hwtimer_l4_timer (duration_t interval) {
  int flags, error;
  unsigned long counter = interval / 1000;		//counter = interval / NSECS_PER_SEC * TIMER_TICK_FREQ

  hw_save_flags_and_cli (flags);

  /* One-shot mode */
  //printf("interal 0x%lld\n", interval);
  timer3->load = counter;
  timer3->ctrl |= VERSATILE_TIMER_ENABLE;

  hw_restore_flags (flags);
  //while(1)printf("raw_irq_status %d  irq_status %d\n", timer3->raw_irq_status, timer3->irq_status);
}

//----------------------------//
// get_max_interval_l4_timer //
//----------------------------//

static duration_t get_max_interval_l4_timer (void) {
  return PERIODIC_INTERVAL_MAX;
}

//----------------------------//
// get_min_interval_l4_timer //
//----------------------------//

static duration_t get_min_interval_l4_timer (void) {
  return PERIODIC_INTERVAL_MIN;
}

//-----------------------------//
// set_timer_handler_l4_timer //
//-----------------------------//

static timer_handler_t set_timer_handler_l4_timer (timer_handler_t 
						    timer_handler) {
  unsigned int flags;
  timer_handler_t old_l4_timer_user_handler = l4_timer_user_handler;

  hw_save_flags_and_cli (flags);
  l4_timer_user_handler = timer_handler;
  hw_restore_flags (flags);

  return old_l4_timer_user_handler;
}

//----------------------------//
// shutdown_hwtimer_l4_timer //
//----------------------------//

static void shutdown_hwtimer_l4_timer (void) {
  hw_disable_irq(REALVIEW_TIMER23_IRQ);
  install_irq_handler_sys (REALVIEW_TIMER23_IRQ, NULL);
}

hwtimer_t l4_timer = {
  init_hwtimer: init_hwtimer_l4_timer,
  set_hwtimer: set_hwtimer_l4_timer,
  get_max_interval: get_max_interval_l4_timer,
  get_min_interval: get_min_interval_l4_timer,
  set_timer_handler: set_timer_handler_l4_timer,
  shutdown_hwtimer: shutdown_hwtimer_l4_timer,
};

//----------------//
// init_l4_timer //
//----------------//

static int init_l4_timer (void) {
  int ret;
  l4_addr_t timer2_virt_addr = 0;

  ret = l4io_request_iomem(TIMER2_PHYS_ADDR, 0x1000, L4IO_MEM_NONCACHED, &timer2_virt_addr);
  if (ret)
  {
    printf("[Realview Timer2] Error: Could not map device memory\n");
    return 1;
  }

  timer2 = (volatile Timer_t *)timer2_virt_addr;

  /* Periodic mode */
  timer2->clear = 1;
  timer2->ctrl &= ~(VERSATILE_TIMER_ENABLE);
  timer2->ctrl = VERSATILE_TIMER_PERIODIC | VERSATILE_TIMER_IE | VERSATILE_TIMER_32BIT;
  timer2->load = VERSATILE_TIMER_RELOAD;
  timer2->ctrl |= VERSATILE_TIMER_ENABLE;

  current_time_hi = 0;
  current_time_lo = 0;

  //while(1)printf("init timer for time keeping 0x%x.\n", timer2->val);

  //install_irq_handler_sys (REALVIEW_TIMER23_IRQ, l4_timer_irq_handler);

  //hw_enable_irq(REALVIEW_TIMER23_IRQ);

  return 0;
}

//----------------//
// read_l4_timer //
//----------------//

static hwtime_t read_l4_timer (void) {
  unsigned long time_lo = ~(timer2->val);

  if (time_lo < current_time_lo) {
     /* The 32-bit counter has wrapped around. */
     current_time_hi++;
  }
  current_time_lo = time_lo;

  /*
   * The versatile kernel timer runs at 1MHz so no work is required here
   * to convert ticks into microseconds.
   */
  return ((unsigned long long)current_time_hi << 32) | ((unsigned long long)current_time_lo);
}

//---------------------//
// read_l4_timer_nsec //
//---------------------//

static duration_t read_l4_timer_nsec (void) {
  return read_l4_timer() * 1000;		//read_l4_timer() / TIMER_TICK_FREQ * NSECS_PER_SEC
}

//---------------------------//
// hwtime2duration_l4_timer //
//---------------------------//

static duration_t hwtime2duration_l4_timer (hwtime_t hwtime) {
  return hwtime * 1000;			// hwtime / TIMER_TICK_FREQ * NSES_PER_SEC
}

//------------------//
// getres_l4_timer //
//------------------//

static duration_t getres_l4_timer (void) {
  return PERIODIC_INTERVAL_MIN; // 1 usec
}

//--------------------//
// shutdown_l4_timer //
//--------------------//

static void shutdown_l4_timer (void) {
}

system_clock_t l4_timer_clock = {
  init_clock: init_l4_timer,
  gettime_hwt: read_l4_timer,
  gettime_nsec: read_l4_timer_nsec,
  hwtime2duration: hwtime2duration_l4_timer,
  getres: getres_l4_timer,
  shutdown_clock: shutdown_l4_timer,
};

//REGISTER_DRV(init_hwtimer_icp_timer, 0, "INTEGRATORCP PIT");
//REGISTER_DRV(init_icp_timer, 0, "INTEGRATORCP CLOCK");

void pitlink(void);
void
pitlink(void){
  printf("pit link.\n");
  if (init_l4_timer() || init_hwtimer_l4_timer()) {
    printf("Init timer failed.\n");
    enter_kdebug("Timer Init Failed.\n");
  }

  install_irq_handler_sys (REALVIEW_TIMER23_IRQ, l4_timer_irq_handler);

  hw_enable_irq(REALVIEW_TIMER23_IRQ);
}
