/*
 * $FILE: signal.c
 *
 * Signals
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

/*
 * Fixed various bugs in signal handler mask - Vicent Brocal
 */
 
#include <config.h>
#include <bitop.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <arch/processor.h>

#include <l4/sys/thread.h>
#include <l4/sys/vcpu.h>

struct signal_table_struct signal_table [NRSIGNALS];

//--------------------//
// sa_handler_SIG_DFL //
//--------------------//

static void sa_handler_SIG_DFL (int signal) {
  /* 
   * Finishing with the whole application, the guilt is a signal
   */
  exit_sys (-1);
}

//--------------------//
// sa_handler_SIG_IGN //
//--------------------//

static void sa_handler_SIG_IGN (int signal) {
  /* Doing nothing */
}

//-----------------------//
// WAKEUP_THREAD_handler //
//-----------------------//

static void WAKEUP_THREAD_handler (int signal) {
  // When the thread receive one of this signal it automatically wakes
  // up so we have to do nothing
}

//----------------------//
// SLEEP_THREAD_handler //
//----------------------//

static void SLEEP_THREAD_handler (int signal) {
  suspend_thread (current_thread);
  scheduling ();
}

//-----------------------//
// CANCEL_THREAD_handler //
//-----------------------//

static void CANCEL_THREAD_handler (int signal) {
  // Canceling this thread
  pthread_exit_sys ((void *) 0);
}

//---------------//
// sigaction_sys //
//---------------//

asmlinkage int sigaction_sys (int sig, const struct sigaction *act,
			      struct sigaction *oact) {
  if (sig < FIRST_USER_SIGNAL || sig >= NRSIGNALS) {
    SET_ERRNO (EINTR);
    return -1;
  }

  if (oact) {
    if (signal_table [sig].sa_handler == sa_handler_SIG_DFL)
      oact -> sa_handler = SIG_DFL;
    else
      if (signal_table [sig].sa_handler == sa_handler_SIG_IGN)
	oact -> sa_handler = SIG_IGN;
      else
	oact -> sa_handler = signal_table [sig].sa_handler;
    oact -> sa_mask = signal_table [sig].sa_mask;
  }

  if (act) {
    if (act -> sa_handler == SIG_DFL) 
      signal_table [sig].sa_handler = sa_handler_SIG_DFL;
    else
      if (act -> sa_handler == SIG_IGN)
	signal_table [sig].sa_handler = sa_handler_SIG_IGN;
      else 
	signal_table [sig].sa_handler = act -> sa_handler;

    signal_table [sig].sa_mask = act -> sa_mask;
  }
  return 0;
}

//------------------//
// pthread_kill_sys //
//------------------//

asmlinkage int pthread_kill_sys (pthread_t thread, int sig) {
  if (!thread || thread -> magic_number != PTHREAD_MAGIC_NUMBER) {
    SET_ERRNO (ESRCH);
    return -1;
  }

  if (sig <  0 || sig >= NRSIGNALS) {
    SET_ERRNO (EINVAL);
    return -1;
  }
  
  thread -> sigpending.sig |= (1 << sig);
  /* 
   * If the signal isn't blocked, the thread is woken up
   */

  if ((~(thread -> sigmask.sig & ~non_maskable_signals) &
      thread -> sigpending.sig) || GET_THREAD_ACTIVATE_ON_SIGNAL(thread))
    activate_thread (thread);

  return 0;
}

//---------------------//
// pthread_sigmask_sys //
//---------------------//

asmlinkage int pthread_sigmask_sys (int how, const sigset_t *set, 
				    sigset_t *oset) {
  if (oset)
    oset -> sig = current_thread -> sigmask.sig;

  if (set)
    switch (how) {
    case SIG_BLOCK:
      current_thread -> sigmask.sig |= set -> sig;
      break;
    case SIG_SETMASK:
      current_thread -> sigmask.sig = set -> sig;
      break;
    case SIG_UNBLOCK:
      current_thread -> sigmask.sig &= ~(set -> sig);
      break;
    default:
      SET_ERRNO (EINVAL);
      return -1;
    }
  
  return 0;
}

//----------------//
// sigsuspend_sys //
//----------------//

asmlinkage int sigsuspend_sys (const sigset_t *mask) {
  sigset_t old_mask = current_thread -> sigmask;
  int flags;

  hw_save_flags_and_cli (flags);

  if (mask)
    current_thread -> sigmask = *mask;

  suspend_thread (current_thread);
  scheduling ();
  current_thread -> sigmask = old_mask;
  
  hw_restore_flags (flags);

  return 0;
}

//-------------//
// sigwait_sys //
//-------------//

asmlinkage int sigwait_sys (const sigset_t *set, int *sig) {
  int flags, pending;

  if (!set) {
    SET_ERRNO (EINVAL);
    return -1;
  }
  
  hw_save_flags_and_cli (flags);
  pending = (current_thread -> sigpending.sig & 
	     (set -> sig & ~non_maskable_signals));
  if (pending) {
    pending = _ffs (pending);
    if (sig)
      *sig = pending;
    current_thread -> sigpending.sig &= ~(1<<pending);
    hw_restore_flags (flags);
    return 0;
  }
  
  SET_THREAD_ACTIVATE_ON_SIGNAL(current_thread, 1);
  sigsuspend_sys (set);
  SET_THREAD_ACTIVATE_ON_SIGNAL(current_thread, 0);
  pending = (current_thread -> sigpending.sig & 
	     (set -> sig & ~non_maskable_signals));
  if (pending) {
    pending = _ffs (pending);
    if (sig)
      *sig = pending;
    current_thread -> sigpending.sig &= ~(1<<pending);
  } else {
    hw_restore_flags (flags);
    SET_ERRNO (EINVAL);
    return -1;
  }

  hw_restore_flags (flags);

  return 0;
}

//----------------//
// sigpending_sys //
//----------------//

asmlinkage int sigpending_sys (sigset_t *set) {
  if (set) *set = current_thread -> sigpending;

  return 0;
}

//--------------//
// init_signals //
//--------------//

int init_signals (void) {
  int signal;
  memset ((char *)&signal_table, 0, 
	  sizeof (struct signal_table_struct) * NRSIGNALS);
  
  for (signal = 0; signal < NRSIGNALS; signal ++) {
    signal_table [signal].sa_handler = sa_handler_SIG_IGN;
    sigaddset (&signal_table [signal].sa_mask, signal);
  }

  signal_table [CANCEL_SIGNAL].sa_handler = CANCEL_THREAD_handler;
  signal_table [SLEEP_SIGNAL].sa_handler = SLEEP_THREAD_handler;
  signal_table [WAKEUP_SIGNAL].sa_handler = WAKEUP_THREAD_handler;
  return 0;
}

//---------------//
// do_do_signals //
//---------------//

void do_do_signals(int signal, unsigned int func) {
  l4_msgtag_t tag;
  printf("hoho do_do_signals.\n");
  //pthread_to_vcpu(l4ertl_vcpu, current_thread);
  current_thread->signal_kregs.ARM_pc = __builtin_return_address(0);
  current_thread->signal_kregs.ARM_cpsr = l4ertl_vcpu->saved_state;
  printf("signal before sp 0x%x\n", (unsigned long)(current_thread->signal_kregs.ARM_sp));
  printf("signal ip 0x%x\n", (unsigned long)(current_thread->signal_kregs.ARM_pc));
  l4ertl_vcpu->r.r[0] = (unsigned long)signal;
  l4ertl_vcpu->r.lr = 0xFEDCBA00;
  l4ertl_vcpu->r.ip = func;
  //l4ertl_vcpu->r.sp = current_thread->signal_kregs.ARM_sp;
  l4ertl_vcpu->r.sp = current_thread->entry_uregs.ARM_sp + 512;
  l4ertl_vcpu->entry_sp = current_thread->kentry_sp;
  l4ertl_vcpu->saved_state = L4_VCPU_F_USER_MODE
             | L4_VCPU_F_UNFINISHED_SIGNAL
  //           | L4_VCPU_F_IRQ
             | L4_VCPU_F_EXCEPTIONS
             | L4_VCPU_F_PAGE_FAULTS
             | L4_VCPU_F_FPU_ENABLED
             | L4_VCPU_F_HANDLING_SIGNAL;
  printf("switch to the user space.\n");
  tag = l4_thread_vcpu_resume_start();
  l4_thread_vcpu_resume_commit(L4_INVALID_CAP, tag);
}

//------------//
// do_signals //
//------------//

void do_signals (void) {
  int flags, signal, p1 = 0, p2 = 0;
  unsigned long sigpend, old_sigmask;

  hw_save_flags_and_cli (flags);

  /*
   * Executing pending signals
   */
  sigpend = (current_thread -> sigpending.sig 
	     & ~(current_thread -> sigmask.sig & ~non_maskable_signals));

  if (sigpend || current_thread -> sigpending.sig)
    current_thread -> sigpending.sig &= ~sigpend;

  /* 
   * Looking at the current thread canceability state
   */
  if (GET_THREAD_CANCEL_STATE(current_thread)) {
    current_thread -> sigpending.sig |= (sigpend & (1 << CANCEL_SIGNAL));
    sigpend &= ~(1 << CANCEL_SIGNAL);
  }

  old_sigmask = current_thread -> sigmask.sig;

  //current_thread->signal_kregs.ARM_lr = 0;
  for (signal = 0; signal < NRSIGNALS && sigpend; signal ++) {
    current_thread -> sigmask.sig = old_sigmask | signal_table [signal].sa_mask.sig | (1 << signal);
    if ((sigpend & 1)) {
      hw_sti ();				//I don't know why it is triggered in this place  ??????????????????????????
      // Excuting the handler associated with this signal
      printf("sa_sigaction 0x%x\n", (unsigned long)signal_table [signal].sa_sigaction);

#if 0
      if ((unsigned long)signal_table [signal].sa_sigaction > 
	  (unsigned long)&_stext && 
	  (unsigned long)signal_table [signal].sa_sigaction < 
	  (unsigned long)&_etext) {
	// internal handler
	if (signal_table [signal].sa_sigaction)
	  signal_table [signal].sa_sigaction (signal, 
					      (siginfo_t *) p1, (void *) p2);
      } else {
	// external handler
	if (signal_table [signal].sa_sigaction) {
	  CALL_SIGNAL_HANDLER 
	    (signal_table [signal].sa_sigaction, signal, 
	     (siginfo_t *)p1, (void *)p2);
	}
      }
#else
	if (signal_table [signal].sa_handler) {
           asm volatile ("mov %0, sp":"=r"((current_thread->signal_kregs.ARM_sp)));
           asm volatile ("mov %0, r0":"=r"((current_thread->signal_kregs.ARM_r0)));
           asm volatile ("mov %0, r1":"=r"((current_thread->signal_kregs.ARM_r1)));
           asm volatile ("mov %0, r2":"=r"((current_thread->signal_kregs.ARM_r2)));
           asm volatile ("mov %0, r3":"=r"((current_thread->signal_kregs.ARM_r3)));
           asm volatile ("mov %0, r4":"=r"((current_thread->signal_kregs.ARM_r4)));
           asm volatile ("mov %0, r5":"=r"((current_thread->signal_kregs.ARM_r5)));
           asm volatile ("mov %0, r6":"=r"((current_thread->signal_kregs.ARM_r6)));
           asm volatile ("mov %0, r7":"=r"((current_thread->signal_kregs.ARM_r7)));
           asm volatile ("mov %0, r8":"=r"((current_thread->signal_kregs.ARM_r8)));
           asm volatile ("mov %0, r9":"=r"((current_thread->signal_kregs.ARM_r9)));
           asm volatile ("mov %0, r10":"=r"((current_thread->signal_kregs.ARM_r10)));
           asm volatile ("mov %0, r11":"=r"((current_thread->signal_kregs.ARM_r11)));
           asm volatile ("mov %0, r12":"=r"((current_thread->signal_kregs.ARM_r12)));
           //asm volatile ("mov %0, lr":"=r"((current_thread->signal_kregs.ARM_lr)));
           current_thread->signal_kregs.ARM_lr = __builtin_return_address(0);
           //printf("current_thread->signal_kregs.ARM_lr 0x%x\n", current_thread->signal_kregs.ARM_lr);
/*
           printf("sp 0x%x\n", current_thread->signal_kregs.ARM_sp);
           printf("r0 0x%x\n", current_thread->signal_kregs.ARM_r0);
           printf("r1 0x%x\n", current_thread->signal_kregs.ARM_r1);
           printf("r2 0x%x\n", current_thread->signal_kregs.ARM_r2);
           printf("r3 0x%x\n", current_thread->signal_kregs.ARM_r3);
           printf("r4 0x%x\n", current_thread->signal_kregs.ARM_r4);
           printf("r5 0x%x\n", current_thread->signal_kregs.ARM_r5);
           printf("r6 0x%x\n", current_thread->signal_kregs.ARM_r6);
           printf("r7 0x%x\n", current_thread->signal_kregs.ARM_r7);
           printf("r8 0x%x\n", current_thread->signal_kregs.ARM_r8);
           printf("r9 0x%x\n", current_thread->signal_kregs.ARM_r9);
           printf("r10 0x%x\n", current_thread->signal_kregs.ARM_r10);
           printf("r11 0x%x\n", current_thread->signal_kregs.ARM_r11);
           printf("r12 0x%x\n", current_thread->signal_kregs.ARM_r12);
           printf("lr 0x%x\n", current_thread->signal_kregs.ARM_lr);
           printf("before sp 0x%x lr real 0x%x fault 0x%x\n", current_thread->signal_kregs.ARM_sp, __builtin_return_address(0), current_thread->signal_kregs.ARM_lr);
*/           //printf("before SP 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", current_thread->signal_kregs.ARM_sp, *((unsigned long *)(current_thread->signal_kregs.ARM_sp)), *((unsigned long *)(current_thread->signal_kregs.ARM_sp + 4)), *((unsigned long *)(current_thread->signal_kregs.ARM_sp + 8)), *((unsigned long *)(current_thread->signal_kregs.ARM_sp + 12)), *((unsigned long *)(current_thread->signal_kregs.ARM_sp + 16)), *((unsigned long *)(current_thread->signal_kregs.ARM_sp + 20)), *((unsigned long *)(current_thread->signal_kregs.ARM_sp + 24)), *((unsigned long *)(current_thread->signal_kregs.ARM_sp + 28)));
           do_do_signals(signal, (unsigned long)(signal_table [signal].sa_handler));
	   printf("hoho.\n");
           l4ertl_vcpu->saved_state = l4ertl_vcpu->saved_state & ~L4_VCPU_F_HANDLING_SIGNAL;
           //l4ertl_vcpu->saved_state = l4ertl_vcpu->saved_state | L4_VCPU_F_IRQ;
           if(l4ertl_vcpu->saved_state & L4_VCPU_F_HANDLING_SIGNAL)
               printf("SIGNAL is in handling 21.\n");
             else
               printf("SIGNAL is in handling 22.\n");

  //printf("original lr 0x%x\n", current_thread->signal_kregs.ARM_lr);
	   //printf("hihi.\n");
           }
     //printf("haha.\n");
     //while(1);

#endif
      hw_cli ();
      printf("hphp.\n");
    }
    sigpend >>= 1;
  }

  //printf("1 why\n");
  //printf("3 why\n");
  //printf("4 why\n");
  current_thread -> sigmask.sig = old_sigmask;
  printf("2 why\n");
  hw_restore_flags (flags);
/*  unsigned int sp;
  asm volatile ("mov %0, sp":"=r"(sp));
  printf("lr 0x%x\n", __builtin_return_address(0));
*/
  if(!(l4ertl_vcpu->saved_state & L4_VCPU_F_HANDLING_SIGNAL) && (l4ertl_vcpu->saved_state & L4_VCPU_F_UNFINISHED_SIGNAL)) {
    printf("current_thread 0x%x\n", (unsigned long)current_thread);
    printf("sp 0x%x\n", current_thread->signal_kregs.ARM_sp);
    printf("pc 0x%x\n", current_thread->signal_kregs.ARM_pc);
    printf("lr 0x%x\n", current_thread->signal_kregs.ARM_lr);
/*    printf("fp 0x%x\n", current_thread->signal_kregs.ARM_r10);
    printf("r9 0x%x\n", current_thread->signal_kregs.ARM_r9);
    printf("r8 0x%x\n", current_thread->signal_kregs.ARM_r8);
    printf("r7 0x%x\n", current_thread->signal_kregs.ARM_r7);
    printf("r6 0x%x\n", current_thread->signal_kregs.ARM_r6);
    printf("r5 0x%x\n", current_thread->signal_kregs.ARM_r5);
    printf("r4 0x%x\n", current_thread->signal_kregs.ARM_r4);
*/
    l4ertl_vcpu->saved_state = l4ertl_vcpu->saved_state & (~L4_VCPU_F_UNFINISHED_SIGNAL);

    asm volatile (
      "pop {r4, r5, r6, r7, r8, r9, sl, fp, lr}		\n\t"
      "mov r4, %[newr4]					\n\t"
      "mov r5, %[newr5]					\n\t"
      "mov r6, %[newr6]					\n\t"
      "mov r7, %[newr7]					\n\t"
      "mov r8, %[newr8]					\n\t"
      "mov r9, %[newr9]					\n\t"
      "mov sl, %[newr10]				\n\t"
      "mov fp, %[newr11]				\n\t"
      "mov lr, %[newlr]					\n\t"
      "mov pc, lr					\n\t"
      ::[newlr]"r"(current_thread->signal_kregs.ARM_lr),
       [newr11]"r"(current_thread->signal_kregs.ARM_r11),
       [newr10]"r"(current_thread->signal_kregs.ARM_r10),
       [newr9]"r"(current_thread->signal_kregs.ARM_r9),
       [newr8]"r"(current_thread->signal_kregs.ARM_r8),
       [newr7]"r"(current_thread->signal_kregs.ARM_r7),
       [newr6]"r"(current_thread->signal_kregs.ARM_r6),
       [newr5]"r"(current_thread->signal_kregs.ARM_r5),
       [newr4]"r"(current_thread->signal_kregs.ARM_r4));

  }
  printf("end of do signal.\n");
}
