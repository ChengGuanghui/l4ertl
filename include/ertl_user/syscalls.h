/*
 * $FILE: syscalls.h
 *
 * System calls, ARM version
 *
 * $VERSION: 1.0
 *
 * Author: Miguel Masmano <mimastel@doctor.upv.es>
 *
  * $LICENSE:
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

// This header file defines the way to call PaRTiKle's functions

#ifndef _ULIBC_SYSCALLS_H_
#define _ULIBC_SYSCALLS_H_

#include <arch/syscalls.h>

#include <kernel_inc/syscalls.h>

#define __syscall0(name_nr, __res) \
  __res = L4ERTL_SYSCALL0(name_nr)

#define __syscall1(arg1, name_nr, __res) \
  __res = L4ERTL_SYSCALL1(name_nr, arg1) 

#define __syscall2(arg1, arg2, name_nr, __res) \
  __res = L4ERTL_SYSCALL2(name_nr, arg1, arg2) 

#define __syscall3(arg1, arg2, arg3, name_nr, __res) \
  __res = L4ERTL_SYSCALL3(name_nr, arg1, arg2, arg3) 

#define __syscall4(arg1, arg2, arg3, arg4, name_nr, __res) \
  __res = L4ERTL_SYSCALL4(name_nr, arg1, arg2, arg3, arg4)

#define __syscall5(arg1, arg2, arg3, arg4, arg5, name_nr, __res) \
  __res = L4ERTL_SYSCALL5(name_nr, arg1, arg2, arg3, arg4, arg5)

#define __syscall6(arg1, arg2, arg3, arg4, arg5, arg6, name_nr, __res) \
  __res = L4ERTL_SYSCALL6(name_nr, arg1, arg2, arg3, arg4, arg5, arg6)


/**********************************************/

#define __syscall_return(type, res) \
  return (type) (res); \

#define _syscall0(type,name) \
type name(void) { \
  long __res; \
  __syscall0(name##_nr, __res); \
  __syscall_return(type,__res); \
}

#define _syscall1(type,name,type1,arg1) \
type name(type1 arg1) { \
  long __res; \
  __syscall1(arg1, name##_nr, __res); \
  __syscall_return(type,__res); \
}

#define _syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) { \
  long __res; \
  __syscall2(arg1, arg2, name##_nr, __res); \
  __syscall_return(type,__res); \
}

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) { \
  long __res; \
  __syscall3(arg1, arg2, arg3, name##_nr, __res); \
  __syscall_return(type,__res); \
}

#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) { \
  long __res; \
  __syscall4(arg1, arg2, arg3, arg4, name##_nr, __res); \
  __syscall_return(type,__res); \
}

#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
          type5,arg5) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5) { \
  long __res; \
  __syscall5(arg1, arg2, arg3, arg4, arg5, name##_nr, __res); \
  __syscall_return(type,__res); \
}

#define _syscall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
          type5,arg5,type6,arg6) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5,type6 arg6) { \
  long __res; \
  __syscall6(arg1, arg2, arg3, arg4, arg5, arg6, name##_nr, __res); \
  __syscall_return(type,__res); \
}

#endif
