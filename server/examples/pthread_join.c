// Creating our first thread

#include <stdio.h>
#include <time.h>
#include <pthread.h>


static pthread_t t1;

//static int c1 = 0;

void *f(void *args) {
  struct timespec t = {1, 0};

  printf("new thread nano sleep1 0x%x.\n", (unsigned long)t1);
  nanosleep (&t, 0);
 
  return (void *)23;
}

int main (int argc, char **argv) {
  int ret_v;

  pthread_create (&t1, 0, f, 0);

  pthread_join (t1, (void *) &ret_v);

  printf ("Returned value: %d\n", ret_v);

  pthread_exit ((void *)0);
  return 0;
}
