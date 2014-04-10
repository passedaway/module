#include <linux/module.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/version.h>

/*  cdev_init, cdev_add.. */
#include <linux/cdev.h>

/* ioctl, _IO, */
#include <linux/ioctl.h>
/* sema */
#include <linux/semaphore.h>

/*  copy to/from user, put, get */
#include <asm/uaccess.h>

/*  wait queue head */
#include <linux/wait.h>
#include <linux/sched.h>

/* poll */
#include <linux/poll.h>

#define MY_IOC_MAGIC    'Q'
#define C_RESET     _IO(MY_IOC_MAGIC, 0)
#define C_GETRP     _IOR(MY_IOC_MAGIC, 1, int)
#define C_GETWP     _IOR(MY_IOC_MAGIC, 2, int)


static unsigned int major = 123;
static char *devname = "mycdev";

typedef struct dev_private{
    char buf[4096];
    int wpos, rpos;
    char *wp, *rp;
    int overflow;
}dev_private_t;

static dev_private_t gd={
    .buf = {0},
    .wpos = 0,
    .rpos = 0,
    .overflow = 0,
};

//DECLARE_MUTEX
//static DECLARE_MUTEX( mysem );
static struct semaphore mysem;
/* same with struct semaphore mysem */

/* wati queue head */
#if 0
static wait_queue_head_t mywaitq;
#else
static DECLARE_WAIT_QUEUE_HEAD(mywaitq);
#endif

module_param(major, int, 0);
module_param(devname, charp, 0);

static int mycdev_open(struct inode *inode, struct file *filp)
{
    printk("%s in\n", __FUNCTION__);

    printk("%s out\n", __FUNCTION__);
    return 0;
}

static int mycdev_close(struct inode *inode, struct file *filp)
{
    printk("%s in\n", __FUNCTION__);

    printk("%s out\n", __FUNCTION__);
    return 0;
}

static ssize_t mycdev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int tmp_len = 0;
	int ret = 0;
    printk("%s in\n", __FUNCTION__);
    if( down_interruptible( &mysem ) )
    {
        printk("error:%s %d down_interruptible\n",__FUNCTION__, __LINE__);
        return -ERESTARTSYS;
    }

	while( gd.wpos == gd.rpos)
	{
#if 0
        goto OUT;
#else
		//1. release lock
		up( &mysem );
		//2.wait
		printk("[%s][%d] wpos !> rpos, will wait \n",__FUNCTION__, __LINE__);
#if 0
		wait_event_interruptible(mywaitq, gd.wpos > gd.rpos);
#else
		if( wait_event_interruptible(mywaitq, gd.wpos > gd.rpos) )
		{
			printk("error:[%s][%d] wait_evnet_initerrupt by signal\n"
					"current pid=%d name=%s down_interruptible\n",
					__FUNCTION__, __LINE__,
					current->pid, current->comm);
			return -ERESTARTSYS;
		}
#endif
		//3.down again
		if( down_interruptible( &mysem ) )
		{
			printk("error:%s %d current pid=%d name=%s down_interruptible\n",__FUNCTION__, __LINE__,
						current->pid, current->comm);
			return -ERESTARTSYS;
		}
#endif
	}
	tmp_len = gd.wpos-gd.rpos;
	count = count > tmp_len?tmp_len:count;

    copy_to_user(buf, (char *)(gd.buf + gd.rpos), count);
    gd.rpos += count;
    gd.rpos %= 1024;/*notice:!!! this is 1024,not 4096*/

OUT:
	ret = count;
    up( &mysem );
    printk("%s out\n", __FUNCTION__);
    return ret;
}

static ssize_t mycdev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk("%s in\n", __FUNCTION__);
    /* use sema , »¥³âµÄ·ÃÎÊ*/
    if( down_interruptible( &mysem ) )
    {
        printk("error:%s %d down_interruptible\n",__FUNCTION__, __LINE__);
        return -ERESTARTSYS;
    }

    if(count>=(sizeof(gd.buf)-gd.wpos) )
        count = sizeof(gd.buf)-gd.wpos;
    copy_from_user((char *)(gd.buf + gd.wpos), buf, count);
    gd.wpos+=count;
    gd.wpos %= 4096;

	//wake up
	wake_up_interruptible(&mywaitq);

    /* use sema */
    up(&mysem);
    printk("%s out\n", __FUNCTION__);
    return count;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static long mycdev_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
#else
static int mycdev_ioctl(struct inode *inode, struct file * file, unsigned int cmd, unsigned long arg)
#endif
{
	int ret = 0;
    printk("%s in\n", __FUNCTION__);

    switch(cmd)
    {
        case C_RESET:
            printk("%s C-reset\n", __FUNCTION__);
            gd.rpos = gd.wpos = 0;
            break;

        case C_GETRP:
			 printk("%s C-getrp\n", __FUNCTION__);
			 ret = __put_user(gd.rpos, (int __user *)arg);
            break;

        case C_GETWP:
			 printk("%s C-getwp\n", __FUNCTION__);
			 ret = __put_user(gd.wpos, (int __user*)arg);
            break;

        default:
            printk("%s %d notty\n", __FUNCTION__, cmd);
            ret = -ENOTTY;
    }
    printk("%s out\n", __FUNCTION__);
    return ret;
}

static unsigned int mycdev_poll(struct file *filp, poll_table *wait)
{
	unsigned int ret = 0;

	printk("[%s][%d] in\n", __FUNCTION__, __LINE__);
	poll_wait(filp, &mywaitq, wait);
	if( gd.rpos < gd.wpos )
		ret |= POLLIN | POLLRDNORM; /*  readable */

	printk("[%s][%d] out\n", __FUNCTION__, __LINE__);
	return ret;
}

static struct file_operations mycdev_fops = {
    .owner = THIS_MODULE,
    .open = mycdev_open,
    .release = mycdev_close,
    .read = mycdev_read,
    .write = mycdev_write,
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
    .unlocked_ioctl = mycdev_ioctl,
    #else
    .ioctl = mycdev_ioctl,
    #endif
	.poll = mycdev_poll,
};

#define USE_OLD_METHOD 0
#if !USE_OLD_METHOD
struct cdev *mycdev;
#endif

static int __init mod_init(void)
{
    int ret = 0;
    dev_t devno = 0;
#if USE_OLD_METHOD
    /* this is old method */
    printk("old method register chardev\n");
    ret = register_chrdev(major, devname, &mycdev_fops);
    if( ret != 0 )
    {
        printk("Error : registe_chrdev %d\n", ret);
        goto OUT;
    }
#else
    printk("new method register chardev\n");
    /* this is new method */
    devno = MKDEV(major, 0);
    /* num'1' is the number of devices will be register*/
    ret = register_chrdev_region(devno, 1, devname);
    if( ret )
    {
        printk("Error %d register_chrdev_region \n", ret);
        goto OUT;
    }

    mycdev = cdev_alloc();
    cdev_init(mycdev, &mycdev_fops);
    mycdev->owner = THIS_MODULE;
    mycdev->ops = &mycdev_fops;
    /* if not pointer, use this
    *struct cdev mycdev;
    *cdev_init(&mycdev, &mycdev_fops);
    *mycdev.ops = &mycdev_fops;
    */

    ret = cdev_add(mycdev, devno, 1);
    if( ret )
    {
        printk("Error %d adev_add \n", ret);
        goto OUT1;
    }

#endif

    /* test semaphore */
    sema_init(&mysem, 1);/* same with : init_MUTEX(&sem) */
#if 0
	/* test wait queue, can use macro to declear and init it */
	init_waitqueue_head(&mywaitq);
#endif

    printk("register success: %s %d\n", devname, major);
    return 0;

OUT1:
    unregister_chrdev_region(devno, 1);
    printk("register Error: %s %d\n", devname, major);
OUT:
    return ret;
}

static void __exit mod_exit(void)
{
#if USE_OLD_METHOD
    unregister_chrdev(major, devname);
#else
    cdev_del(mycdev);
    unregister_chrdev_region( MKDEV(major,0), 1);
#endif

    printk("unreigste device ok\n");
    return;
}


module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("zhaocq");

