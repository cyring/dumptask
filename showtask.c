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
	printf("cpu\tstate\t\ttask\t\tpid\n");

	int cnt;
	for (cnt = 0; cnt < gate->count; cnt++) {
		printf("%d\t%ld\t%16s\t%d\n",
			gate->tasklist[cnt].wake_cpu,
			gate->tasklist[cnt].state,
			gate->tasklist[cnt].comm,
			gate->tasklist[cnt].pid);

	}
	printf("%d tasks\n", gate->count);
}

int main(int argc, char *argv[])
{
	int rc = (geteuid() != 0), drv = -1;
	struct task_gate_st *taskgate = NULL;
	unsigned long reqSize = sizeof(struct task_gate_st);
	unsigned long reqPage = ROUND_TO_PAGE(reqSize);
/*
	printf("showtask: mmap[page=%lu,size=%lu,slot=%lu,pid=%d]\n",
		reqPage, reqSize, sizeof(struct task_list_st),PID_MAX_DEFAULT);
*/
	if (!rc && ((drv = open(DRV_FILENAME, O_RDWR|O_SYNC)) != -1)) {
		if ((taskgate = mmap(NULL, reqPage,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				drv, 0)) != NULL) {

			if (ioctl(drv, DUMPTASK_IOCTL_DUMP, NULL) != -1)
				showtask(taskgate);

			munmap(taskgate, reqPage);
		}
		else
			rc = 3;
		close(drv);
	}
	else
		rc = 3;
	return(rc);
}
