/*
 * showtask, dumptask
 * Copyright (C) 2017 CYRIL INGENIERIE
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

void showtask(struct task_gate_st *gate)
{
	printf(	"   cpu      state     |      task      |"	\
		"             pid     tgid     ppid\n"		\
		"----------------------+----------------+"	\
		"----------------------------------\n");

	int cnt;
	for (cnt = 0; cnt < gate->count; cnt++) {
		printf("%5hd  %8hx       |%16s|          %5d    %5d    %5d\n",
			gate->tasklist[cnt].wake_cpu,
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
	}
	return(rc);
}
