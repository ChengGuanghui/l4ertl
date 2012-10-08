PKGDIR	?= ../..
L4DIR	?= $(PKGDIR)/../..

SRCS	= $(sort $(wildcard *.c))

OBJS	= $(shell echo $(SRCS) | sed 's|\.[cS]|.o|g')

all:  start.o

#CFLAGS	= -nostdinc -nostdlib -Wall -O2 -D__ASSEMBLY__  -I$(PKGDIR)/include/ertl_user
CFLAGS	= -nostdinc -nostdlib -Wall -g -O2 -fno-strict-aliasing  -march=armv4 -Wextra -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations  -fno-common -fno-stack-protector -I$(PKGDIR)/include/ertl_user
RM	= rm
CC	= arm-none-linux-gnueabi-gcc
LD	= arm-none-linux-gnueabi-ld
AR	= arm-none-linux-gnueabi-ar
OBJDUMP	= arm-none-linux-gnueabi-objdump

start.o: start.c
	$(CC) -Wall $(CFLAGS)  -c -o $@ $<
clean:
	$(Q)$(RM) $(OBJS)

distclean:
	$(Q)$(RM) $(OBJS)
