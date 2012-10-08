#include <l4/io.h>

#include <stdio.h>
#include <config.h>
#include <irqs.h>
#include <processor.h>

#include <l4/re/env.h>
#include <l4/vcpu/vcpu.h>
#include <l4/sys/utcb.h>
#include <l4/sys/kdebug.h>

//------------------------//
// setup_cpu //
//-----------//

int setup_cpu (void) {
  init_irqs_traps ();

  return 0;
}

//----------//
// iopl_sys //
//----------//

int iopl_sys (int level) {
  return 3;
}

//-------------//
// get_cpu_khz //
//-------------//

unsigned long get_cpu_khz (void) {
  l4_kernel_info_t *kip = l4re_kip(); 

  if(kip)
    return kip->frequency_cpu;
  else {
    printf("Can't map KIP to local place.\n");
    enter_kdebug("KIP failed");
  }

  return 0;
}
