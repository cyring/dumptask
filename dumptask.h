/*
 * showtask, dumptask
 * Copyright (C) 2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define BITBSR(_base, _index)				\
({							\
	register unsigned char _ret = 0;		\
	asm volatile					\
	(						\
		"bsr	%[base], %[index]"	"\n\t"	\
		"setz	%[ret]"				\
		: [ret]   "+r" (_ret),			\
		  [index] "=r" (_index) 		\
		: [base]  "r" (_base)			\
		: "cc", "memory"			\
	);						\
	_ret;						\
})

#define DEVNAME "dumptask"
#define DRV_FILENAME "/dev/"DEVNAME

#ifndef TASK_COMM_LEN
#define	TASK_COMM_LEN 16
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE (sysconf(_SC_PAGESIZE))
#endif

#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12
#endif

#ifndef PID_MAX_DEFAULT
#define PID_MAX_DEFAULT (1<<15)
#endif

#define ROUND_TO_PAGE(_size)	PAGE_SIZE * ((_size / PAGE_SIZE) 	\
				+ ((_size % PAGE_SIZE)? 1 : 0));

#ifndef get_order
	#define get_order(n)		\
	({				\
		int z = n, i = -1;	\
		z--;			\
		z >>= PAGE_SHIFT;	\
		BITBSR(z, i);		\
		i + 1;			\
	})
#endif

#define DUMPTASK_IOCTL_MAGIC 0xc6
#define DUMPTASK_IOCTL_DUMP _IO(DUMPTASK_IOCTL_MAGIC, 0)

struct task_list_st {
	unsigned long long	runtime,
				usertime,
				systime;
	pid_t			pid,
				tgid,
				ppid;
	short int		state;		// TASK_STATE_MAX = 0x1000
	short int		wake_cpu;	// Max of 64K CPU
	char			comm[TASK_COMM_LEN];
};

struct task_gate_st {
	int count;
	struct task_list_st tasklist[PID_MAX_DEFAULT];
};
