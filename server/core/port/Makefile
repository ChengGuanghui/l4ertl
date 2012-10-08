PRTK            = ../../
CONF            = prtkcf
DEVS            = devcons devroot devtest
LINK            = koutput rtc pit
ROOT            = $(CONF).c $(CONF).root.c

AWK = awk
O = o
DATA2C = $(PRTK)/utils/data2c
export CONF PRTK AWK O DATA2C

KERNDATE:=$(shell date '+%s')

.PHONY: $(CONF).root.c $(CONF).c $(CONF).root.h

$(CONF).c:
	(echo dev  $(DEVS); echo link $(LINK)) | sh ./mkdevb > $(CONF).c
	sh ./mkroot -c $(CONF)
	sh ./mkroot -h $(CONF)
