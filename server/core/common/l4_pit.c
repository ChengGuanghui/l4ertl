#include <l4/sys/thread.h>
#include <l4/re/env.h>
#include <stdio.h>
#include <stddef.h>

#include <hwtimers.h>
#include <irqs.h>
#include <l4/irqs.h>
#include <time.h>
#include <processor.h>

#define PERIODIC_INTERVAL_MAX 0xFFFFFFFF
#define PERIODIC_INTERVAL_MIN 2			//microseconds
#define PERIODIC_INTERVAL_MSEC 1000		//microseconds
#define L4_TIMER0_IRQ 2000

// Definitions
static timer_handler_t l4_timer_user_handler = NULL;

//static duration_t clock_ctime = 0;

//-----------------------//
// l4_timer_irq_handler //
//-----------------------//

static int l4_timer_irq_handler (context_t *context) {

#if 0
  io_write (PIT_TIMERXINTCLR(CNT0), 0x1);

  // Another fix for QEMU
  // Cleaning Timer INT does not cleans the PIC interrupt
  // To work around the problem, we program a timer count without
  // interrupts, that will clear the PIC
  io_write (PIT_TIMERXCTRL(CNT0), 0x83);
  io_write (PIT_BASE_ADDR, 0x1);

  while (io_read(PIT_BASE_ADDR + 0x4));
  ////////////////////////////////////////


  // Timer set in periodic mode
  io_write (PIT_TIMERXCTRL(CNT0), 0xe2);
  //io_write (PIT_TIMERXLD(CNT0),
  //(PERIODIC_INTERVAL_NSEC * bus_hz)/ NSECS_PER_SEC);
  io_write (PIT_TIMERXLD(CNT0), 0x2625a00);
#endif
  if (l4_timer_user_handler)
    (*l4_timer_user_handler) ();

  return 0;
}

//------------------------//
// init_hwtimer_l4_timer //
//------------------------//

static int init_hwtimer_l4_timer (void) {
  install_irq_handler_sys (L4_TIMER_IRQ, l4_timer_irq_handler);
  //hw_enable_irq(L4_TIMER_IRQ);
  return 0;
}

//-----------------------//
// set_hwtimer_l4_timer //
//-----------------------//

static void set_hwtimer_l4_timer (duration_t interval) {
  int flags, error;
  //unsigned long cntr = (interval * bus_hz)/NSECS_PER_SEC;

  hw_save_flags_and_cli (flags);
  // ENABLE | FREE_RUNNING | IE | NO_DIVISOR | 32b | ONESHOT
  l4_msg_regs_t *mr = l4_utcb_mr();
  l4_msgtag_t tag;

  //io_write (PIT_TIMERXCTRL(CNT0), 0xa3);
  //io_write (PIT_TIMERXLD(CNT0), cntr);
#ifdef ARCH_arm
  mr->mr[0] = interval;
  tag = l4_msgtag(0, 1, 0, 0);
#else
  mr->mr[0] = 0xFFFFFFFF & interval;
  mr->mr[1] = interval >> 32;
  tag = l4_msgtag(0, 2, 0, 0);
#endif

  if((error = l4_msgtag_has_error(l4_ipc_send(irq_thread_caps[L4_TIMER_IRQ], l4_utcb(), tag, L4_IPC_NEVER))) > 0)
    printf("%s: Timer setting failed with %s\n", __func__, l4sys_errtostr(error));

  l4_thread_yield();

  hw_restore_flags (flags);
}

//----------------------------//
// get_max_interval_l4_timer //
//----------------------------//

static duration_t get_max_interval_l4_timer (void) {
  return PERIODIC_INTERVAL_MAX; /*(ICP_TIMER_ACCURATELY * NSECS_PER_SEC/ICP_TIMER_HZ);*/
}

//----------------------------//
// get_min_interval_l4_timer //
//----------------------------//

static duration_t get_min_interval_l4_timer (void) {
  return PERIODIC_INTERVAL_MIN; // 20 milli seconds
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
  hw_disable_irq(L4_TIMER_IRQ); 
  install_irq_handler_sys (L4_TIMER_IRQ, NULL);
}

hwtimer_t l4_timer = {
  init_hwtimer: init_hwtimer_l4_timer,
  set_hwtimer: set_hwtimer_l4_timer,
  get_max_interval: get_max_interval_l4_timer,
  get_min_interval: get_min_interval_l4_timer,
  set_timer_handler: set_timer_handler_l4_timer,
  shutdown_hwtimer: shutdown_hwtimer_l4_timer,
};

static l4_kernel_info_t *kip;

//----------------//
// init_l4_timer //
//----------------//

static int init_l4_timer (void) {
  kip = l4re_kip();
  return 0;
}

//----------------//
// read_l4_timer //
//----------------//

static hwtime_t read_l4_timer (void) {
  return kip->clock;
}

//---------------------//
// read_l4_timer_nsec //
//---------------------//

//big endian or little endian, microseconds not nanoseconds
static duration_t read_l4_timer_nsec (void) {
  return kip->clock;
}

//---------------------------//
// hwtime2duration_l4_timer //
//---------------------------//

static duration_t hwtime2duration_l4_timer (hwtime_t hwtime) {
  return hwtime;
}

//------------------//
// getres_l4_timer //
//------------------//

static duration_t getres_l4_timer (void) {
  return PERIODIC_INTERVAL_MSEC; // 1 usec
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
  init_l4_timer();
  init_hwtimer_l4_timer();
}
