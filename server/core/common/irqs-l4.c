#include <irqs.h>
#include <l4/processor.h>

#include <sched.h>

//--------//
// do_irq //
//--------//

extern void scheduling (void);

void do_irq (context_t context) {
  int flags;

  hw_save_flags_and_cli (flags);
#if 1
  irq_nesting_counter ++;
  if (hw_irq_ctrl [context.irqnr].ack)
    hw_ack_irq (context.irqnr);
#endif
  if (irq_handler_table [context.irqnr])
    (*irq_handler_table [context.irqnr]) (&context);
  else
    default_irq_handler (&context);

#if 1
  if (hw_irq_ctrl [context.irqnr].end)
    hw_end_irq (context.irqnr);

  irq_nesting_counter --;
#endif

  hw_restore_flags (flags);

  if (irq_nesting_counter == SCHED_PENDING) {
    scheduling ();
  }

  do_signals();
}

//---------------//
// init_arch_irq //
//---------------//

extern void init_l4_icu (void);
int init_arch_irqs (void) {

  init_l4_icu ();

  return 0;
}
