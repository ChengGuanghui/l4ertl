#ifndef _ARCH_THREAD_H_
#define _ARCH_THREAD_H_

#include <l4/vcpu/vcpu.h>

l4_utcb_t * l4ertl_thread_create(void (*thread_func)(void *data),
                                 unsigned vcpu,
                                 void *stack_pointer,
                                 void *stack_data, unsigned stack_data_size,
                                 int prio,
                                 l4_vcpu_state_t **vcpu_state,
                                 const char *name, int irq);

#endif

