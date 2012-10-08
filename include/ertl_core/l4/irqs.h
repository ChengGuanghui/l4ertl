/*
 * $FILE: irqs.h
 *
 * Arch-dependent part of the exceptions
 *
 * $VERSION: 1.0
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
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

#ifndef _ARCH_EXCEPTIONS_H_
#define _ARCH_EXCEPTIONS_H_

#ifdef _KERNEL_

#include <l4/sys/types.h>
#include <config.h>

#define TRAP_NR 8

#define IRQ_NR 128 /* Board dependent */

extern unsigned long irq_bits[IRQ_NR];
extern l4_cap_idx_t irq_caps[IRQ_NR];	
extern l4_cap_idx_t irq_thread_caps[IRQ_NR];	
extern unsigned long irq_nums[IRQ_NR];
extern unsigned long irq_prios[IRQ_NR];

#define L4_TIMER_IRQ 0

#endif	// _KERNEL_

#ifndef __ASSEMBLY__

typedef struct context_struct {
  unsigned long error_code:16;
  unsigned long irqnr:16;
} context_t;

void do_irq (context_t context); 
void do_signals(void);

#endif

#endif
