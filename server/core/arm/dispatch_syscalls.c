#include <l4/sys/kdebug.h>
#include <l4/sys/debugger.h>
#include <l4/sys/thread.h>
#include <l4/sys/vcpu.h>
#include <l4/vcpu/vcpu.h>
#include <l4/re/c/log.h>

#include <l4/processor.h>
#include <l4/irqs.h>
#include <l4/regs.h>
#include <syscall-test.h>
#include <syscalls.h>

#include <stdio.h>
#include <sched.h>

#define L4ERTL_ARM_SYSCALL        0x000F0042    /* carefully chosen... */

static void dispatch_syscalls(int nr, int op, struct pt_regs *regs)
{

  //printf("before thread: 0x%x\n", (unsigned long)current_thread);
  switch(nr)
  {
    case 0:
      //printf("para %d with name :%s\n", nr, syscall_names[op]);
      regs->ARM_r0 = ((int (*)(void))syscall_table[op])();
      break;
    case 1:
      //printf("para %d with name :%s ret: 0x%x arg 0x%x\n", nr, syscall_names[op], regs->ARM_r0,  regs->ARM_r1);
      regs->ARM_r0 = ((int (*)(unsigned long)) syscall_table[op])(regs->ARM_r1);
      break;
    case 2:
      //printf("para %d with name :%s ret: 0x%x arg 0x%x, 0x%x\n", nr, syscall_names[op], regs->ARM_r0, regs->ARM_r1 , regs->ARM_r2);
      regs->ARM_r0 = ((int (*) (unsigned long, unsigned long))syscall_table[op])(regs->ARM_r1, regs->ARM_r2);
      break;
    case 3:
      //if(op != write_nr)
        //printf("para %d with name :%s ret: 0x%x arg 0x%x, 0x%x, 0x%x\n", nr, syscall_names[op], regs->ARM_r0, regs->ARM_r1, regs->ARM_r2, regs->ARM_r3);
      regs->ARM_r0 =((int (*) (unsigned long, unsigned long, unsigned long)) syscall_table[op])
                                                                (regs->ARM_r1, regs->ARM_r2, regs->ARM_r3);
      break;
    case 4:
      //printf("para %d with name :%s ret: 0x%x arg: 0x%x, 0x%x, 0x%x, 0x%x\n", nr, syscall_names[op], regs->ARM_r0, regs->ARM_r1, regs->ARM_r2, regs->ARM_r3, regs->ARM_r4);
      if(op == pthread_create_idle_nr) {
        printf("pthread_create_idle_nr.\n");
        ((int (*)(unsigned long, unsigned long, unsigned long, unsigned long)) syscall_table[op])(regs->ARM_r1, regs->ARM_r2, regs->ARM_r3, regs->ARM_r4);
      }
      else
        regs->ARM_r0 = ((int (*)(unsigned long, unsigned long, unsigned long, unsigned long)) syscall_table[op])(regs->ARM_r1, regs->ARM_r2, regs->ARM_r3, regs->ARM_r4);
        break;
      case 5:
        //printf("para %d with name :%s ret: 0x%x arg: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", nr, syscall_names[op], regs->ARM_r0, regs->ARM_r1, regs->ARM_r2, regs->ARM_r3, regs->ARM_r4, regs->ARM_r5);
        if(op == pthread_create_nr) {
          //printf("pthread_create_nr.\n");
          ((int (*)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long))syscall_table[op])(regs->ARM_r1, regs->ARM_r2, regs->ARM_r3, regs->ARM_r4, regs->ARM_r5);
        }
        else
          regs->ARM_r0 = ((int (*)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long))syscall_table[op])(regs->ARM_r1, regs->ARM_r2, regs->ARM_r3, regs->ARM_r4, regs->ARM_r5);
        break;
      case 6:
        //printf("para %d with name :%s ret: 0x%x arg: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", nr, syscall_names[op], regs->ARM_r0, regs->ARM_r1, regs->ARM_r2, regs->ARM_r3, regs->ARM_r4, regs->ARM_r5, regs->ARM_r6);
        regs->ARM_r0 = ((int (*)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long))syscall_table[op])(regs->ARM_r1, regs->ARM_r2, regs->ARM_r3, regs->ARM_r4, regs->ARM_r5, regs->ARM_r6);
        break;
      default:
        printf("Warning: Unsupported System Call %d at op %d\n", nr, op);

  }
  //printf("after thread: 0x%x\n", (unsigned long)current_thread);
}

static void l4ertl_entry_from_user(l4_vcpu_state_t *vcpu)
{
  if(l4vcpu_is_irq_entry(vcpu))
  {
    if (vcpu->i.label != 0)
    {
      context_t context;
      context.irqnr = vcpu->i.label;
      printf("hehe start of do irq %d thread: 0x%x.\n", (int)(vcpu->i.label), (unsigned long)current_thread);
      do_irq(context);
      printf("end of do irq. 0x%x\n", (unsigned long)current_thread);
      //current_thread->cpsr = L4_VCPU_F_USER_MODE;
      //pthread_to_vcpu(vcpu, current_thread);	//sync to back to the l4vcpu
    }
    else
    {
      printf("Unclassifiable message\n");
    }
  }

  else if(l4vcpu_is_page_fault_entry(vcpu))
  {
    if(vcpu->saved_state & L4_VCPU_F_HANDLING_SIGNAL)
        printf("papa.\n");
    else
        printf("pepe.\n");
    if((vcpu->saved_state & L4_VCPU_F_HANDLING_SIGNAL) && (vcpu->r.pfa == 0xFEDCBA00))	//port/signal.c
    {
      printf("get signal page fault sp : 0x%x.\n", (unsigned long)(current_thread->signal_kregs.ARM_sp));
      printf("get signal page fault pc : 0x%x.\n", (unsigned long)(current_thread->signal_kregs.ARM_pc));
      __asm__ __volatile__ (
        "mov sp, %[sp]			\t\n"
       // "mov r0, %[r0]			\t\n"
       // "mov r1, %[r1]			\t\n"
/*        "mov r2, %[r2]			\t\n"
        "mov r3, %[r3]			\t\n"
*/        "mov r4, %[r4]			\t\n"
        "mov r5, %[r5]			\t\n"
        "mov r6, %[r6]			\t\n"
        "mov r7, %[r7]			\t\n"
        "mov r8, %[r8]			\t\n"
        "mov r9, %[r9]			\t\n"
        "mov r10, %[r10]		\t\n"
        "mov r11, %[r11]		\t\n"
        "mov r12, %[r12]		\t\n"
        "mov lr, %[lr]			\t\n"
        "mov pc, %[pc]			\t\n"
        :: [sp] "r" (current_thread->signal_kregs.ARM_sp),
/*
        [r0] "r" (current_thread->signal_kregs.ARM_r0),
        [r1] "r" (current_thread->signal_kregs.ARM_r1),
        */[r2] "r" (current_thread->signal_kregs.ARM_r2),
        [r3] "r" (current_thread->signal_kregs.ARM_r3),
        [r4] "r" (current_thread->signal_kregs.ARM_r4),
        [r5] "r" (current_thread->signal_kregs.ARM_r5),
        [r6] "r" (current_thread->signal_kregs.ARM_r6),
        [r7] "r" (current_thread->signal_kregs.ARM_r7),
        [r8] "r" (current_thread->signal_kregs.ARM_r8),
        [r9] "r" (current_thread->signal_kregs.ARM_r9),
        [r10] "r" (current_thread->signal_kregs.ARM_r10),
        [r11] "r" (current_thread->signal_kregs.ARM_r11),
        [r12] "r" (current_thread->signal_kregs.ARM_r12),
        [lr] "r" (current_thread->signal_kregs.ARM_lr),
        [pc] "r" (current_thread->signal_kregs.ARM_pc));
        printf("hehe dispatch system call.\n");
     /*
       l4_msgtag_t tag;
       vcpu->r.ip = current_thread->signal_kregs.ARM_pc;
       vcpu->r.sp = current_thread->signal_kregs.ARM_sp;
       vcpu->saved_state = current_thread->signal_kregs.ARM_cpsr;
       printf("test.\n");
       tag = l4_thread_vcpu_resume_start();
       l4_thread_vcpu_resume_commit(L4_INVALID_CAP, tag);
    */
    }
    else {
      printf("user mode page fault: pfa:0x%x, ip:0x%x\n", (unsigned int)(vcpu->r.pfa), (unsigned int)(vcpu->r.ip));
      printf("sp  0x%x\n", (unsigned int)(vcpu->r.sp));
      printf("ip  0x%x\n", (unsigned int)(vcpu->r.ip));
      printf("current thread 0x%x\n", (unsigned long)current_thread);
      enter_kdebug("user fault");
   }
  }

  else
  {
    if(vcpu->r.r[7] == L4ERTL_ARM_SYSCALL)
    {
      int nr = vcpu->r.r[0] >> 24;
      int op = vcpu->r.r[0] & (0xffff);

      pthread_t old_thread = current_thread;
      unsigned long r0 = current_thread->entry_uregs.ARM_r0;

      hw_sti();			//Allow that irq could be triggered
      dbprintf("start of system call: %s\n", syscall_names[op]);
      dispatch_syscalls(nr, op, &(old_thread->entry_uregs));
      dbprintf("end of system call 11: %s.\n", syscall_names[op]);
      asm volatile ("push {r2, r3}");
      do_signals();
      asm volatile ("pop {r2, r3}");
      //printf("end of system call: 22 %s.\n", syscall_names[op]);
      hw_cli();			//Forbid that irq could be triggered

      l4ertl_vcpu->r.r[0]  = current_thread->entry_uregs.ARM_r0;
      l4ertl_vcpu->r.r[1]  = current_thread->entry_uregs.ARM_r1;
      l4ertl_vcpu->r.r[2]  = current_thread->entry_uregs.ARM_r2;
      l4ertl_vcpu->r.r[3]  = current_thread->entry_uregs.ARM_r3;
      l4ertl_vcpu->r.r[4]  = current_thread->entry_uregs.ARM_r4;
      l4ertl_vcpu->r.r[5]  = current_thread->entry_uregs.ARM_r5;
      l4ertl_vcpu->r.r[6]  = current_thread->entry_uregs.ARM_r6;
      l4ertl_vcpu->r.r[7]  = current_thread->entry_uregs.ARM_r7;
      l4ertl_vcpu->r.r[8]  = current_thread->entry_uregs.ARM_r8;
      l4ertl_vcpu->r.r[9]  = current_thread->entry_uregs.ARM_r9;
      l4ertl_vcpu->r.r[10]  = current_thread->entry_uregs.ARM_r10;
      l4ertl_vcpu->r.r[11]  = current_thread->entry_uregs.ARM_r11;
      l4ertl_vcpu->r.r[12]  = current_thread->entry_uregs.ARM_r12;
      l4ertl_vcpu->r.sp  = current_thread->entry_uregs.ARM_sp;
      l4ertl_vcpu->r.lr  = current_thread->entry_uregs.ARM_lr;
      l4ertl_vcpu->r.ip  = current_thread->entry_uregs.ARM_pc;

      //ptregs_to_vcpu(vcpu, (struct pt_regs *)(&current_thread->entry_uregs));

      if(op != pthread_create_idle_nr && op != pthread_create_nr)
        l4ertl_vcpu->r.r[0] = r0;

    }

    else
    {
      printf("current thread 0x%x\n", (unsigned long)current_thread);
      printf("Undefined System call.\n");
      enter_kdebug();
    }
  }

  //printf("current thread 0x%x kentry_sp 0x%x\n", (unsigned long)current_thread, current_thread->kentry_sp);
  l4ertl_vcpu->entry_sp = current_thread->kentry_sp;
}

static void l4ertl_entry_from_kern(l4_vcpu_state_t *vcpu)
{
  if (l4vcpu_is_irq_entry(vcpu))
  {
    dprintf("hehe kernel irq %d 0x%x.\n", (int)(vcpu->i.label), (unsigned long)current_thread);
    if (vcpu->i.label != 0)
    {
       pthread_t old_thread = current_thread;

      context_t context;
      context.irqnr = vcpu->i.label;
      dbprintf("hehe kern do_irq %d current_thread 0x%x cpsr:0x%x\n", vcpu->i.label, (unsigned long)current_thread, current_thread->cpsr);
      do_irq(context);
      dbprintf("kern do_irq current_thread 0x%x cpsr:0x%x\n", (unsigned long)current_thread, current_thread->cpsr);
    }
    else
      enter_kdebug("vcpu kernel mode irq");
  }
  else if (l4vcpu_is_page_fault_entry(vcpu))
  {
    printf("kernel mode page fault: pfa:0x%x, ip:0x%x\n", (unsigned int)(vcpu->r.pfa), (unsigned int)(vcpu->r.ip));
    enter_kdebug("kernel fault");
  }
  else
  {
    if(vcpu->r.r[7] == L4ERTL_ARM_SYSCALL)
    {
      int nr = vcpu->r.r[0] >> 24;
      int op = vcpu->r.r[0] & (0xffff);
      unsigned long r0 = vcpu->r.r[0];

      pthread_t old_thread = current_thread;

      dbprintf("kernel start of system call: %s\n", syscall_names[op]);
      dispatch_syscalls(nr, op, &(old_thread->entry_uregs));

      if(old_thread != current_thread)
         pthread_to_vcpu(vcpu, current_thread);

      if(op != pthread_create_idle_nr && op != pthread_create_nr)
        vcpu->r.r[0] = r0;

    }
    else {
      printf("undefined system call from kernel.\n");
      printf("IP:0x%x\n", (unsigned long)(vcpu->r.ip));
    }
  }
}

/*

  entry (whatever from user space by system call triggered or from kernel space by irq triggered)
  set current_thread in kernel mode
  saved the vcpu state
















*/
void l4ertl_vcpu_entry_c(void);
void l4ertl_vcpu_entry_c(void)
{
   l4_msgtag_t tag;

   l4ertl_vcpu->state = 0;

   dbprintf("\nEnter kernel mode.\n");
   dbprintf("Thread:0x%x saved_cpsr:0x%x ip 0x%x lr: 0x%x sp: 0x%x stack: 0x%x\n", (unsigned long)current_thread, l4ertl_vcpu->saved_state, l4ertl_vcpu->r.ip, l4ertl_vcpu->r.lr, l4ertl_vcpu->r.sp, l4ertl_vcpu->entry_sp);
   current_thread->cpsr &= ~L4_VCPU_F_USER_MODE;   //set the current_thread in kernel mode
   current_thread->saved_cpsr = l4ertl_vcpu->saved_state;

   vcpu_to_pthread(l4ertl_vcpu, current_thread);  //sync the vcpu to current_thread

   //unsigned long sp;
   //asm volatile ("mov %0, sp":"=r"(sp));
   //printf("start current_thread sp:0x%x.\n", (unsigned long)sp);
   if(l4ertl_vcpu->saved_state & L4_VCPU_F_USER_MODE)
   {
     printf("start of user entry.\n");
     l4ertl_entry_from_user(l4ertl_vcpu);
     if(l4ertl_vcpu->saved_state & L4_VCPU_F_UNFINISHED_SIGNAL) {
       l4ertl_vcpu->saved_state = l4ertl_vcpu->saved_state & ~L4_VCPU_F_UNFINISHED_SIGNAL;
       //printf("start of user entry 0x%x.\n", (unsigned long)current_thread);
     }
     else {
       asm volatile ("push {r2, r3}");
       do_signals();
       asm volatile ("pop {r2, r3}");
       printf("22 start of user entry 0x%x.\n", (unsigned long)current_thread);
     }
/*
     asm volatile(
        "add r3, sp, #0xc	\t\n"
        "mov sp, r3		\t\n"
	:::"r3");
*/
     //asm volatile("mov %0, sp":"=r"(sp));
     //dprintf("end of user entry sp:0x%xcurrent_thread:0x%x.\n", (unsigned long)sp, (unsigned long)current_thread);
     current_thread->saved_cpsr |= L4_VCPU_F_USER_MODE;
     dbprintf("end of user entry.\n");
   }
   else
   {
     dbprintf("start of kernel entry.\n");
     l4ertl_entry_from_kern(l4ertl_vcpu);
     current_thread->saved_cpsr &= ~L4_VCPU_F_USER_MODE;
     dbprintf("end of kernel entry.\n");
   }

   dprintf("thread: 0x%x saved_cpsr: 0x%x ip back to user space 0x%x\n", (unsigned long)current_thread, current_thread->saved_cpsr, l4ertl_vcpu->r.lr);
#if 0
   if(current_thread->saved_cpsr & L4_VCPU_F_USER_MODE)
     l4ertl_vcpu->saved_state = L4_VCPU_F_USER_MODE;
   else
     l4ertl_vcpu->saved_state = 0;

   l4ertl_vcpu->saved_state |= L4_VCPU_F_IRQ
   			| L4_VCPU_F_EXCEPTIONS
			| L4_VCPU_F_PAGE_FAULTS
			| L4_VCPU_F_FPU_ENABLED;
#endif
   l4ertl_vcpu->saved_state = current_thread->saved_cpsr;


   //printf("current_thread->kernel_context_switch_happened 0x%x\n", current_thread->kernel_context_switch_happened);
   dprintf("Thread: 0x%x current_thread->kentry_sp 0x%x\n", (unsigned long)current_thread, current_thread->kentry_sp);
   /* needs the method like context_switch */

   pthread_to_vcpu(l4ertl_vcpu, current_thread);
   l4ertl_vcpu->entry_sp = current_thread->kentry_sp;
   dbprintf("Back to User Space thread:0x%x saved_state:0x%x ip 0x%x, lr 0x%x sp 0x%x entry_sp 0x%xk kentry_sp 0x%x\n", (unsigned long)current_thread, l4ertl_vcpu->saved_state, l4ertl_vcpu->r.ip, l4ertl_vcpu->r.lr, l4ertl_vcpu->r.sp, l4ertl_vcpu->entry_sp, current_thread->kentry_sp);

   tag = l4_thread_vcpu_resume_start();
   l4_thread_vcpu_resume_commit(L4_INVALID_CAP, tag);
}

void ret_may_switch(pthread_t new_thread, pthread_t old_thread)
{
   l4_msgtag_t tag;
   //printf("state 0x%x\n", l4ertl_vcpu->saved_state);

   if(old_thread != new_thread)
   {
     l4ertl_vcpu->state = 0;	//disable anything

     pthread_to_vcpu(l4ertl_vcpu, new_thread);

     l4ertl_vcpu->saved_state = L4_VCPU_F_IRQ
                        | L4_VCPU_F_EXCEPTIONS
                        | L4_VCPU_F_PAGE_FAULTS
                        | L4_VCPU_F_FPU_ENABLED ;

     if(new_thread->saved_cpsr & L4_VCPU_F_USER_MODE)
       l4ertl_vcpu->saved_state |= L4_VCPU_F_USER_MODE;
     else
       l4ertl_vcpu->saved_state &= (~L4_VCPU_F_USER_MODE);


     tag = l4_thread_vcpu_resume_start();
     l4_thread_vcpu_resume_commit(L4_INVALID_CAP, tag);
  }
}
