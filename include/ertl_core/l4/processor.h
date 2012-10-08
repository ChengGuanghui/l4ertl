/*
 * $FILE: processor.h
 *
 * Processor related functions: Context switch, Enable/Inable
 * interrupts, etc
 *
 * $VERSION: 1.0
 *
 * Author: Miguel Masmano <mimastel@doctor.upv.es>
 *
 * $LICENSE:  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef _ARCH_PROCESSOR_H_
#define _ARCH_PROCESSOR_H_

#ifndef _KERNEL_
#error Internal file. Do not include it in your sources.
#endif

#define IDLE

#include <stdio.h>
#include <types.h>
#include <l4/sys/vcpu.h>
#include <l4/vcpu/vcpu.h>

#include <l4/irqs.h>

#define L4_VCPU_F_HANDLING_SIGNAL 0x100
#define L4_VCPU_F_UNFINISHED_SIGNAL 0x1000

extern l4_vcpu_state_t *l4ertl_vcpu;
extern l4_cap_idx_t vcpu_task;
extern l4_cap_idx_t vcpu_cap;

static void l4ertl_do_pending_irq(l4_vcpu_state_t *vcpu) {
  context_t context;
  context.irqnr = vcpu->i.label;
  do_irq(context);
  do_signals();
}

static void l4ertl_setup_ipc(l4_utcb_t *utcb) {
}

static inline void hw_cli(void) {
  l4vcpu_irq_disable(l4ertl_vcpu);
}

static inline void hw_sti(void) {
  l4vcpu_irq_enable(l4ertl_vcpu, l4_utcb(), l4ertl_do_pending_irq, l4ertl_setup_ipc);
}

static inline void hw_restore_flags(unsigned long flags) {
  //l4vcpu_irq_restore(l4ertl_vcpu, flags, l4_utcb(), l4ertl_do_pending_irq, l4ertl_setup_ipc);
}

static inline unsigned long get_current_cpsr() {
  return l4ertl_vcpu->state;
}

#define hw_save_flags(flags) flags = l4vcpu_state(l4ertl_vcpu)

#define hw_save_flags_and_cli(flags) {\
  hw_save_flags(flags); \
}

#if 0
#define hw_save_flags_and_cli(flags) {\
  hw_save_flags(flags); \
  hw_cli();		\
}
#endif

void context_switch (pthread_t, pthread_t *);
void context_switch_to (pthread_t);

void vcpu_to_pthread(l4_vcpu_state_t *vcpu, pthread_t thread);

void pthread_to_vcpu(l4_vcpu_state_t *vcpu, pthread_t thread);

void ret_may_switch(pthread_t new_thread, pthread_t old_thread);

#define CALL_SIGNAL_HANDLER(hdl, signal, sinfo, void_p) {}

#define CALL_CANCEL_HANDLER(hdl, arg) {}

#endif
