PREFIX=/opt/arm-2010.09/bin/arm-none-linux-gnueabi-
CC=$(PREFIX)gcc
LD=$(PREFIX)ld
OBJCOPY=$(PREFIX)objcopy
RM=rm

objs=timers.o
startobj=/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/pkg/l4ertl/server/crt0/start.o

userent=/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/pkg/l4ertl/server/ulibc/user_point_entry.o

export USE_PRIVATE_LIBGCC =yes

#CFLAGS= -Wall -O2 -D__ASSEMBLY__ -fno-common -fno-strict-aliasing -fno-builtin -fexceptions -nostdlib -nostdinc -I/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/pkg/l4ertl/include/ertl_user/
#CFLAGS= -Wall -g -O2 -fno-strict-aliasing  -march=armv4 -Wextra -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations  -fno-common -fno-stack-protector -D__ASSEMBLY__ -nostdlib -nostdinc -I/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/pkg/l4ertl/include/ertl_user/
CFLAGS= -Wall -g -march=armv4 -Wextra -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations  -fno-common -fno-stack-protector -nostdlib -nostdinc -I/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/pkg/l4ertl/include/ertl_user/

boot.bin: $(objs)
	$(LD) -nostdinc++ -nostdinc -nostdlib  -m armelf_linux_eabi -o boot.elf  /opt/arm-2010.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/crtbeginT.o /home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/pkg/l4ertl/server/crt0/start.o $(objs) -Bstatic -T partikle.lds --start-group --static  /opt/arm-2010.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/libgcc.a /opt/arm-2010.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/libgcc_eh.a  /home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/pkg/l4ertl/server/ulibc/libulibc.a --end-group --warn-common -gc-sections /opt/arm-2010.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/crtend.o 
#	-L/opt/arm-2010.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/ \
	-L/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/pkg/l4ertl/server/ulibc \
	-o boot.elf $(objs) --start-group -lgcc -lgcc_eh -lulibc --end-group -gc-sections /opt/arm-2010.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/crtend.o $(startobj) /opt/arm-2010.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/crtbeginT.o  /opt/arm-2010.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/libgcc.a /opt/arm-2010.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/libgcc_eh.a
#	$(LD) -no-integrated-cpp -nostdinc++ -nostdinc -nostdlib  \
	-L/opt/arm-2010.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/ \
	-L/home/cheng/TUDOS/Src/l4re-snapshot-2011102517/src/l4/pkg/l4ertl/server/ulibc \
	-o boot.elf $(objs) --start-group -lgcc -lgcc_eh -lulibc --end-group
	#$(OBJCOPY) -G startV boot.elf
#	$(LD) -T partikle.lds -o test boot.elf
#	$(RM) -f boot.elf
%.o:%.c  
	$(CC) -Wall $(CFLAGS) -c -o $@  $<
%.o:%.S  
	$(CC) -Wall $(CFLAGS)  -c -o $@ $<
	
clean:  
	rm -f *.bin test *elf  *.o   
