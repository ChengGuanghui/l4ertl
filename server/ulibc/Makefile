PKGDIR	?= ../..
L4DIR	?= $(PKGDIR)/../..

SRCS	= $(sort $(wildcard *.c))
#assert.c  
#io.c      math.c     rtl.c       # signal.c  stdlib.c  tlsf.c	\
#	errno.c   irqs.c    mman.c     sched.c      stdarg.c  string.c  user_entry_point.c	\
#	errstr.c  libgen.c  
#pthread.c  
#semaphore.c  
#stdio.c   
#time.c

OBJS	= $(shell echo $(SRCS) | sed 's|\.[cS]|.o|g')

all: libulibc.a

#CFLAGS	= -nostdinc -nostdlib -Wall -O2 -D__ASSEMBLY__  -I$(PKGDIR)/include/ertl_user
#CFLAGS	= -nostdinc -nostdlib -Wall -O2 -fno-common -fno-strict-aliasing -fno-builtin -fexceptions  -I$(PKGDIR)/include/ertl_user
#CFLAGS	= -nostdinc -nostdlib -Wall -g -O2 -fno-strict-aliasing  -march=armv4 -Wextra -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations  -fno-common -fno-stack-protector -I$(PKGDIR)/include/ertl_user
CFLAGS	= -nostdinc -nostdlib -Wall -g  -march=armv4 -Wextra -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations  -fno-common -fno-stack-protector -I$(PKGDIR)/include/ertl_user
RM	= rm
CC	= arm-none-linux-gnueabi-gcc
LD	= arm-none-linux-gnueabi-ld
AR	= arm-none-linux-gnueabi-ar
OBJDUMP	= arm-none-linux-gnueabi-objdump

libulibc.a: $(OBJS)
	$(Q)$(AR) -crs libulibc.a $(OBJS) $(Q2)
clean:
	$(Q)$(RM) $(OBJS)

distclean:
	$(Q)$(RM) -f libulibc.a $(OBJS)
