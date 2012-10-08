#include "dev.h"
#include "prtkcf.root.h"
ulong ndevs = 11;

extern Dev consdevtab;
extern Dev rootdevtab;
extern Dev testdevtab;
Dev* devtab[]={
	&consdevtab,
	&rootdevtab,
	&testdevtab,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

extern void koutputlink(void);
extern void rtclink(void);
extern void pitlink(void);
void links(void){
	koutputlink();
	pitlink();
}

unsigned long kerndate = KERNDATE;
