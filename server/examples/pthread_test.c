// Creating our first thread

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>
#include <hwirqs.h>


int main (int argc, char **argv) {
  int i = 0;
  struct timespec t = {1, 0};

  do {printf("HELLOO, WORLD i:%d.\n", i); nanosleep (&t, 0); printf("HELLO, WORLD i:%d.\n", i);i++;} while(i);
  
  printf ("Hello World\n");
  
  return 0;
}
