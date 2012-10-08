#ifndef _REGS_H_
#define _REGS_H_

#include <l4/vcpu/vcpu.h>

#ifndef _KERNEL_
#error Internal file. Do not include it in your sources.
#endif

#ifdef ARCH_arm

/*
 * PSR bits
 */
#define USR26_MODE      0x00000000
#define FIQ26_MODE      0x00000001
#define IRQ26_MODE      0x00000002
#define SVC26_MODE      0x00000003
#define USR_MODE        0x00000010
#define FIQ_MODE        0x00000011
#define IRQ_MODE        0x00000012
#define SVC_MODE        0x00000013
#define ABT_MODE        0x00000017
#define UND_MODE        0x0000001b
#define SYSTEM_MODE     0x0000001f
#define MODE32_BIT      0x00000010
#define MODE_MASK       0x0000001f
#define PSR_T_BIT       0x00000020
#define PSR_F_BIT       0x00000040
#define PSR_I_BIT       0x00000080
#define PSR_A_BIT       0x00000100
#define PSR_E_BIT       0x00000200
#define PSR_J_BIT       0x01000000
#define PSR_Q_BIT       0x08000000
#define PSR_V_BIT       0x10000000
#define PSR_C_BIT       0x20000000
#define PSR_Z_BIT       0x40000000
#define PSR_N_BIT       0x80000000

struct pt_regs {
  unsigned long regs[17];
};

enum {
  DO_SYSTEM_CALL = 0,
  DO_IRQ = 1,
};

#define ARM_cpsr        regs[16]
#define ARM_pc          regs[15]
#define ARM_lr          regs[14]
#define ARM_sp          regs[13]
#define ARM_r12         regs[12]
#define ARM_r11         regs[11]
#define ARM_r10         regs[10]
#define ARM_r9          regs[9]
#define ARM_r8          regs[8]
#define ARM_r7          regs[7]
#define ARM_r6          regs[6]
#define ARM_r5          regs[5]
#define ARM_r4          regs[4]
#define ARM_r3          regs[3]
#define ARM_r2          regs[2]
#define ARM_r1          regs[1]
#define ARM_r0          regs[0]

static inline void vcpu_to_ptregs(l4_vcpu_state_t *v,
                                  struct pt_regs *regs)
//                                  struct pt_regs *regs, int type)
{
  regs->ARM_r0 = v->r.r[0];
  regs->ARM_r1 = v->r.r[1];
  regs->ARM_r2 = v->r.r[2];
  regs->ARM_r3 = v->r.r[3];
  regs->ARM_r4 = v->r.r[4];
  regs->ARM_r5 = v->r.r[5];
  regs->ARM_r6 = v->r.r[6];

//  if(type == DO_IRQ)
//  {
  regs->ARM_r7  = v->r.r[7];
  regs->ARM_r8  = v->r.r[8];
  regs->ARM_r9  = v->r.r[9];
  regs->ARM_r10 = v->r.r[10];
  regs->ARM_r11 = v->r.r[11];
  regs->ARM_r12 = v->r.r[12];
  regs->ARM_sp    = v->r.sp;
  regs->ARM_lr    = v->r.lr;
  regs->ARM_pc    = v->r.ip;
  //}
}

static inline void ptregs_to_vcpu(l4_vcpu_state_t *v,
                                  struct pt_regs *regs)
//                                  struct pt_regs *regs, int type)
{
  v->r.r[0]  = regs->ARM_r0;

//if(type == DO_IRQ)
//  {
  v->r.r[1] = regs->ARM_r1;
  v->r.r[2] = regs->ARM_r2;
  v->r.r[3] = regs->ARM_r3;
  v->r.r[4] = regs->ARM_r4;
  v->r.r[5] = regs->ARM_r5;
  v->r.r[6] = regs->ARM_r6;
  v->r.r[7] = regs->ARM_r7;
  v->r.r[8] = regs->ARM_r8;
  v->r.r[9] = regs->ARM_r9;
  v->r.r[10] = regs->ARM_r10;
  v->r.r[11] = regs->ARM_r11;
  v->r.r[12] = regs->ARM_r12;
  v->r.sp    = regs->ARM_sp;
  v->r.lr    = regs->ARM_lr;
  v->r.ip    = regs->ARM_pc;
//  }
}

#endif  //ARCH_arm

#endif
