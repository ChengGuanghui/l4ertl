#include <l4/irqs.h>
#include <l4/thread.h>

#include <l4/sys/kdebug.h>
#include <l4/sys/err.h>

#include <l4/sys/thread.h>
#include <l4/sys/scheduler.h>
#include <l4/sys/factory.h>
#include <l4/re/env.h>
#include <l4/re/c/rm.h>
#include <l4/log/log.h>
#include <l4/re/c/util/cap.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/util/kumem_alloc.h>
#include <l4/sys/debugger.h>
#include <l4/vcpu/vcpu.h>

#include <string.h>
#include <stdio.h>

#define L4ERTL_IRQ_THREAD_STACK_SIZE 8192

static char irq_thread_stacks[L4ERTL_IRQ_THREAD_STACK_SIZE * IRQ_NR] 
   __attribute((aligned(L4ERTL_IRQ_THREAD_STACK_SIZE)));

#ifdef ARCH_arm
void __thread_launch(void);
asm(
"__thread_launch:\n"
"	ldmia sp!, {r0}\n" // arg1
"	ldmia sp!, {r1}\n" // func
"	ldmia sp!, {lr}\n" // ret
"	bic sp, sp, #7\n"
"	mov pc, r1\n"
);
#endif
#ifdef ARCH_amd64
void __thread_launch(void);
asm(
"__thread_launch:\n"
"	popq %rdi\n" // arg1
"	ret\n"
);
#endif

l4_utcb_t * l4ertl_thread_create(  void (*thread_func)(void *data),
                                 unsigned vcpu,
                                 void *stack_pointer,
                                 void *stack_data, unsigned stack_data_size,
                                 int prio,
                                 l4_vcpu_state_t **vcpu_state,
                                 const char *name, int irq)
{
	l4_cap_idx_t l4cap;
	l4_sched_param_t schedp;
	l4_msgtag_t res;
	char l4ertl_name[20] = "l4ertl.";
	l4_utcb_t *utcb;
	l4_umword_t *sp, *sp_data;

	/* Prefix name with 'l4ertl.' */
	strncpy(l4ertl_name + strlen(l4ertl_name), name,
	        sizeof(l4ertl_name) - strlen(l4ertl_name));
	l4ertl_name[sizeof(l4ertl_name) - 1] = 0;

	l4cap = l4re_util_cap_alloc();
	if (l4_is_invalid_cap(l4cap))
		return 0;

	irq_thread_caps[irq] = l4cap;

	res = l4_factory_create_thread(l4re_env()->factory, l4cap);
	if (l4_error(res))
		goto out_free_cap;

	if (!stack_pointer) {
		stack_pointer = (char *)(irq_thread_stacks[L4ERTL_IRQ_THREAD_STACK_SIZE * irq] + L4ERTL_IRQ_THREAD_STACK_SIZE - 1);
		if (!stack_pointer) {
			printf("no more stacks, bye");
			goto out_free_cap;
		}
	}


	sp_data = (l4_umword_t *)((char *)stack_pointer - stack_data_size);
	memcpy(sp_data, stack_data, stack_data_size);

	sp = sp_data;
#ifdef ARCH_amd64
	sp = (l4_umword_t *)((l4_umword_t)sp & ~0xf);
	*(--sp) = 0;
	*(--sp) = (l4_umword_t)thread_func;
	*(--sp) = (l4_umword_t)sp_data;
#elif defined(ARCH_arm)
	*(--sp) = 0;
	*(--sp) = (l4_umword_t)thread_func;
	*(--sp) = (l4_umword_t)sp_data;
#else
	*(--sp) = (l4_umword_t)sp_data;
	*(--sp) = 0;
#endif

	l4_debugger_set_object_name(l4cap, l4ertl_name);

        l4_addr_t kumem;
        l4re_util_kumem_alloc(&kumem, 0, L4_BASE_TASK_CAP, l4re_env()->rm);
        utcb = (l4_utcb_t *)kumem;
	if (!utcb)
		goto out_rel_cap;

	if (vcpu_state) {
		*vcpu_state = (l4_vcpu_state_t *)(kumem + L4_UTCB_OFFSET);
		if (!*vcpu_state)
			goto out_rel_cap;
	}

	l4_thread_control_start();
	l4_thread_control_pager(l4re_env()->rm);
	l4_thread_control_exc_handler(l4re_env()->rm);
	l4_thread_control_bind(utcb, L4_BASE_TASK_CAP);
	res = l4_thread_control_commit(l4cap);
	if (l4_error(res))
		goto out_rel_cap;

	if (vcpu_state) {
		res = l4_thread_vcpu_control(l4cap, (l4_addr_t)(*vcpu_state));

		if (l4_error(res))
			goto out_rel_cap;
	}

	schedp = l4_sched_param(prio, 0);
	//schedp.affinity = l4_sched_cpu_set(l4x_cpu_physmap_get_id(vcpu), 0, 1);

	res = l4_scheduler_run_thread (l4re_env()->scheduler, l4cap, &schedp);
	if (l4_error(res)) {
		printf("%s: Failed to set cpu%d of thread '%s': %ld.\n",
		           __func__, vcpu, name, l4_error(res));
		goto out_rel_cap;
	}

#if defined(ARCH_arm) || defined(ARCH_amd64)
	res = l4_thread_ex_regs(l4cap, (l4_umword_t)__thread_launch,
	                        (l4_umword_t)sp, 0);
#else
	res = l4_thread_ex_regs(l4cap, (l4_umword_t)thread_func,
	                        (l4_umword_t)sp, 0);
#endif
	if (l4_error(res))
		goto out_rel_cap;

	return utcb;

out_rel_cap:
	l4re_util_cap_release(l4cap);
out_free_cap:
	l4re_util_cap_free(l4cap);

	return 0;
}
