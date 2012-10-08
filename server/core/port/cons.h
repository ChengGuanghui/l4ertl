// interface functions for console i/o,
// these are defined for each arch and are implemented
// by arch dependend function calls.

#include <l4/arch_types.h>

extern int read_cons (int fd, void * str, size_t n);
extern int write_cons (int fd, const void * str, size_t n);
