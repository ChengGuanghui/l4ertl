#include <l4/re/c/util/cap_alloc.h>
#include <l4/sys/debugger.h>
#include <l4/sys/kdebug.h>
#include <l4/re/env.h>
#include <l4/sys/ipc.h>
#include <l4/sys/factory.h>
#include <l4/sys/thread.h>
#include <l4/sys/irq.h>
#include <l4/sys/err.h>
#include <l4/re/env.h>
#include <l4/util/util.h>
#include <l4/io/io.h>

#include <config.h>
#include <bitop.h>
#include <irqs.h>
#include <l4/irqs.h>
#include <l4/processor.h>
#include <l4/thread.h>

#include <string.h>
#include <stdio.h>

#define CONFIG_PRIORITY_BASE 2

#define L4ERTL_GPTIMER1_HW_IRQ 	70	//versatile express A9 board
#define L4ERTL_GPTIMER2_HW_IRQ 	78	//versatile express A9 board

l4_cap_idx_t vcpu_self_cap;
extern l4_cap_idx_t vcpu_cap;
unsigned long irq_bits[IRQ_NR];
l4_cap_idx_t irq_caps[IRQ_NR];
l4_cap_idx_t irq_thread_caps[IRQ_NR];
unsigned long irq_nums[IRQ_NR];
unsigned long irq_prios[IRQ_NR];

//----------------//
//  assert_irq    //
//----------------//
static void assert_irq(unsigned int irq) {
  if(irq != L4ERTL_GPTIMER1_HW_IRQ && irq != L4ERTL_GPTIMER2_HW_IRQ) {
     printf("IRQ No. %d is not permitted to be triggered yet.\n", irq);
     enter_kdebug("Exception");
  }
}

//---------------//
// enable_l4_irq //
//------------- -//

static void enable_l4_irq (unsigned int irq) {

  int flags, error;
  l4_msgtag_t tag;

  assert_irq(irq);

  hw_save_flags_and_cli(flags);

  irq_bits[irq] = 1;

  tag = l4_irq_attach(irq_caps[irq], irq, vcpu_cap);//	46/70
  //tag = l4_irq_attach(irq_caps[irq], irq_nums[irq] << 2, vcpu_self_cap);	46/70
  //tag = l4_irq_attach(irq_caps[irq], irq_nums[irq] << 2, l4re_env()->main_thread); 46/70
  if(error = l4_ipc_error(tag, l4_utcb()))
    printf("attach error %s\n", l4sys_errtostr(error));

  tag = l4_irq_unmask(irq_caps[irq]);
  if(error = l4_ipc_error(tag, l4_utcb()))
    printf("unmask error %s\n", l4sys_errtostr(error));

  hw_restore_flags (flags);
}

//-----------------//
// disable_l4_irq  //
//----------------//

static void disable_l4_irq (unsigned int irq) {
  int flags;

  assert_irq(irq);

  hw_save_flags_and_cli(flags);

  irq_bits[irq] = 0;
  //printf("%s irq %d\n", __func__, irq);
  l4_irq_detach(irq_caps[irq]);

  hw_restore_flags (flags);
}

//----------------//
//  ack_l4_irq    //
//----------------//

static void ack_l4_irq(unsigned int irq) {
  //printf("%s: %d\n", __func__, irq);
}

//----------------//
//  end_l4_irq    //
//----------------//

static void end_l4_irq(unsigned int irq) {
  //printf("%s: %d\n", __func__, irq);
}

void init_l4_icu(void);
void init_l4_icu (void) {
  int irq;

  vcpu_self_cap = l4re_get_env_cap("vcpu thread");
  if(l4_capability_equal(vcpu_self_cap, vcpu_cap))
     printf("equal %d.\n", l4_debugger_global_id(vcpu_cap));
   else
     printf("unequal %d %d.\n", l4_debugger_global_id(vcpu_cap), l4_debugger_global_id(vcpu_self_cap));

  for (irq = 0; irq < IRQ_NR; irq ++) {

    irq_bits[irq] = 0; //disable all irqs initially

    hw_irq_ctrl [irq] = (hw_irq_ctrl_t){
      .enable = enable_l4_irq,
      .disable = disable_l4_irq,
      .ack = disable_l4_irq,
      .end = enable_l4_irq,
    };

    irq_caps[irq] = l4re_util_cap_alloc();
  }

   l4_msgtag_t tag;
   irq_nums[L4ERTL_GPTIMER1_HW_IRQ] =  L4ERTL_GPTIMER1_HW_IRQ;
   irq_nums[L4ERTL_GPTIMER2_HW_IRQ] =  L4ERTL_GPTIMER2_HW_IRQ;
}
