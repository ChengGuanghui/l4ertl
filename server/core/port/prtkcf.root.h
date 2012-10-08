extern unsigned char root4code[];
extern int root4len;
int rootmaxq = 5;
Dirtab roottab[5] = {
	{"",	{0, 0, QTDIR},	 0,	0555},
	{"dev",	{1, 0, QTDIR},	 0,	0555},
	{"tmp",	{2, 0, QTDIR},	 0,	0555},
	{"usr",	{3, 0, QTDIR},	 0,	0555},
	{"NOTES",	{4, 0, QTFILE},	 0,	0444},
};
Rootdata rootdata[5] = {
	{0,	 &roottab[1],	 3,	NULL},
	{0,	 NULL,	 0,	 NULL},
	{0,	 NULL,	 0,	 NULL},
	{0,	 &roottab[4],	 1,	NULL},
	{3,	 root4code,	 0,	 &root4len},
};
