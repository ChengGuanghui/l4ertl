#include <l4/sys/kdebug.h>
#include <l4/sys/debugger.h>
#include <l4/sys/thread.h>
#include <l4/sys/vcpu.h>
#include <l4/vcpu/vcpu.h>
#include <l4/processor.h>
#include <l4/regs.h>

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>

//extern void context_switch (pthread_t new_thread, pthread_t *current_thread);
//                                         r0                     r1

//void context_switch (pthread_t new_thread, pthread_t *current)
void context_switch_to (pthread_t new_thread)
{
  printf("old_thread:0x%x state:%d  current_thread:0x%x state:%d\n", (unsigned long)current_thread, GET_THREAD_STATE(current_thread), (unsigned long)new_thread, GET_THREAD_STATE(new_thread));
  int disable = 0;
  unsigned int sp;
  if(l4ertl_vcpu->state & L4_VCPU_F_IRQ)
    disable = 1;

  if(disable)
    hw_cli();

  //unsigned long cpsr;
  pthread_t old_thread = current_thread;
  unsigned long retaddr = __builtin_return_address(0);

#if 0
  printf("new thread: 0x%x old_thread: 0x%x\n", (unsigned long)new_thread, (unsigned long)old_thread);

  printf("old thread prio: %d\n", old_thread->sched_param.sched_priority);
  printf("context switch from old thread: 0x%x IP: 0x%x to new thread: 0x%x\n", (unsigned long)old_thread,old_thread->entry_uregs.ARM_pc, (unsigned long)new_thread);
  printf("retaddr 0x%x\n", retaddr);
#endif

  unsigned long lr, fp, r10, r9, r8, r7, r6, r5, r4, r3, r2, r1, r0;
  asm volatile (
//      "mov lr, %[retaddr]		\t\n"
//      "mov %[lrvalue], lr		\t\n"
      "stmdb sp!, {r0 - r10, fp, lr}	\t\n"		//store {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
      "str %[retaddr], [sp, #44]	\t\n"		//fix lr in the  stack
//      "str ip, [sp, #-4]! 		\t\n"		//store ip
      "str sp, %[ARMsp]		\t\n"		//store sp
//      "mov %[lrvalue], lr               \t\n"
//      "mov %[fpvalue], fp               \t\n"
//      "mov %[r10value], r10             \t\n"
//      "mov %[r9value], r9               \t\n"
//      "mov %[r8value], r8               \t\n"
//      "mov %[r7value], r7               \t\n"
//      "mov %[r6value], r6               \t\n"
//      "mov %[r5value], r5               \t\n"
//      "mov %[r4value], r4               \t\n"
//      "mov %[r3value], r3               \t\n"
//      "mov %[r2value], r2               \t\n"
//      "mov %[r1value], r1               \t\n"
//      "mov %[r0value], r0               \t\n"
      : [lrvalue] "=r" (lr), [ARMsp] "=m" (old_thread->ksp)
//, [fpvalue] "=r" (fp), [r10value] "=r" (r10), [r9value] "=r" (r9), [r8value] "=r" (r8), [r7value] "=r" (r7), [r6value] "=r" (r6), [r5value] "=r" (r5), [r4value] "=r" (r4), [r3value] "=r" (r3), [r2value] "=r" (r2), [r1value] "=r" (r1), [r0value] "=r" (r0)
      : [retaddr] "r" (retaddr));

//  printf("switch from kernel mode lr:0x%x fp:0x%x r10:0x%x r9:0x%x r8:0x%x r7:0x%x r6:0x%x r5:0x%x r4:0x%x r3:0x%x r2:0x%x r1:0x%x r0:0x%x\n", lr, fp, r10, r9, r8, r7, r6, r5, r4, r3, r2, r1, r0);

#if 0
  printf("-4 0x%x\n", *((unsigned long *)(old_thread->ksp - 4 * 4)));
  printf("-3 0x%x\n", *((unsigned long *)(old_thread->ksp - 3 * 4)));
  printf("-2 0x%x\n", *((unsigned long *)(old_thread->ksp - 2 * 4)));
  printf("-1 0x%x\n", *((unsigned long *)(old_thread->ksp - 1 * 4)));
  printf("0 0x%x\n", *((unsigned long *)(old_thread->ksp + 0 * 4)));
  printf("1 0x%x\n", *((unsigned long *)(old_thread->ksp + 1 * 4)));
  printf("2 0x%x\n", *((unsigned long *)(old_thread->ksp + 2 * 4)));
  printf("3 0x%x\n", *((unsigned long *)(old_thread->ksp + 3 * 4)));
  printf("4 0x%x\n", *((unsigned long *)(old_thread->ksp + 4 * 4)));
  printf("5 0x%x\n", *((unsigned long *)(old_thread->ksp + 5 * 4)));
  printf("6 0x%x\n", *((unsigned long *)(old_thread->ksp + 6 * 4)));
  printf("7 0x%x\n", *((unsigned long *)(old_thread->ksp + 7 * 4)));
  printf("8 0x%x\n", *((unsigned long *)(old_thread->ksp + 8 * 4)));
  printf("9 0x%x\n", *((unsigned long *)(old_thread->ksp + 9 * 4)));
  printf("10 0x%x\n", *((unsigned long *)(old_thread->ksp + 10 * 4)));
  printf("11 0x%x\n", *((unsigned long *)(old_thread->ksp + 11 * 4)));
  printf("12 0x%x\n", *((unsigned long *)(old_thread->ksp + 12 * 4)));
  printf("13 0x%x\n", *((unsigned long *)(old_thread->ksp + 13 * 4)));
  printf("14 0x%x\n", *((unsigned long *)(old_thread->ksp + 14 * 4)));
  printf("15 0x%x\n", *((unsigned long *)(old_thread->ksp + 15 * 4)));
  printf("16 0x%x\n", *((unsigned long *)(old_thread->ksp + 16 * 4)));
  printf("17 0x%x\n", *((unsigned long *)(old_thread->ksp + 17 * 4)));
  printf("18 0x%x\n", *((unsigned long *)(old_thread->ksp + 18 * 4)));

  printf("OLD_thread 0x%x return address 0x%x sp 0x%x loc 0x%x\n", (unsigned long)old_thread, retaddr, *(unsigned long *)(old_thread->ksp), old_thread->ksp);
  printf("NEW_thread 0x%x sp 0x%x\n", (unsigned long)new_thread,  new_thread->ksp);
#endif
  //old_thread->kregs.ARM_pc = retaddr;
  //old_thread->kregs.ARM_cpsr = get_current_cpsr();

  old_thread->cpsr &= ~L4_VCPU_F_USER_MODE;

  current_thread = new_thread;

  if(new_thread->kentry_sp == 0) {
     unsigned long sp;

     sp = *(unsigned long *)(new_thread);
     l4ertl_vcpu->r.ip = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->saved_state = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.lr = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[12] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[11] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[10] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[9] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[8] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[7] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[6] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[5] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[4] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[3] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[2] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[1] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.r[0] = *((unsigned long*)sp);

     sp += 4;
     l4ertl_vcpu->r.sp = *((unsigned long*)sp);


    //allocate kernel stack
    alloc_kstack(new_thread);
    l4ertl_vcpu->entry_sp = new_thread->kentry_sp;
    dbprintf("new thread 0x%x sp 0x%x\n",  (unsigned long)new_thread, new_thread->kentry_sp);

    l4_thread_vcpu_resume_commit(L4_INVALID_CAP, l4_thread_vcpu_resume_start());

  }
  else if(new_thread->cpsr & L4_VCPU_F_USER_MODE) {
    dbprintf("switch to user mode 0x%x\n", (unsigned long)new_thread);

    ptregs_to_vcpu(l4ertl_vcpu, (struct pt_regs *)(&new_thread->entry_uregs));
    l4ertl_vcpu->saved_state = L4_VCPU_F_USER_MODE              //cpsr
             | L4_VCPU_F_EXCEPTIONS
             | L4_VCPU_F_PAGE_FAULTS
             | L4_VCPU_F_IRQ
             | L4_VCPU_F_FPU_ENABLED;
   l4_thread_vcpu_resume_commit(L4_INVALID_CAP, l4_thread_vcpu_resume_start());
  }
  else if(!(new_thread->cpsr & L4_VCPU_F_USER_MODE)) {

  dbprintf("switch to kernel mode 0x%x.\n", (unsigned long)new_thread);
#if 0
  printf("M old_thread 0x%x sp 0x%x\n", (unsigned long)old_thread, old_thread->kregs.ARM_sp);
  printf("M new_thread 0x%x swith to kernel mode sp 0x%x.\n", (unsigned long)new_thread, new_thread->kregs.ARM_sp);

  printf("old_thread:0x%x sp 0x%x\n", (unsigned long)old_thread, old_thread->kregs.ARM_sp);
  printf("old_thread:0x%x r0 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp)));
  printf("old_thread:0x%x r1 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 0 * 4)));
  printf("old_thread:0x%x r2 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 1 * 4)));
  printf("old_thread:0x%x r3 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 2 * 4)));
  printf("old_thread:0x%x r4 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 3 * 4)));
  printf("old_thread:0x%x r5 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 4 * 4)));
  printf("old_thread:0x%x r6 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 5 * 4)));
  printf("old_thread:0x%x r7 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 6 * 4)));
  printf("old_thread:0x%x r8 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 7 * 4)));
  printf("old_thread:0x%x r9 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 8 * 4)));
  printf("old_thread:0x%x r10 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 9 * 4)));
  printf("old_thread:0x%x fp 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 10 * 4)));
  printf("old_thread:0x%x lr 0x%x\n", (unsigned long)old_thread, *((unsigned long *)(old_thread->kregs.ARM_sp + 11 * 4)));

  printf("new_thread:0x%x  sp 0x%x\n",(unsigned long)new_thread, new_thread->kregs.ARM_sp);
  printf("new_thread:0x%x -1 0x%x\n",(unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp - 1 * 4)));
  printf("new_thread:0x%x r0 0x%x\n",(unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp)));
  printf("new_thread:0x%x r1 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 0 * 4)));
  printf("new_thread:0x%x r2 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 1 * 4)));
  printf("new_thread:0x%x r3 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 2 * 4)));
  printf("new_thread:0x%x r4 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 3 * 4)));
  printf("new_thread:0x%x r5 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 4 * 4)));
  printf("new_thread:0x%x r6 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 5 * 4)));
  printf("new_thread:0x%x r7 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 6 * 4)));
  printf("new_thread:0x%x r8 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 7 * 4)));
  printf("new_thread:0x%x r9 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 8 * 4)));
  printf("new_thread:0x%x r10 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 9 * 4)));
  printf("new_thread:0x%x fp 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 10 * 4)));
  printf("new_thread:0x%x lr 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 11 * 4)));
  printf("new_thread:0x%x r13 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 12 * 4)));
  printf("new_thread:0x%x r13 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 13 * 4)));
  printf("new_thread:0x%x r13 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 14 * 4)));
  printf("new_thread:0x%x r13 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 15 * 4)));
  printf("new_thread:0x%x r13 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 16 * 4)));
  printf("new_thread:0x%x r13 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 17 * 4)));
  printf("new_thread:0x%x r13 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 18 * 4)));
  printf("new_thread:0x%x r13 0x%x\n", (unsigned long)new_thread, *((unsigned long *)(new_thread->kregs.ARM_sp + 19 * 4)));
#endif
    //new_thread->kernel_context_switch_happened = 0;		//kernel mode context switch not happened yet

    __asm__ __volatile__ (
      "ldr sp, %[ARMsp]			\t\n"
//      "ldr ip, [sp, #4]!   		\t\n"
      "ldmia sp!, {r0 - r10, fp, lr}	\t\n"
      "mov %[lrvalue], lr		\t\n"
//      "mov %[fpvalue], fp		\t\n"
//      "mov %[r10value], r10		\t\n"
//      "mov %[r9value], r9		\t\n"
//      "mov %[r8value], r8		\t\n"
//      "mov %[r7value], r7		\t\n"
//      "mov %[r6value], r6		\t\n"
//      "mov %[r5value], r5		\t\n"
//      "mov %[r4value], r4		\t\n"
//      "mov %[r3value], r3		\t\n"
//      "mov %[r2value], r2		\t\n"
//      "mov %[r1value], r1		\t\n"
//      "mov %[r0value], r0		\t\n"
        "mov %[spvalue], sp		\t\n"
      : [lrvalue] "=r" (lr),  [spvalue] "=r" (sp)
//, [fpvalue] "=r" (fp), [r10value] "=r" (r10), [r9value] "=r" (r9), [r8value] "=r" (r8), [r7value] "=r" (r7), [r6value] "=r" (r6), [r5value] "=r" (r5), [r4value] "=r" (r4), [r3value] "=r" (r3), [r2value] "=r" (r2), [r1value] "=r" (r1), [r0value] "=r" (r0)
      : [ARMsp] "m" (new_thread->ksp));

    current_thread = old_thread;
    //current_thread->kernel_context_switch_happened = 1;		//kernel mode context switch happened
#if 0
    printf("current_thread 0x%x new_thread 0x%x old_thread 0x%x lr 0%x sp 0x%x\n", (unsigned long)current_thread, (unsigned long)new_thread, (unsigned long)old_thread, lr, sp);

  printf("M 0 0x%x\n", __builtin_return_address(0));
  printf("M 0 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp)));
  printf("M 1 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 1 * 4)));
  printf("M 2 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 2 * 4)));
  printf("M 3 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 3 * 4)));
  printf("M 4 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 4 * 4)));
  printf("M 5 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 5 * 4)));
  printf("M 6 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 6 * 4)));
  printf("M 7 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 7 * 4)));
  printf("M 8 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 8 * 4)));
  printf("M 9 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 9 * 4)));
  printf("M 10 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 10 * 4)));
  printf("M 11 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 11 * 4)));
  printf("M 12 0x%x\n", *((unsigned long *)(current_thread->kregs.ARM_sp + 12 * 4)));

#endif
    //printf("switch to kernel mode lr:0x%x fp:0x%x r10:0x%x r9:0x%x r8:0x%x r7:0x%x r6:0x%x r5:0x%x r4:0x%x r3:0x%x r2:0x%x r1:0x%x r0:0x%x\n", lr, fp, r10, r9, r8, r7, r6, r5, r4, r3, r2, r1, r0);
  }

}

void vcpu_to_pthread(l4_vcpu_state_t *vcpu, pthread_t thread)
{
  if(vcpu->saved_state & L4_VCPU_F_USER_MODE)
    vcpu_to_ptregs(vcpu, (struct pt_regs *)(&thread->entry_uregs));
  else {printf("vcpu_to_pthread entry_kregs.\n");
    vcpu_to_ptregs(vcpu, (struct pt_regs *)(&thread->entry_kregs));}
}

void pthread_to_vcpu(l4_vcpu_state_t *vcpu, pthread_t thread)
{
  if(thread->saved_cpsr & L4_VCPU_F_USER_MODE)
    ptregs_to_vcpu(vcpu, (struct pt_regs *)(&thread->entry_uregs));
  else {printf("pthread_to_vcpu entry_kregs.\n");
    ptregs_to_vcpu(vcpu, (struct pt_regs *)(&thread->entry_kregs));}
}
