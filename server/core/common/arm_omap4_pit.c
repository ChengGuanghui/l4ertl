#include <l4/sys/kdebug.h>
#include <l4/sys/scheduler.h>
#include <l4/sys/debugger.h>
#include <l4/sys/factory.h>
#include <l4/sys/thread.h>
#include <l4/sys/irq.h>
#include <l4/sys/ipc.h>
#include <l4/re/env.h>
#include <l4/re/c/util/cap.h>
#include <l4/io/io.h>
#include <stdio.h>
#include <stddef.h>

#include <hwtimers.h>
#include <irqs.h>
#include <l4/irqs.h>
#include <time.h>
#include <processor.h>

#define TIMER_TICK_FREQ			38400000ULL

#define GPTIMER1_PHYS_ADDR		0x4a318000
#define GPTIMER2_PHYS_ADDR		0x48032000
#define GPTIMER10_PHYS_ADDR		0x48086000

#define WKUPCM_PHYS_ADDR                0x4a306000
#define WKUPCM_SIZE                     0x2000
#define CM_WKUP_GPTIMER1_CLKCTRL        0x4a307840

#define CM2_PHYS_ADDR                   0x4a008000
#define CM2_SIZE                        0x2000
#define CM_L4PER_GPTIMER2_CLKCTRL       0x4a009438
#define CM_L4PER_GPTIMER10_CLKCTRL      0x4a009428

#define SYS_CLKSEL_PHYS_ADDR            0x4a306110

#define GPTIMER1_IRQ			69
#define GPTIMER2_IRQ			70
#define GPTIMER10_IRQ			78

/*
 * TISR - shows which interrupt events are pending.
 * (pages 2464-2465)
 */
#define TISR_MATCH (1UL)
#define TISR_OVF (2UL)

/*
 * TIER - controls (enable/disable) the interrupt events.
 * (pages 2465-2466)
 */
#define TIER_MATCH (1UL)
#define TIER_OVF (2UL)

/*
 * TCLR - controls the features of the timer.
 * (pages 2467-2468)
 */
/* ST - starts (1) or stops (0) the timer. */
#define TCLR_ST (1UL)
/* AR - autoreload the counter on overflow. */
#define TCLR_AR (2UL)
/* CE - compare enable. */
#define TCLR_CE (0x40UL)

#define TCLR_SETUP (TCLR_ST | TCLR_AR | TCLR_CE)

/* slection bit */
#define SYS_CLK_BIT (24)
/* enable/disable bit */
#define MODULE_ENABLE_VALUE (0x2)
/* status bit */
#define IDLEST_START_BIT 16
/* status mask */
#define IDLEST_MASK 0x3

// Definitions
static timer_handler_t l4_timer_user_handler = NULL;

static volatile unsigned long current_time_hi;

typedef struct omap4_gpt {
    /* 0x0 */
    l4_umword_t tidr[4];
    /* 0x10 */
    l4_umword_t tiocp_cfg;
    l4_umword_t tistat;
    l4_umword_t tisr;
    l4_umword_t tier;
    /* 0x20 */
    l4_umword_t twer;
    l4_umword_t tclr;
    l4_umword_t tcrr;
    l4_umword_t tldr;
    /* 0x30 */
    l4_umword_t ttgr;
    l4_umword_t twps;
    l4_umword_t tmar;
    l4_umword_t tcar1;
    /* 0x40 */
    l4_umword_t tsicr;
    l4_umword_t tcar2;
    /* The following registers are only present on GPT1, 2 and 10. */
    l4_umword_t tpir;
    l4_umword_t tnir;
    /* 0x50 */
    l4_umword_t tcvr;
    l4_umword_t tocr;
    l4_umword_t towr;
}Timer_t;


volatile Timer_t *timer2_reg;
volatile Timer_t *timer10_reg;

static l4_umword_t init_ticks = 0;

//static duration_t clock_ctime = 0;

//---------------------------------------//
//          set_sysclk_freq             //
//--------------------------------------//
static int set_sysclk_freq(void) {
  int ret;
  l4_addr_t l4wkup_virt_addr = 0;

  volatile unsigned int *sys_clksel_virt_addr;

  ret =l4io_request_iomem(WKUPCM_PHYS_ADDR, WKUPCM_SIZE, L4IO_MEM_NONCACHED, &l4wkup_virt_addr);
  if (ret)
  {
    printf("[L4WKUP] Error: Could not map device memory\n");
    return 1;
  }

  sys_clksel_virt_addr = (volatile unsigned int *)(l4wkup_virt_addr + SYS_CLKSEL_PHYS_ADDR - WKUPCM_PHYS_ADDR);

  *sys_clksel_virt_addr = 0x7;          //38.4MHZ
}

//-----------------------//
// hwtimer_irq_handler //
//-----------------------//

static int hwtimer_irq_handler (context_t *context) {

  /* Disable timer2 */
  timer2_reg->tier = 0;

  /* Clear interrupts */
  timer2_reg->tisr = TISR_OVF | TISR_MATCH;

  if (l4_timer_user_handler)
    (*l4_timer_user_handler) ();

  return 0;
}

//------------------------//
// init_hwtimer_l4_timer //
//------------------------//

static int init_hwtimer_l4_timer (void) {

  l4_addr_t l4wkup_virt_addr = 0;
  l4_addr_t cm2_virt_addr = 0;

  volatile unsigned int *gptimer2_clkctrl;

  if (l4io_request_iomem(WKUPCM_PHYS_ADDR, WKUPCM_SIZE, L4IO_MEM_NONCACHED, &l4wkup_virt_addr)) {
    printf("[L4WKUP] Error: Could not map device memory\n");
    return 1;
  }

  /* Set gptimer2 CLK to SYS_CLK 38.4MHZ */
  if (l4io_request_iomem(CM2_PHYS_ADDR, CM2_SIZE, L4IO_MEM_NONCACHED, &cm2_virt_addr)) {
    printf("[CM2] Error: Could not map device memory\n");
    return 1;
  }

  gptimer2_clkctrl = (volatile unsigned int *)(cm2_virt_addr + CM_L4PER_GPTIMER2_CLKCTRL - CM2_PHYS_ADDR);

  /* Set GPTIMER1 sys_clk */
  *gptimer2_clkctrl =  (*gptimer2_clkctrl) & (~(1 << SYS_CLK_BIT));

   /* Enable sys_clk */
  *gptimer2_clkctrl =
     ((*gptimer2_clkctrl) & (0xFFFFFFFC)) | ( MODULE_ENABLE_VALUE);

  /* Wait for Ready */
  while(((*gptimer2_clkctrl) >> IDLEST_START_BIT) & IDLEST_MASK);

  printf("End of timer2 setting.\n");

  if (l4io_request_iomem(GPTIMER2_PHYS_ADDR, 0x1000, L4IO_MEM_NONCACHED, (l4_addr_t *)(&timer2_reg))) {
    printf("[GPTIMER2] Error: Could not map device memory\n");
    return 2;
  }

  if (l4io_request_irq(GPTIMER2_IRQ, irq_caps[GPTIMER2_IRQ]) < 0) {
    printf("[GPTIMER2] Error: Could not get irq.\n");
    return 3;
  }

  /* Disable interrupts */
  timer2_reg->tier = 0;

  /* Clear_interrupts. */
  timer2_reg->tisr = TISR_OVF | TISR_MATCH;

  /* One-Shot Mode */
  install_irq_handler_sys (GPTIMER2_IRQ, hwtimer_irq_handler);

  hw_enable_irq(GPTIMER2_IRQ);

  return 0;
}

//-----------------------//
// set_hwtimer_l4_timer //
//-----------------------//
static void set_hwtimer_l4_timer (duration_t interval) {
  int flags;
  unsigned long counter = interval / 10000 * 384;		//counter = interval / NSECS_PER_SEC * TIMER_TICK_FREQ

  hw_save_flags_and_cli (flags);

  /* One-shot mode */
  timer2_reg->tisr =  TISR_OVF | TISR_MATCH;	//clear interrupts
  timer2_reg->tcrr = 0xFFFFFFFF - counter;	//counter starting point
  timer2_reg->tier = TIER_OVF;			//enable overflow interrupt
  timer2_reg->tclr = TCLR_SETUP;		//start
  /* timer1 */

  hw_enable_irq(GPTIMER2_IRQ);

  hw_restore_flags (flags);
}

//----------------------------//
// get_max_interval_l4_timer //
//----------------------------//

static duration_t get_max_interval_l4_timer (void) {
  return 0xFFFFFFFF /  38.4  * 1000;		/* 0xFFFFFFFF / TIMER_TICK_FREQ * NSECS_PER_SEC */
}

//----------------------------//
// get_min_interval_l4_timer //
//----------------------------//

static duration_t get_min_interval_l4_timer (void) {
  return NSECS_PER_SEC / TIMER_TICK_FREQ;
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
  hw_disable_irq(GPTIMER2_IRQ);
  install_irq_handler_sys (GPTIMER2_IRQ, NULL);
}

hwtimer_t l4_timer = {
  init_hwtimer: init_hwtimer_l4_timer,
  set_hwtimer: set_hwtimer_l4_timer,
  get_max_interval: get_max_interval_l4_timer,
  get_min_interval: get_min_interval_l4_timer,
  set_timer_handler: set_timer_handler_l4_timer,
  shutdown_hwtimer: shutdown_hwtimer_l4_timer,
};

//-----------------------//
// l4_timer_irq_handler //
//-----------------------//

static l4_cap_idx_t time_thread_cap;
static char time_thread_stack[8 << 10];

static void l4_timer_irq_handler (void) {

  /* Disable timer19 */
  timer10_reg->tier = 0 ;

  timer10_reg->tisr = TISR_OVF | TISR_MATCH;

  current_time_hi ++;

  /* Enable overflow interrupts */
  timer10_reg->tier = TISR_OVF;
}

//----------------//
// gptimer10_init //
//----------------//

static int gptimer10_init (void) {

  /* set GPTIMER10 frequency 38.4MHZ */
  l4_addr_t cm2_virt_addr = 0;
  volatile unsigned int *gptimer10_clkctrl;

  if (l4io_request_iomem(CM2_PHYS_ADDR, CM2_SIZE, L4IO_MEM_NONCACHED, &cm2_virt_addr)) {
    printf("[CM2] Error: Could not map device memory\n");
    return 1;
  }

  gptimer10_clkctrl = (volatile unsigned int *)(cm2_virt_addr + CM_L4PER_GPTIMER10_CLKCTRL - CM2_PHYS_ADDR);

  /* Set GPTIMER10 sys_clk */
  *gptimer10_clkctrl =  (*gptimer10_clkctrl) & (~(1 << SYS_CLK_BIT));

  /* Set GPTIMER10 sys_clk */
  *gptimer10_clkctrl =
     ((*gptimer10_clkctrl) & (0xFFFFFFFC)) | ( MODULE_ENABLE_VALUE);

  /* Wait for Ready */
  while(((*gptimer10_clkctrl) >> IDLEST_START_BIT) & IDLEST_MASK);

  printf("End of timer10 setting.\n");

  if (l4io_request_iomem(GPTIMER10_PHYS_ADDR, 0x1000, L4IO_MEM_NONCACHED, (l4_addr_t *)(&timer10_reg))) {
    printf("[OMAP GPTimer10] Error: Could not map device memory\n");
    return 2;
  }

  /* Periodic and autoreload mode */
  /* Disable timer10 first */
  timer10_reg->tier = 0;
  /* Clear all the interrupts */
  timer10_reg->tisr = TISR_OVF | TISR_MATCH;
  timer10_reg->tldr = 0;
  timer10_reg->tcrr = 0;
  timer10_reg->tier = TISR_OVF;
  timer10_reg->tclr = TCLR_SETUP;

  current_time_hi = 0;

  return 0;
}

//-----------------//
// time_irq_thread //
//-----------------//

static void time_irq_thread(void *data) {

  l4_cap_idx_t irq_cap = l4re_util_cap_alloc();
  l4_msgtag_t tag;

  //if(l4io_request_irq(GPTIMER1_IRQ, irq_cap) < 0)
  if(l4io_request_irq(GPTIMER10_IRQ, irq_cap) < 0) {
    printf("Request irq failed.\n");
    enter_kdebug("Request IRQ failed");
  }

  tag = l4_irq_attach(irq_cap, 0, time_thread_cap);
  if (l4_ipc_error(tag, l4_utcb())) {
    printf("Attach irq failed.\n");
    enter_kdebug("Attach IRQ failed");
  }

  tag = l4_irq_unmask(irq_cap);
  if (l4_ipc_error(tag, l4_utcb())) {
    printf("Unmask irq failed.\n");
    enter_kdebug("Unmask IRQ failed");
  }

  if(gptimer10_init()) {
     printf("GPTimer10 init failed.\n");
     enter_kdebug("GPTimer10 init failed");
  }

  l4_umword_t label;
  while(1) {
    tag = l4_irq_wait(irq_cap, &label, L4_IPC_NEVER);

    if (l4_ipc_error(tag, l4_utcb())) {
       printf("[KEYP] Error: Receive irq failed\n");
       continue;
    }

    l4_timer_irq_handler();
  }

}

//----------------//
// init_l4_timer //
//----------------//

static int init_l4_timer(void) {
  char name[15];
  l4_msgtag_t res;
  long err;
  l4_sched_param_t schedp;

  sprintf(name, "time.i%d", GPTIMER10_IRQ);

  time_thread_cap = l4re_util_cap_alloc();
  if (l4_is_invalid_cap(time_thread_cap))
    return 4;

  if((err = l4_error(l4_factory_create_thread(l4re_env()->factory, time_thread_cap))) > 0) {
    printf("Failed: %s\n", l4sys_errtostr(err));
    l4re_util_cap_free(time_thread_cap);
    return 5;
  }

  l4_addr_t kumem;
  l4_utcb_t *utcb;

  l4re_util_kumem_alloc(&kumem, 0, L4_BASE_TASK_CAP, l4re_env()->rm);
  utcb = (l4_utcb_t *)kumem;
  if(!utcb) {
    l4re_util_cap_release(time_thread_cap);
    l4re_util_cap_free(time_thread_cap);
    return 6;
  }

  l4_thread_control_start();
  l4_thread_control_pager(l4re_env()->rm);
  l4_thread_control_exc_handler(l4re_env()->rm);
  l4_thread_control_bind(utcb, L4_BASE_TASK_CAP);
  res = l4_thread_control_commit(time_thread_cap);
  if (l4_error(res)) {
    l4re_util_cap_release(time_thread_cap);
    l4re_util_cap_free(time_thread_cap);
    return 7;
  }

  schedp = l4_sched_param(2, 0);
  res = l4_scheduler_run_thread (l4re_env()->scheduler, time_thread_cap, &schedp);
  if (l4_error(res)) {
    l4re_util_cap_release(time_thread_cap);
    l4re_util_cap_free(time_thread_cap);
    return 8;
  }

  res = l4_thread_ex_regs(time_thread_cap, (l4_umword_t)time_irq_thread, 
      (l4_umword_t)(time_thread_stack + sizeof(time_thread_stack)), 0);
  if (l4_error(res)) {
    l4re_util_cap_release(time_thread_cap);
    l4re_util_cap_free(time_thread_cap);
    return 9;
  }

  return 0;
}

//----------------//
// read_l4_timer //
//----------------//

static hwtime_t read_l4_timer (void) {

  /*
   * The versatile kernel timer runs at 1MHz so no work is required here
   * to convert ticks into microseconds.
   */
  return ((unsigned long long)current_time_hi << 32) | ((unsigned long long)(timer10_reg->tcrr));
}

//---------------------//
// read_l4_timer_nsec //
//---------------------//

static duration_t read_l4_timer_nsec (void) {
  return read_l4_timer() / 384 * 10000;		//read_l4_timer() / TIMER_TICK_FREQ * NSECS_PER_SEC
}

//---------------------------//
// hwtime2duration_l4_timer //
//---------------------------//

static duration_t hwtime2duration_l4_timer (hwtime_t hwtime) {
  return hwtime / 384 * 10000;		// hwtime / TIMER_TICK_FREQ * NSES_PER_SEC
}

//------------------//
// getres_l4_timer //
//------------------//

static duration_t getres_l4_timer (void) {
  return 10000 / 384; 				// NSECS_PER_SEC / TIMER_TICK_FREQ;
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
  set_sysclk_freq();

  if (init_l4_timer() || init_hwtimer_l4_timer()) {
    printf("Init timer failed.\n");
    enter_kdebug("Timer Init Failed.\n");
  }

  printf("end of pit link.\n");

}
