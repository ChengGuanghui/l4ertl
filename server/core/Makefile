PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= $(PKGNAME)

CONF		= prtkcf
ROOT		= port/$(CONF).c port/$(CONF).root.c

KERNDATE:=$(shell date '+%s')

# list your .c or .cc files here
LIBC		=  libc/stdarg.c libc/stdio.c libc/stdlib.c libc/string.c libc/time.c libc/tlsf.c libc/vfprintf.c libc/vsprintf.c
L4_COMMON	= common/l4_koutput.c common/l4_processor.c common/l4_panic.c common/l4_timers.c common/cons-l4.c \
		common/irqs-l4.c common/l4_sysmem.c common/l4_icu.c common/thread.c
PORT		= port/irqs.c port/sched.c port/setup.c port/time.c port/errno.c port/heap.c port/signal.c port/pthread.c port/devcons.c \
		port/devroot.c port/devtest.c port/dev.c port/semaphore.c port/rtl.c port/syscalls.c port/fs.c 

SPECS		= arm/arm_context_switch.c arm/arm_l4_stack.c arm/dispatch_syscalls.c

#MACH		= common/arm_versatile_vexpressa9_pit.c
MACH		= common/arm_omap4_pit.c
#MACH		= common/l4_pit.c common/l4_rtc.c

SRC_C		= vmain.c elf-loader.c $(LIBC) $(L4_COMMON) $(PORT) $(ROOT) $(SPECS) $(MACH)

#SRC_S		= arm/context_switch.S

#SRC_CC          =

# list requirements of your program here
REQUIRES_LIBS   = libvcpu l4re_c-util libio l4re_c

include $(L4DIR)/mk/prog.mk

ARCH=arm

LIBC_INCLUDE = -nostdinc -nostdlib -g -O2 -fno-strict-aliasing  -march=armv4 -Wextra -Wall -Wno-unused-parameter -Wno-unused-function -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations  -fno-common -std=gnu99 -fno-stack-protector -Wno-pointer-sign

#CFLAGS = -I$(PKGDIR)/include/ertl_core/ -D_KERNEL_ -D__KERNEL__ -D__L4_ERTL -D__DEBUG__ -DKERNDATE=$(KERNDATE) -DARCH_$(ARCH)
CFLAGS = -I$(PKGDIR)/include/ertl_core/ -D_KERNEL_ -D__KERNEL__ -D__L4_ERTL  -DKERNDATE=$(KERNDATE) -DARCH_$(ARCH)
#ASFLAGS = -I$(PKGDIR)/include/ertl_core/ -D_KERNEL_ -D__KERNEL__ -D__L4_ERTL -DKERNDATE=$(KERNDATE) -DARCH_$(ARCH)
