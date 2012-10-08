#include "../port/cons.h"

#include <l4/re/c/log.h>

#include <string.h>
#include <stdio.h>

// todo complete read_cons
int
read_cons(int fd, void * str, size_t n){ 
	//Not supportd yet
	return 0;
}

int
write_cons(int fd, const void * str, size_t n){
    char *c = (char *)str;
    l4re_log_printn(c, n);
    if(n < strlen(c))
      return n;
    else
      return strlen(c);
}

