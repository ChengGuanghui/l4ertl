#ifndef _ULIBC_ARM_SYSCALLS_H_
#define _ULIBC_ARM_SYSCALLS_H_

//#include <kernel_inc/errno.h>

#ifndef EINTER
#define EINTR 4
#endif

#ifndef _PT_REGS
#define _PT_REGS

struct pt_regs {
         unsigned long uregs[18];
};

#define ARM_cpsr        uregs[16]
#define ARM_pc          uregs[15]
#define ARM_lr          uregs[14]
#define ARM_sp          uregs[13]
#define ARM_ip          uregs[12]
#define ARM_fp          uregs[11]
#define ARM_r10         uregs[10]
#define ARM_r9          uregs[9]
#define ARM_r8          uregs[8]
#define ARM_r7          uregs[7]
#define ARM_r6          uregs[6]
#define ARM_r5          uregs[5]
#define ARM_r4          uregs[4]
#define ARM_r3          uregs[3]
#define ARM_r2          uregs[2]
#define ARM_r1          uregs[1]
#define ARM_r0          uregs[0]
#define ARM_ORIG_r0     uregs[17]

#endif

#define __l4ertl_sys_mux		555

//#define __xn_mux_code(shifted_id,op) ((op << 24)|shifted_id|(__xn_sys_mux & 0xffff))
#define __ertl_mux_code(nr,op) ((nr << 24) | (op & 0xffff))
#define __ertl_mux_shifted_id(id) ((id << 16) & 0xff0000)

//#define XENO_ARM_SYSCALL        0x000F0042	/* carefully chosen... */
#define L4ERTL_ARM_SYSCALL        0x000F0042	/* carefully chosen... */

/* Register mapping for accessing syscall args. */

#define __ertl_reg_mux(regs)      ((regs)->ARM_ORIG_r0)
#define __ertl_reg_rval(regs)     ((regs)->ARM_r0)
#define __ertl_reg_arg1(regs)     ((regs)->ARM_r1)
#define __ertl_reg_arg2(regs)     ((regs)->ARM_r2)
#define __ertl_reg_arg3(regs)     ((regs)->ARM_r3)
#define __ertl_reg_arg4(regs)     ((regs)->ARM_r4)
#define __ertl_reg_arg5(regs)     ((regs)->ARM_r5)

/* In OABI_COMPAT mode, handle both OABI and EABI userspace syscalls */
#define __ertl_reg_mux_p(regs)    ( ((regs)->ARM_r7 == __NR_OABI_SYSCALL_BASE + L4ERTL_ARM_SYSCALL) || \
				  ((regs)->ARM_r7 == __NR_SYSCALL_BASE + L4ERTL_ARM_SYSCALL) )
#define __ertl_linux_mux_p(regs, nr) \
				( ((regs)->ARM_r7 == __NR_OABI_SYSCALL_BASE + (nr)) || \
				  ((regs)->ARM_r7 == __NR_SYSCALL_BASE + (nr)) )

#define __ertl_mux_id(regs)       ((__ertl_reg_mux(regs) >> 16) & 0xff)
#define __ertl_mux_op(regs)       ((__ertl_reg_mux(regs) >> 24) & 0xff)

/* Purposedly used inlines and not macros for the following routines
   so that we don't risk spurious side-effects on the value arg. */

static inline void __ertl_success_return(struct pt_regs *regs, int v)
{
	__ertl_reg_rval(regs) = v;
}

static inline void __ertl_error_return(struct pt_regs *regs, int v)
{
	__ertl_reg_rval(regs) = v;
}

static inline void __ertl_status_return(struct pt_regs *regs, int v)
{
	__ertl_reg_rval(regs) = v;
}

static inline int __ertl_interrupted_p(struct pt_regs *regs)
{
	return __ertl_reg_rval(regs) == (unsigned long)(-EINTR);
}

/*
 * Some of the following macros have been adapted from Linux's
 * implementation of the syscall mechanism in <asm-arm/unistd.h>:
 *
 * The following code defines an inline syscall mechanism used by
 * Xenomai's real-time interfaces to invoke the skin module
 * services in kernel space.
 */

#define LOADARGS_0(muxcode, dummy...)	\
	__a0 = (unsigned long) (muxcode)
#define LOADARGS_1(muxcode, arg1)	\
	LOADARGS_0(muxcode);		\
	__a1 = (unsigned long) (arg1)
#define LOADARGS_2(muxcode, arg1, arg2)	\
	LOADARGS_1(muxcode, arg1);	\
	__a2 = (unsigned long) (arg2)
#define LOADARGS_3(muxcode, arg1, arg2, arg3) 	\
	LOADARGS_2(muxcode,  arg1, arg2);	\
	__a3 = (unsigned long) (arg3)
#define LOADARGS_4(muxcode,  arg1, arg2, arg3, arg4)	\
	LOADARGS_3(muxcode,  arg1, arg2, arg3);		\
	__a4 = (unsigned long) (arg4)
#define LOADARGS_5(muxcode, arg1, arg2, arg3, arg4, arg5)	\
	LOADARGS_4(muxcode, arg1, arg2, arg3, arg4);		\
	__a5 = (unsigned long) (arg5)
#define LOADARGS_6(muxcode, arg1, arg2, arg3, arg4, arg5, arg6)	\
	LOADARGS_5(muxcode, arg1, arg2, arg3, arg4, arg5);		\
	__a6 = (unsigned long) (arg6)

#define CLOBBER_REGS_0 "r0"
#define CLOBBER_REGS_1 CLOBBER_REGS_0, "r1"
#define CLOBBER_REGS_2 CLOBBER_REGS_1, "r2"
#define CLOBBER_REGS_3 CLOBBER_REGS_2, "r3"
#define CLOBBER_REGS_4 CLOBBER_REGS_3, "r4"
#define CLOBBER_REGS_5 CLOBBER_REGS_4, "r5"
#define CLOBBER_REGS_6 CLOBBER_REGS_5, "r6"

#define LOADREGS_0 __r0 = __a0
#define LOADREGS_1 LOADREGS_0; __r1 = __a1
#define LOADREGS_2 LOADREGS_1; __r2 = __a2
#define LOADREGS_3 LOADREGS_2; __r3 = __a3
#define LOADREGS_4 LOADREGS_3; __r4 = __a4
#define LOADREGS_5 LOADREGS_4; __r5 = __a5
#define LOADREGS_6 LOADREGS_5; __r6 = __a6

#define ASM_INDECL_0							\
	unsigned long __a0; register unsigned long __r0  __asm__ ("r0");
#define ASM_INDECL_1 ASM_INDECL_0;					\
	unsigned long __a1; register unsigned long __r1  __asm__ ("r1")
#define ASM_INDECL_2 ASM_INDECL_1;					\
	unsigned long __a2; register unsigned long __r2  __asm__ ("r2")
#define ASM_INDECL_3 ASM_INDECL_2;					\
	unsigned long __a3; register unsigned long __r3  __asm__ ("r3")
#define ASM_INDECL_4 ASM_INDECL_3;					\
	unsigned long __a4; register unsigned long __r4  __asm__ ("r4")
#define ASM_INDECL_5 ASM_INDECL_4;					\
	unsigned long __a5; register unsigned long __r5  __asm__ ("r5")
#define ASM_INDECL_6 ASM_INDECL_5;					\
	unsigned long __a6; register unsigned long __r6  __asm__ ("r6")

#define ASM_INPUT_0 "0" (__r0)
#define ASM_INPUT_1 ASM_INPUT_0, "r" (__r1)
#define ASM_INPUT_2 ASM_INPUT_1, "r" (__r2)
#define ASM_INPUT_3 ASM_INPUT_2, "r" (__r3)
#define ASM_INPUT_4 ASM_INPUT_3, "r" (__r4)
#define ASM_INPUT_5 ASM_INPUT_4, "r" (__r5)
#define ASM_INPUT_6 ASM_INPUT_5, "r" (__r6)

#define __sys2(x)	#x
#define __sys1(x)	__sys2(x)

#define __SYS_REG , "r7"
#define __SYS_REG_DECL register unsigned long __r7 __asm__ ("r7")
#define __SYS_REG_SET __r7 = L4ERTL_ARM_SYSCALL
#define __SYS_REG_INPUT ,"r" (__r7)
#define __ertl_syscall "swi\t0"

#define L4ERTL_DO_SYSCALL(nr, shifted_id, op, args...)			\
	({								\
		ASM_INDECL_##nr;					\
		__SYS_REG_DECL;						\
		LOADARGS_##nr(__ertl_mux_code(nr,op), args);	\
		__asm__ __volatile__ ("" : /* */ : /* */ :		\
				      CLOBBER_REGS_##nr __SYS_REG);	\
		LOADREGS_##nr;						\
		__SYS_REG_SET;						\
		__asm__ __volatile__ (					\
			__ertl_syscall					\
			: "=r" (__r0)					\
			: ASM_INPUT_##nr __SYS_REG_INPUT		\
			: "memory");					\
		(int) __r0;						\
	})

#define L4ERTL_SYSCALL0(op)			\
	L4ERTL_DO_SYSCALL(0,0,op)
#define L4ERTL_SYSCALL1(op,a1)			\
	L4ERTL_DO_SYSCALL(1,0,op,a1)
#define L4ERTL_SYSCALL2(op,a1,a2)		\
	L4ERTL_DO_SYSCALL(2,0,op,a1,a2)
#define L4ERTL_SYSCALL3(op,a1,a2,a3)		\
	L4ERTL_DO_SYSCALL(3,0,op,a1,a2,a3)
#define L4ERTL_SYSCALL4(op,a1,a2,a3,a4)	\
	L4ERTL_DO_SYSCALL(4,0,op,a1,a2,a3,a4)
#define L4ERTL_SYSCALL5(op,a1,a2,a3,a4,a5)		\
	L4ERTL_DO_SYSCALL(5,0,op,a1,a2,a3,a4,a5)
#define L4ERTL_SYSCALL6(op,a1,a2,a3,a4,a5,a6)		\
	L4ERTL_DO_SYSCALL(6,0,op,a1,a2,a3,a4,a5,a6)
#define L4ERTL_SYSBIND(a1,a2,a3,a4)				\
	L4ERTL_DO_SYSCALL(4,0,__ertl_sys_bind,a1,a2,a3,a4)

#endif /* _ULIBC_ARM_SYSCALLS_H_ */

// vim: ts=4 et sw=4 sts=4
