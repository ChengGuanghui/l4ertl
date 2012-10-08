#include <config.h>
#include <pthread.h>
#include <sched.h>
#include <stack.h>
#include <string.h>
#include <time.h>
#include <tlsf.h>
#include <sysmemory.h>
#include <l4/processor.h>

#include <l4/sys/kdebug.h>
#include <l4/sys/debugger.h>
//32 threads, every thread has a kern_stack with 8K

#define KERN_STACK_SIZE (8 * 1024)
#define MAX_KERN_THRADS (32)

static volatile unsigned long kstack[MAX_KERN_THRADS][KERN_STACK_SIZE];
static volatile unsigned long kstack_flag = 0;

void alloc_kstack (pthread_t thread) {

  int i;
  for (i = 0; i < MAX_KERN_THRADS; i++)
    if(!(kstack_flag & (1 << i)))
      break;

  if (i == MAX_KERN_THRADS) {
    printf("Threads number is already up to maximum limitation : 32. \n");
    enter_kdebug("KStack Fault\n");
  }

  thread->kentry_sp = (l4_umword_t)(kstack[i] + KERN_STACK_SIZE);
  kstack_flag |= (1 << i);
  //printf("thread 0x%x kstack 0x%x\n", (unsigned long)thread, (unsigned long)(thread->kentry_sp));
}

void free_kstack(pthread_t thread) {
  int i;
  for (i = 0; i < MAX_KERN_THRADS; i++)
    if((unsigned long)(thread->kentry_sp) > (unsigned long)kstack[i] &&
       (unsigned long)(thread->kentry_sp) <= (unsigned long)(kstack[i] + KERN_STACK_SIZE))
       break;

  if (i == MAX_KERN_THRADS) {
    printf("Can't find the thread 0x%x kernel stack. \n", (unsigned long)thread);
    enter_kdebug("KStack Fault\n");
  }

  kstack_flag &= ~(1 << i);
}

//-----------------//
// configure_stack //
//-----------------//
void configure_stack (unsigned long size) {
  //For L4, this can be ignored because this function is only used for precheck the availability of such memory size.
}

//-------------//
// alloc_stack //
//-------------//
//
//All the stacks of the thread are organized together as the continous memory
//In fact this function means to allocate an continuous memory from the stack->bottom to stack->top
int alloc_stack (stack_t *stack) { 
  if (!(stack -> stack_bottom =
     (unsigned long *) malloc_ex (stack -> stack_size, memory_pool)))
     return -1;

  return 0;

}

//---------------//
// dealloc_stack //
//---------------//
void dealloc_stack (stack_t *stack) { //free these memory size
   if (stack -> stack_bottom)
      free_ex (stack -> stack_bottom, memory_pool);

}

#if 0
//---------//
// startup //
//---------//
// First function executed when a new thread is created
static int startup (void *(*start_routine) (void *), void *args) {
//these two functions are same in the i386 and the xm_i386
//I could copy all the code here. But I must consider about how to encapsulate the L4 thread into the L4eRTL thread.
//Like L4Linux did

  void *retval;
  
  current_thread -> starting_time = monotonic_clock->gettime_nsec();
  //Initialising FPU
  //__asm__ __volatile__ ("finit\n\t" ::);
  printf("startup start_routine 0x%x\n", start_routine);
  printf("startup args 0x%x\n", args);
  hw_sti ();
  retval = (*start_routine) (args);
  pthread_exit_sys (retval);
  
  // This point will never be reached
  return 0;
}
#endif

//-------------//
// setup_stack //
//-------------//

unsigned long *setup_stack (unsigned long *stack,
  void *(*startup)(void *),
  void *(*start_routine) (void *), void *args) {

  unsigned long top = (unsigned long) stack;

  *--(stack) = top ; // sp
  *--(stack) = (unsigned long) start_routine; // r0
  *--(stack) = (unsigned long) args; //r1
  *--(stack) = 0; // r2
  *--(stack) = 0; // r3
  *--(stack) = 0; // r4
  *--(stack) = 0; // r5
  *--(stack) = 0; // r6
  *--(stack) = 0; // r7
  *--(stack) = 0; // r8
  *--(stack) = 0; // r9
  *--(stack) = 0; // r10
  *--(stack) = 0; // r11
  *--(stack) = 0; // r12
  *--(stack) = 0; // lr
  *--(stack) = L4_VCPU_F_USER_MODE		//cpsr
             | L4_VCPU_F_EXCEPTIONS
             | L4_VCPU_F_PAGE_FAULTS
             | L4_VCPU_F_IRQ
             | L4_VCPU_F_FPU_ENABLED;
  *--(stack) = (unsigned long) startup; // ip

  return stack;
}

//-------------//
// setup_stack2 //
//-------------//
//although setup_stack2 could work but it is very occassional
//Actually setup_stack2 is based on the stack_based value passed method
//setup_stack is based on the register based value passed method
unsigned long *setup_stack2 (unsigned long *stack,
  void *(*start_routine) (void *), void *args) {

  unsigned long top = (unsigned long) stack;

  *--(stack) = top; // sp
  *--(stack) = (unsigned long) args; // r0
  *--(stack) = 0; // r1
  *--(stack) = 0; // r2
  *--(stack) = 0; // r3
  *--(stack) = 0; // r4
  *--(stack) = 0; // r5
  *--(stack) = 0; // r6
  *--(stack) = 0; // r7
  *--(stack) = 0; // r8
  *--(stack) = 0; // r9
  *--(stack) = 0; // r10
  *--(stack) = 0; // r11
  *--(stack) = 0; // r12
  *--(stack) = 0; // lr
  *--(stack) = L4_VCPU_F_USER_MODE              //cpsr
             | L4_VCPU_F_EXCEPTIONS
             | L4_VCPU_F_PAGE_FAULTS
             | L4_VCPU_F_IRQ
             | L4_VCPU_F_FPU_ENABLED;

  *--(stack) = (unsigned long) start_routine; // ip
 
  return stack;
}
