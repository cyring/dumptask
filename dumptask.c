#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/sched.h>

MODULE_AUTHOR ("CyrIng");
MODULE_DESCRIPTION ("Dump Task");
MODULE_SUPPORTED_DEVICE ("all");
MODULE_LICENSE ("GPL");

static struct task_list_st {
	long state;
	int wake_cpu;
	pid_t pid;
	char comm[TASK_COMM_LEN];
} tasklist[PID_MAX_DEFAULT];

static int do_dump_task(struct task_list_st *tlist)
{
	int i = 0;

	struct task_struct *pTask;
	for_each_process(pTask) {

		struct task_struct *tTask;
		for_each_thread(pTask, tTask) {

			tlist[i].state	  = tTask->state;
			tlist[i].wake_cpu = tTask->wake_cpu;
			tlist[i].pid	  = tTask->pid;
			memcpy(tlist[i].comm, tTask->comm, TASK_COMM_LEN);

		i++;
		}
	}
	return(i);
}

static struct task_struct *dump_tid = NULL;

int dump_threadfunc(void *arg)
{
	if (arg != NULL) {
		struct task_list_st *ptasklist = (struct task_list_st *) arg;

		int b = 0;
		while (!kthread_should_stop()) {
			int i;
			i = do_dump_task(ptasklist);

			msleep(1000);

			while (i > 1) {
				printk("dumptask: cpu[%d][%ld] task: %s (%d)\n",
					tasklist[i].wake_cpu,
					tasklist[i].state,
					tasklist[i].comm,
					tasklist[i].pid);

					i--;
			}
			b = !b;
			printk("dumptask: %s\n", b ? "Tick" : "Tock");
		}
	}
	return(0);
}

#define	DEVNAME "dumptask"
#define	FILENAME "/dev/"DEVNAME

static struct {
	int major;
	struct cdev *kcdev;
	dev_t nmdev, mkdev;
	struct class *clsdev;
} dumptask_dev;

static struct file_operations dumptask_fops = {
	.open	= nonseekable_open,
};

static int __init dumptask_init(void)
{
	dumptask_dev.kcdev = cdev_alloc();
	dumptask_dev.kcdev->ops = &dumptask_fops;
	dumptask_dev.kcdev->owner = THIS_MODULE;

        if (alloc_chrdev_region(&dumptask_dev.nmdev, 0, 1, FILENAME) >= 0) {
		dumptask_dev.major = MAJOR(dumptask_dev.nmdev);
		dumptask_dev.mkdev = MKDEV(dumptask_dev.major,0);

		if (cdev_add(dumptask_dev.kcdev, dumptask_dev.mkdev, 1) >= 0) {
			struct device *tmpdev;

			dumptask_dev.clsdev =class_create(THIS_MODULE, DEVNAME);

			if ((tmpdev=device_create(dumptask_dev.clsdev,
						NULL,
						dumptask_dev.mkdev,
						NULL,
						DEVNAME)) != NULL) {

				dump_tid = kthread_create(dump_threadfunc,
							tasklist,
							"dump_threadfunc");
				if (dump_tid == NULL)
					return(-EBUSY);
				else {
				  printk("dumptask: loaded [mem=%lu/max=%lu]\n",
					sizeof(tasklist),
					sizeof(tasklist) / sizeof(tasklist[0]));

					wake_up_process(dump_tid);
				}
			} else {
				printk("dumptask: device_create():KO\n");
				return(-EBUSY);
			}
		} else {
			printk("dumptask: cdev_add():KO\n");
			return(-EBUSY);
		}
	} else {
		printk("dumptask: alloc_chrdev_region():KO\n");
		return(-EBUSY);
	}
	return(0);
}

static void __exit dumptask_cleanup(void)
{
	device_destroy(dumptask_dev.clsdev, dumptask_dev.mkdev);
	class_destroy(dumptask_dev.clsdev);
	cdev_del(dumptask_dev.kcdev);
	unregister_chrdev_region(dumptask_dev.mkdev, 1);

	if (dump_tid != NULL)
		kthread_stop(dump_tid);
	else
		printk("dumptask: no kthread to stop !\n");

	printk("dumptask: unload\n");
}

module_init(dumptask_init);
module_exit(dumptask_cleanup);
