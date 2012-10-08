#!/bin/sh

#for QEMU
#make qemu E=l4ertl_vexpressa9
#make qemu E=l4ertl
#make qemu E=$1
#qemu-system-arm -M vexpress-a9 -m 512 -serial stdio -kernel /home/cheng/TUDOS/Src/l4re-snapshot-2011102517/obj/l4/arm-ca/images/bootstrap.elf 


#for Pandaboard hardware
#make rawimage E=irq-latency MODULE_SEARCH_PATH=/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/conf/examples:/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/obj/fiasco/arm-mp-omap4/:/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/files:/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/obj/l4/arm-omap/bin/arm_armv7a/l4f

make rawimage E=l4ertl_omap4 MODULE_SEARCH_PATH=/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/conf/examples:/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/obj/fiasco/arm-mp-omap4:/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/files:/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/obj/l4/arm-ca/bin/arm_armv4/l4f
