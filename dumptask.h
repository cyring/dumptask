/*
 * showtask, dumptask
 * Copyright (C) 2017-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define LOCKLESS " "
#define BUS_LOCK "lock "

#define BITBSR(_base, _index)				\
({							\
	register unsigned char _ret = 0;		\
	asm volatile					\
	(						\
		"bsr	%[base], %[index]"	"\n\t"	\
		"setz	%[ret]"				\
		: [ret]   "+r" (_ret),			\
		  [index] "=r" (_index) 		\
		: [base]  "rm" (_base)			\
		: "cc", "memory"			\
	);						\
	_ret;						\
})

#define _BITCLR_GPR(_lock, _base, _offset)	\
({						\
	asm volatile				\
	(					\
	_lock	"btrq	%%rdx,	%[base]"	\
		: [base] "=m" (_base)		\
		: "d" (_offset)			\
		: "cc", "memory"		\
	);					\
})

#define _BITCLR_IMM(_lock, _base, _imm8)	\
({						\
	asm volatile				\
	(					\
	_lock	"btrq	%[imm8], %[base]"	\
		: [base] "=m" (_base)		\
		: [imm8] "i" (_imm8)		\
		: "cc", "memory"		\
	);					\
})

#define BITCLR(_lock, _base, _offset)			\
(							\
	__builtin_constant_p(_offset) ?			\
		_BITCLR_IMM(_lock, _base, _offset)	\
	:	_BITCLR_GPR(_lock, _base, _offset)	\
)

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
		if (BITBSR(z, i) == 1)	\
			i = 0;		\
		else			\
			i++;		\
		i;			\
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
