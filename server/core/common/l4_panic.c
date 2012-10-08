/*
 * $FILE: panic.c
 *
 * PANIC !!!!
 *
 * $VERSION: 1.0
 *
 * Author: Miguel Masmano <mimastel@doctor.upv.es>
 *
 * $LICENSE:  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <l4/irqs.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <processor.h>

#include <l4/sys/vcpu.h>

extern void _exit (int);

/* dump_state dumps the CPU registers */
void dump_state (void) {
  l4vcpu_print_state(l4ertl_vcpu, "vcpu");
}

void panic (char *fmt, ...) {
  va_list vl; 
  
  printf ("\n\nSystem PANIC:\n");
  va_start (vl, fmt);
  vprintf (fmt, vl);
  va_end (vl);

  dump_state ();
  
  _exit (-1);
}
