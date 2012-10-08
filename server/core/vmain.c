/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Lukas Grützmacher <lg2@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/c/mem_alloc.h>
#include <l4/re/c/rm.h>
#include <l4/re/c/log.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/util/kumem_alloc.h>
#include <l4/re/env.h>
#include <l4/util/util.h>
#include <l4/vcpu/vcpu.h>

#include <l4/sys/kdebug.h>
#include <l4/sys/debugger.h>
#include <l4/sys/factory.h>
#include <l4/sys/thread.h>
#include <l4/sys/ipc.h>
#include <l4/sys/scheduler.h>
#include <l4/sys/task.h>
#include <l4/sys/irq.h>
#include <l4/sys/utcb.h>

//#include <l4/processor.h>
//#include <l4/irqs.h>
//#include <syscall-test.h>
//#include <syscalls.h>

#include <stdio.h>
#include <string.h>

static char thread_stack[8 << 10];
static char hdl_stack[8 << 10];

void * vcpu_text_vbase;
void * vcpu_stack_vbase;
void * vcpu_vmm_vbase;

#define VCPU_TEXT_LOG2_SIZE 23	// 1 << 23; 8M
#define VCPU_STACK_LOG2_SIZE 16	// 1 << 16; 64K
#define VCPU_VMM_LOG2_SIZE 23 	// 1 << 23; 8M

unsigned long vcpu_text_size = 1 << VCPU_TEXT_LOG2_SIZE;
unsigned long vcpu_stack_size = 1 << VCPU_STACK_LOG2_SIZE;
unsigned long vcpu_vmm_size = 1 << VCPU_VMM_LOG2_SIZE;

const l4_addr_t super_code_map_addr;// = 0x80000000;
const l4_addr_t super_code_stack_addr;// = 0x90000000;

l4_vcpu_state_t *l4ertl_vcpu;

l4_cap_idx_t vcpu_task;
l4_cap_idx_t vcpu_cap;

static void user_vcpu_user_arch(void){}

void l4ertl_vcpu_entry(void);
extern void l4ertl_vcpu_entry_c(void);

asm(
".global l4ertl_vcpu_entry \n\t"
"l4ertl_vcpu_entry: \n\t"
"       bic   sp, sp, #7\n\t"
"       b     l4ertl_vcpu_entry_c \n\t"
);

extern int setup_kernel(void);
static void vcpu_thread(void)
{
  l4_msgtag_t tag;
  printf("Hello vCPU\n");

  setup_kernel();

  //wait for timer irq and other hardware irq initialization finished
  l4_usleep(10000);

  tag = l4_task_map(vcpu_task, L4_BASE_TASK_CAP, l4_fpage((l4_addr_t)vcpu_text_vbase, VCPU_TEXT_LOG2_SIZE, L4_FPAGE_RWX), (l4_addr_t)vcpu_text_vbase);
  if(l4_error(tag))
     printf("Memory Text Mapping error. %s\n", l4sys_errtostr(l4_error(tag)));
  else
     printf("Memory Text Mapping at 0x%x size 0x%x\n", (unsigned int)vcpu_text_vbase, (unsigned int)vcpu_text_size);
  tag = l4_task_map(vcpu_task, L4_BASE_TASK_CAP, l4_fpage((l4_addr_t)vcpu_stack_vbase, VCPU_STACK_LOG2_SIZE, L4_FPAGE_RWX), (l4_addr_t)(vcpu_stack_vbase));
  if(l4_error(tag))
     printf("Stack Mapping error. %s\n", l4sys_errtostr(l4_error(tag)));
  else
     printf("Stack Mapping at 0x%x size 0x%x\n", (unsigned int)vcpu_stack_vbase, (unsigned int)vcpu_stack_size);

  tag = l4_task_map(vcpu_task, L4_BASE_TASK_CAP, l4_fpage((l4_addr_t)vcpu_vmm_vbase, VCPU_VMM_LOG2_SIZE, L4_FPAGE_RWX), (l4_addr_t)(vcpu_vmm_vbase));
  if(l4_error(tag))
     printf("VMM Mapping error. %s\n", l4sys_errtostr(l4_error(tag)));
  else
     printf("VMM Mapping at 0x%x size 0x%x\n", (unsigned int)vcpu_vmm_vbase, (unsigned int)vcpu_vmm_size);

  l4_touch_rw(hdl_stack, sizeof(hdl_stack));
  memset(hdl_stack, 0, sizeof(hdl_stack));
 
  l4ertl_vcpu->saved_state = L4_VCPU_F_USER_MODE
                           | L4_VCPU_F_EXCEPTIONS
                           | L4_VCPU_F_PAGE_FAULTS
                           | L4_VCPU_F_IRQ
		           | L4_VCPU_F_FPU_ENABLED ;
  l4ertl_vcpu->r.ip = (l4_umword_t)vcpu_text_vbase;

  l4ertl_vcpu->r.sp = (l4_umword_t)(vcpu_stack_vbase + vcpu_stack_size);
                           // stack memory in our code
  l4ertl_vcpu->user_task = vcpu_task;

  printf("VCPU entry_sp 0x%x entry_ip 0x%x\n", (l4_umword_t)(hdl_stack + sizeof(hdl_stack)), (l4_umword_t)l4ertl_vcpu_entry);
  l4ertl_vcpu->entry_sp = (l4_umword_t)(hdl_stack + sizeof(hdl_stack));
  l4ertl_vcpu->entry_ip = (l4_umword_t)l4ertl_vcpu_entry;
  
  l4_thread_vcpu_resume_commit(L4_INVALID_CAP, l4_thread_vcpu_resume_start());

  printf("IRET: failed!\n");
  while (1);
}

static int allocate_memory(void ** virt_addr, unsigned long virt_base, 
	unsigned long size_in_bytes, unsigned long flags)
{
  int r;
  l4re_ds_t ds;

  /* Allocate a free capability index for our data space */
  ds = l4re_util_cap_alloc();
  if (l4_is_invalid_cap(ds))
    return -L4_ENOMEM;

  /* Allocate memory via a dataspace */
  if ((r = l4re_ma_alloc(size_in_bytes, ds, flags)))
  {
    printf("Memory allocation failed.\n");
    return r;
  }

  /* Make the dataspace visible in our address space */
  *(unsigned long *)virt_addr = virt_base;
  if ((r = l4re_rm_attach(virt_addr, size_in_bytes,
     L4RE_RM_SEARCH_ADDR, ds, 0, L4_PAGESHIFT)))
  {
    printf("Memory mapping failed.\n");
    return r;
  }

  /* Done, virtual address is in virt_addr */
  return 0;
}

static int init_vcpu_memory(unsigned long vbase)
{
  int r;

  if((r = allocate_memory(&vcpu_text_vbase, vbase, vcpu_text_size, L4RE_MA_PINNED)) > 0)
  {
     printf("Text Memory Allocation failed.\n");
     return r;
  }
  memset(vcpu_text_vbase, 0, vcpu_text_size);
  printf("VCPU Text memory init: \n");
  printf("       Staring from: 0x%x end: 0x%x.\n", (unsigned int)vcpu_text_vbase, (unsigned int)(vcpu_text_vbase + vcpu_text_size));

  vbase = vbase + 0x10000000 - vcpu_stack_size;
  if((r = allocate_memory(&vcpu_stack_vbase, vbase, vcpu_stack_size, L4RE_MA_CONTINUOUS | L4RE_MA_PINNED)) > 0)
  {
     printf("Text Memory Allocation failed.\n");
     return r;
  }
  memset(vcpu_stack_vbase, 0, vcpu_stack_size);
  printf("VCPU Stack memory init: \n");
  printf("       Starting from: 0x%x end: 0x%x\n", (unsigned int)vcpu_stack_vbase, (unsigned int)(vcpu_stack_vbase + vcpu_stack_size));

  return 0;
}

int elf_loader(char *);

int
main(int argc, char ** argv)
{
  unsigned long user_entry;

  user_entry = 0x90000000;

  if(init_vcpu_memory(user_entry)) {
    printf("Init VCPU Memory failed.\n");
    return 1;
  }

  if(!(elf_loader(vcpu_text_vbase))) {
    printf("Loading ELF failed.\n");
    return 1;
  }

  long err;

  l4re_log_print("VCPU Example.\n");

  l4_debugger_set_object_name(l4re_env()->main_thread, "vcputest");

  if(l4_is_invalid_cap(vcpu_task = l4re_util_cap_alloc()))
  {
    printf("Invalid cap allocation in the Line %d, Function: %s.\n", __LINE__, __func__);
    return 1;
  }

  if((err = l4_error(l4_factory_create_task(l4re_env()->factory, vcpu_task, l4_fpage_invalid()))) > 0) {
    printf("Failed: %s \n", l4sys_errtostr(err));
    return err;
  }

  l4_debugger_set_object_name(vcpu_task, "vcpu user task");

  if(l4_is_invalid_cap(vcpu_cap = l4re_util_cap_alloc()))
  {
    printf("Invalid cap allocation in the Line %d, Function: %s.\n", __LINE__, __func__);
    return 1;
  }

  if((err = l4_error(l4_factory_create_thread(l4re_env()->factory, vcpu_cap))) > 0) {
    printf("Failed: %s \n", l4sys_errtostr(err));
    return err;
  }

  l4_debugger_set_object_name(vcpu_cap, "vcpu thread");
  
  l4_touch_rw(thread_stack, sizeof(thread_stack)); 
   
  memset(thread_stack, 0, sizeof(thread_stack));
  
  //get memory for vCPU state
  l4_addr_t kumem;
  l4re_util_kumem_alloc(&kumem, 0, L4_BASE_TASK_CAP, l4re_env()->rm);
  l4_utcb_t *vcpu_utcb = (l4_utcb_t *)kumem;
  l4ertl_vcpu = (l4_vcpu_state_t *)(kumem + L4_UTCB_OFFSET);

  printf("VCPU: utcb = %p, vcpu = %p id = %d\n", vcpu_utcb, l4ertl_vcpu, l4_debugger_global_id(vcpu_cap));

  //Create and start vCPU thread
  l4_thread_control_start();
  l4_thread_control_pager(l4re_env()->rm);
  l4_thread_control_exc_handler(l4re_env()->main_thread);
  //l4_thread_control_exc_handler(l4re_env()->rm);
  l4_thread_control_bind(vcpu_utcb, L4RE_THIS_TASK_CAP);
  if((err = l4_error(l4_thread_control_commit(vcpu_cap))) > 0) {
    printf("Failed: %s in line %d in Function %s\n", l4sys_errtostr(err), __LINE__, __func__);
    return err;
  }

  l4_thread_vcpu_control(vcpu_cap, (l4_addr_t)l4ertl_vcpu);
  l4_thread_ex_regs(vcpu_cap, (l4_umword_t)vcpu_thread, (l4_umword_t)thread_stack + sizeof(thread_stack), 0);

  //0xff is the default priority of l4 thread == lowest priority
  l4_sched_param_t sp = l4_sched_param(0x2, 0);
  if((err = l4_error(l4_scheduler_run_thread(l4re_env()->scheduler, vcpu_cap, &sp))) > 0) {
    printf("Failed: %s in line %d in Function %s\n", l4sys_errtostr(err), __LINE__, __func__);
    return err;
  }
  
  l4_sleep_forever();
#if 0
  l4_cap_idx_t irq;
  l4_umword_t label;
  l4_usleep(200000);
  while(1)
  {
     l4_irq_wait(irq, &label, L4_IPC_NEVER);
     printf("irq is getting %d.\n", label >> 2);
  }
#endif 
  return 0;
}
