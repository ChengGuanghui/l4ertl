#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>


void handler (int s) {
/*  int sp;
  asm volatile ("mov %0, sp":"=r"(sp));
  printf("sp why 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", sp, *((unsigned long *)(sp)), *((unsigned long *)(sp + 4)), *((unsigned long *)(sp + 8)), *((unsigned long *)(sp + 12)), *((unsigned long *)(sp + 16)), *((unsigned long *)(sp + 20)), *((unsigned long *)(sp + 24)), *((unsigned long *)(sp + 28)));
*/
  printf ("Catching signal12345: %d\n",s);
  printf ("Catching signal6780: %d\n",s);
  printf ("Catching signal abcded: %d\n",s);
  printf ("Catching signal effffff: %d\n",s);
  printf ("Catching signalggggggggggggggg: %d\n",s);
}

void *thread_routine (void *args) {
  while (1) usleep (1);
  return 0;
}

int main (int argc, char **argv) {
	timer_t deadline;	
	struct timespec tp = {3,0};
	struct itimerspec it = {{5,0}, {2,0}};

	struct sigaction act;
	sigset_t set;

	sigemptyset(&set);
	pthread_sigmask (SIG_SETMASK, &set, 0);
	act.sa_handler = handler;
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGALARM);
	act.sa_flags = 0;
	sigaction (SIGALARM, &act, 0);

	timer_create (CLOCK_MONOTONIC, 0, &deadline);
	timer_settime(deadline, 0, &it,  0);
	nanosleep (&tp, 0);
	printf ("Hello\n");
	printf ("Hello\n");
	printf ("Hello\n");

        int i = 0;
	while (1);
	return 0;
}
