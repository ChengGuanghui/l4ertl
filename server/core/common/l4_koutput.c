#include <koutput.h>

#include <l4/re/c/log.h>

//--------------//
// koutputlink //
//--------------//

int koutputlink (void) ;
int koutputlink (void) {
  return 0;
}

//--------------//
// print_kernel //
//--------------//

int print_kernel (const char *str, unsigned long length) {
  l4re_log_printn(str, length);
  return length;
}

