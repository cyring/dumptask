/*
 * showtask, dumptask
 * Copyright (C) 2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define DEVNAME "dumptask"
#define DRV_FILENAME "/dev/"DEVNAME

#ifndef TASK_COMM_LEN
#define	TASK_COMM_LEN 16
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE (sysconf(_SC_PAGESIZE))
#endif

#ifndef PID_MAX_DEFAULT
#define PID_MAX_DEFAULT (1<<15)
#endif

#define ROUND_TO_PAGE(_size)	PAGE_SIZE * ((_size / PAGE_SIZE) 	\
				+ ((_size % PAGE_SIZE)? 1 : 0));

#define DUMPTASK_IOCTL_MAGIC 0xc6
#define DUMPTASK_IOCTL_DUMP _IO(DUMPTASK_IOCTL_MAGIC, 0)

struct task_list_st {
	long	state;
	int	wake_cpu;
	pid_t	pid,
		tgid,
		ppid;
	char	comm[TASK_COMM_LEN];
};

struct task_gate_st {
	int count;
	struct task_list_st tasklist[PID_MAX_DEFAULT];
};
