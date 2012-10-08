#ifndef _L4_SYSCALL_TEST__
#define _L4_SYSCALL_TEST__

#include <syscalls.h>

char syscall_names[][NR_SYSCALLS] = {
  "exit_nr",				//0
  "open_nr",
  "close_nr",
  "read_nr",
  "write_nr",
  "lseek_nr",				//5
  "ioctl_nr",
  "mmap_nr",
  "munmap_nr",
  "pthread_exit_nr",
  "pthread_create_nr",			//10
  "pthread_join_nr",
  "pthread_detach_nr",
  "pthread_cleanup_pop_nr",
  "pthread_cleanup_push_nr",
  "pthread_once_nr",			//15
  "pthread_cancel_nr",
  "pthread_setcancelstate_nr",
  "pthread_setcanceltype_nr",
  "pthread_self_nr",
  "pthread_getschedparam_nr",		//20
  "pthread_setschedparam_nr",
  "pthread_mutex_init_nr",
  "pthread_mutex_destroy_nr",
  "pthread_mutex_lock_nr",
  "pthread_mutex_timedlock_nr",		//25
  "pthread_mutex_trylock_nr",
  "pthread_mutex_unlock_nr",
  "pthread_cond_init_nr",
  "pthread_cond_destroy_nr",
  "pthread_cond_signal_nr",		//30
  "pthread_cond_broadcast_nr",
  "pthread_cond_wait_nr",
  "pthread_cond_timedwait_nr",
  "pthread_key_create_nr",
  "pthread_setspecific_nr",		//35
  "pthread_getspecific_nr",
  "pthread_key_delete_nr",
  "pthread_yield_nr",
  "time_nr",
  "gettimeofday_nr",			//40
  "usleep_nr",
  "nanosleep_nr",
  "clock_settime_nr",
  "clock_gettime_nr",
  "clock_getres_nr",			//45
  "sigaction_nr",
  "sigsuspend_nr",
  "sigwait_nr",
  "sigpending_nr",
  "pthread_kill_nr",			//50
  "pthread_sigmask_nr",
  "_get_errno_nr",
  "_set_errno_nr",
  "install_irq_handler_nr",
  "install_trap_handler_nr",		//55
  "hw_disable_irq_nr",
  "hw_enable_irq_nr",
  "hw_ack_irq_nr",
  "hw_end_irq_nr",
  "hw_cli_nr",			//60
  "hw_sti_nr",
  "hw_restore_flags_nr",
  "hw_save_flags_nr",
  "hw_save_flags_and_cli_nr",
  "ualloc_nr",				//65
  "ufree_nr",
  "sem_init_nr",
  "sem_destroy_nr",
  "sem_getvalue_nr",
  "sem_wait_nr",			//70
  "sem_trywait_nr",
  "sem_timedwait_nr",
  "sem_post_nr",
  "pthread_delete_np_nr",
  "pthread_setperiod_np_nr",		//75
  "pthread_getperiod_np_nr",
  "iopl_nr",
  "timer_create_nr",
  "timer_delete_nr",
  "timer_settime_nr",			//80
  "timer_gettime_nr",
  "timer_getoverrun_nr",
  "pthread_create_idle_nr",
  "pthread_create2_nr"
};

#endif
