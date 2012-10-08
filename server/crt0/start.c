extern int startapp(void);

asm(
".section .text		\n"
".globl _start		\n"
"_start:		\n"
"	bic sp, sp, #7	\n"
"	bl startapp	\n"
);

