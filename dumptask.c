/*
 * showtask, dumptask
 * Copyright (C) 2017-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/device.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif

#include "dumptask.h"

MODULE_AUTHOR ("CyrIng");
MODULE_DESCRIPTION ("Dump Task");
MODULE_SUPPORTED_DEVICE ("all");
MODULE_LICENSE ("GPL");

static struct task_gate_st *taskgate = NULL;

long do_dump_task(struct task_gate_st *gate)
{
    if (gate != NULL) {
	struct task_struct *process, *thread;
	int cnt = 0;

	rcu_read_lock();
	for_each_process_thread(process, thread)
	{
		gate->tasklist[cnt].runtime  = thread->se.sum_exec_runtime;
		gate->tasklist[cnt].usertime = thread->utime;
		gate->tasklist[cnt].systime  = thread->stime;
		gate->tasklist[cnt].pid      = thread->pid;
		gate->tasklist[cnt].tgid     = thread->tgid;
		gate->tasklist[cnt].ppid     = thread->parent->pid;
		gate->tasklist[cnt].state    = (short int) thread->state;
		gate->tasklist[cnt].wake_cpu = (short int) thread->wake_cpu;
		memcpy(gate->tasklist[cnt].comm, thread->comm, TASK_COMM_LEN);

		cnt++;
	}
	rcu_read_unlock();
	gate->count = cnt;

	return(0);
    } else {
	return(-1);
    }
}

static struct {
	int major;
	struct cdev *kcdev;
	dev_t nmdev, mkdev;
	struct class *clsdev;
} dumptask_dev;

static int dumptask_mmap(struct file *pfile, struct vm_area_struct *vma)
{
	if (vma->vm_pgoff == 0) {
	    if ((taskgate != NULL)
	      && remap_pfn_range(vma,
				vma->vm_start,
				virt_to_phys((void *) taskgate) >> PAGE_SHIFT,
				vma->vm_end - vma->vm_start,
				vma->vm_page_prot) < 0)
		return(-EIO);
	}
	return(0);
}

static DEFINE_MUTEX(dumptask_mutex);	/* Only one driver instance. */

static int dumptask_open(struct inode *inode, struct file *pfile)
{
	if (!mutex_trylock(&dumptask_mutex))
		return(-EBUSY);
	else
		return(0);
}

static int dumptask_release(struct inode *inode, struct file *pfile)
{
	mutex_unlock(&dumptask_mutex);
	return(0);
}

static long dumptask_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
	long rc;
	switch (cmd) {
	case DUMPTASK_IOCTL_DUMP:
		rc = do_dump_task(taskgate);
		break;
	default:
		rc = 0;
	}
	return(rc);
}

static struct file_operations dumptask_fops = {
	.open    = dumptask_open,
	.release = dumptask_release,
	.mmap    = dumptask_mmap,
	.unlocked_ioctl = dumptask_ioctl,
	.owner   = THIS_MODULE,
};

void dumptask_cleandev(int level)
{
	switch (level) {
	case 4:
		device_destroy(dumptask_dev.clsdev, dumptask_dev.mkdev);
		/* fallthrough */
	case 3:
		class_destroy(dumptask_dev.clsdev);
		/* fallthrough */
	case 2:
		cdev_del(dumptask_dev.kcdev);
		/* fallthrough */
	case 1:
		unregister_chrdev_region(dumptask_dev.mkdev, 1);
		/* fallthrough */
	default:
		break;
	}
}

static int __init dumptask_init(void)
{
    dumptask_dev.kcdev = cdev_alloc();
    dumptask_dev.kcdev->ops = &dumptask_fops;
    dumptask_dev.kcdev->owner = THIS_MODULE;

    if (alloc_chrdev_region(&dumptask_dev.nmdev, 0, 1, DRV_FILENAME) >= 0) {
	dumptask_dev.major = MAJOR(dumptask_dev.nmdev);
	dumptask_dev.mkdev = MKDEV(dumptask_dev.major,0);

	if (cdev_add(dumptask_dev.kcdev, dumptask_dev.mkdev, 1) >= 0) {
	    struct device *tmpdev;

	    dumptask_dev.clsdev = class_create(THIS_MODULE, DEVNAME);

	    if ((tmpdev=device_create(dumptask_dev.clsdev,
					NULL,
					dumptask_dev.mkdev,
					NULL,
					DEVNAME)) != NULL) {

		size_t reqSize = sizeof(struct task_gate_st);
		int reqOrder = get_order(reqSize);
		int reqPages = PAGE_SIZE << reqOrder;

		if ((taskgate = alloc_pages_exact(reqSize, GFP_KERNEL)) != NULL)
		{
			printk( "dumptask: loaded[order=%d,size=%zd,pages=%d,"\
				"slot=%zd,pid=%d]\n",
				reqOrder, reqSize, reqPages,
				sizeof(struct task_list_st), PID_MAX_DEFAULT);
		} else {
			dumptask_cleandev(4);
			return(-ENOMEM);
		}
	    } else {
		dumptask_cleandev(3);
		return(-EBUSY);
	    }
	} else {
	    dumptask_cleandev(2);
	    return(-EBUSY);
	}
    } else {
	dumptask_cleandev(1);
	return(-EBUSY);
    }
    return(0);
}

static void __exit dumptask_cleanup(void)
{
	dumptask_cleandev(4);

	if (taskgate != NULL) {
		size_t reqSize = sizeof(struct task_gate_st);
		free_pages_exact(taskgate, reqSize);
	}
	printk("dumptask: unload\n");
}

module_init(dumptask_init);
module_exit(dumptask_cleanup);
