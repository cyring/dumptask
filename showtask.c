/*
 * showtask, dumptask
 * Copyright (C) 2017-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dumptask.h"

static const char symbol[] = "RSDTtXZPIKWiNm";

void stateToSymbol(short int state, char stateStr[])
{
	unsigned short idx, jdx = 0;

	if (BITBSR(state, idx) == 1)
		stateStr[jdx++] = symbol[0];
	else
		do {
			BITCLR(LOCKLESS, state, idx);
			stateStr[jdx++] = symbol[1 + idx];
		} while (!BITBSR(state, idx));
	stateStr[jdx] = '\0';
}

void showtask(struct task_gate_st *gate)
{
	char stateStr[16];

	printf( "   cpu       state     |      task      |"	\
		"             pid     tgid     ppid\n"		\
		"------------ ----------+----------------+"	\
		"----------------------------------\n");

	int cnt;
	for (cnt = 0; cnt < gate->count; cnt++) {
		stateToSymbol(gate->tasklist[cnt].state, stateStr);

		printf("%5hd%13s%4hx |%16s|          %5d    %5d    %5d\n",
			gate->tasklist[cnt].wake_cpu,
			stateStr,
			gate->tasklist[cnt].state,
			gate->tasklist[cnt].comm,
			gate->tasklist[cnt].pid,
			gate->tasklist[cnt].tgid,
			gate->tasklist[cnt].ppid);
	}
	printf("%d tasks\n", gate->count);
}

int main(int argc, char *argv[])
{
	int rc = (geteuid() != 0), drv = -1;
	struct task_gate_st *taskgate = NULL;
	size_t reqSize = sizeof(struct task_gate_st);
	int reqOrder = get_order(reqSize);
	int reqPages = PAGE_SIZE << reqOrder;

	if (!rc) {
		if ((drv = open(DRV_FILENAME, O_RDWR|O_SYNC)) != -1) {
			if ((taskgate = mmap(NULL, reqPages,
					PROT_READ|PROT_WRITE, MAP_SHARED,
					drv, 0)) != MAP_FAILED) {

				if (ioctl(drv, DUMPTASK_IOCTL_DUMP, NULL) != -1)
					showtask(taskgate);
				else
					rc = 4;

				if (munmap(taskgate, reqPages) == -1)
					rc = 5;
			}
			else
				rc = 3;
			close(drv);
		}
		else
			rc = 2;
	}
	if (argc == 2) {
	    if (!strncmp(argv[1], "-d", 2))
		printf("\nshowtask: mmap[order=%d,size=%zd,pages=%d,"	\
			"slot=%zd,pid=%d]\n",
			reqOrder, reqSize, reqPages,
			sizeof(struct task_list_st),PID_MAX_DEFAULT);
	  else if (!strncmp(argv[1], "-s", 2)) {
		const struct {
			char		symb;
			char		*name;
			unsigned short	value;
		} stateHelp[] = {
			{symbol[ 0],	"RUNNING",		0x0000},
			{symbol[ 1],	"INTERRUPTIBLE",	0x0001},
			{symbol[ 2],	"UNINTERRUPTIBLE",	0x0002},
			{symbol[ 3],	"STOPPED",		0x0004},
			{symbol[ 4],	"TASK_TRACED",		0x0008},

			{symbol[ 5],	"EXIT_DEAD",		0x0010},
			{symbol[ 6],	"EXIT_ZOMBIE",		0x0020},

			{symbol[ 7],	"PARKED",		0x0040},
			{symbol[ 8],	"DEAD",			0x0080},
			{symbol[ 9],	"WAKEKILL",		0x0100},
			{symbol[10],	"WAKING",		0x0200},
			{symbol[11],	"NOLOAD",		0x0400},
			{symbol[12],	"NEW",			0x0800},
			{symbol[13],	"STATE_MAX",		0x1000}
		};
		unsigned int idx;
		for (idx = 0; idx < 14; idx++)
			printf("%c\t%16s\t%4hx\n",
				stateHelp[idx].symb,
				stateHelp[idx].name,
				stateHelp[idx].value);
	    }
	}
	return(rc);
}
