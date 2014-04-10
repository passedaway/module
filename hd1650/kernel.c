/*
 * =====================================================================================
 *
 *       Filename:  kernel.c
 *
 *    Description:  f49v01 for kernel control mode
 *
 *        Version:  1.0
 *        Created:  2012年11月14日 16时52分07秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhao, Changqing (NO), changqing.1230@163.com
 *        Company:  iPanel TV inc.
 *
 * =====================================================================================
 */
#include <linux/module.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>

#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/time.h>

#include <linux/proc_fs.h>

/*  wait queue head */
#include <linux/wait.h>
#include <linux/sched.h>

/* work queue, insead timer */
#include <linux/workqueue.h>

typedef struct {
	unsigned char hh;
	unsigned char mm;
	unsigned char ss;
	unsigned char mode; 
	/*mode:
	 * 0: do not display
	 * 1: show hh:mm
	 * other:not support, do mode 1
	 */
}fp_time_t;

#define MY_IOC_MAGIC    'Q'
#define IOCTL_FP_SET_DOT	_IOW(MY_IOC_MAGIC, 0, int)
#define IOCTL_FP_SET_TIME	_IOW(MY_IOC_MAGIC, 1, fp_time_t)
#define IOCTL_FP_GET_TIME	_IOR(MY_IOC_MAGIC, 2, fp_time_t)

#define DISP_BUF_LEN 32
#define KEY_LEN		16
struct hd1650_data_t{
	unsigned char disp_buf[DISP_BUF_LEN];
	unsigned char displen;
	unsigned char disppos;

	unsigned char key[KEY_LEN];
	unsigned char keyrpos;
	unsigned char keywpos;

	fp_time_t time;
}g_hd1650;

#define put_key(a)	 do{\
						g_hd1650.key[g_hd1650.keywpos] = a; \
						g_hd1650.keywpos++; \
						g_hd1650.keywpos %= KEY_LEN; \
					}while(0)

static inline unsigned char get_key(void)
{
	unsigned char key = g_hd1650.key[g_hd1650.keyrpos]; 
	g_hd1650.keyrpos++; 
	g_hd1650.keyrpos %= KEY_LEN; 
	return key;
}

#define can_read()		(g_hd1650.keywpos != g_hd1650.keyrpos)
static inline int can_write(void)
{
	unsigned char keywpos = g_hd1650.keywpos;
	unsigned char keyrpos = g_hd1650.keyrpos;

	keywpos = (keywpos+1) % KEY_LEN;
	
	return keywpos != keyrpos;
}

static int g_dot_flag = 2; /* init led */
module_param(g_dot_flag, int, 0);

static unsigned int major = 101;
static char *devname = "fp-hd1650";

/* semaphore, for single access */
static struct semaphore mysem;
/* for read  key*/
static DECLARE_WAIT_QUEUE_HEAD(key_waitq);

/* for time debug proc file */
struct proc_dir_entry *proc_fp_dir; /* dir */
struct proc_dir_entry *proc_fp_time; /*file */
struct proc_dir_entry *proc_fp_dot; /* file */

extern void hd1650_platform_init(void);
extern void hd1650_init(void);
extern void hd1650_show_each(unsigned char data, unsigned char pos,unsigned char dot_flag);
extern void hd1650_show(unsigned char *data,unsigned char dot_flag);
extern unsigned char hd1650_getkey(unsigned char *key);

/* work */
static struct work_struct fp_work;
static void fp_work_handler(void *ptr)
{
#define THIS_HZ		8  /* seclec 8 have some spectial resion,
						  HZ=1000, so HZ/8 = 125, the time will accurate( 精准 )
						*/

	static int _times = 0;
	_times ++;
	_times %= THIS_HZ;

	/*  lock */
	if( down_interruptible( &mysem ) )
		return;

	/*  for display */
	/* 4 is the led window size */
	if( 
			!g_hd1650.time.mode /* didnot show time */
			&& ( _times == ((THIS_HZ/2)-1) || _times == (THIS_HZ-1))  /*every seconds two times */
			&&  (g_hd1650.displen > 4)  /* display buf is lager then 4 */
		)
	{
		unsigned char tmp[4] = {0};
		
		if ( g_hd1650.displen - g_hd1650.disppos < 4 )
		{
			int tmp_len = g_hd1650.displen - g_hd1650.disppos;
			memcpy(tmp, &g_hd1650.disp_buf[g_hd1650.disppos], tmp_len);
		}else
		{
			memcpy(tmp, &g_hd1650.disp_buf[g_hd1650.disppos], 4);
		}

		g_hd1650.disppos++;
		g_hd1650.disppos %= g_hd1650.displen;

		hd1650_show(tmp, g_dot_flag);
	}

	/* for show time */
	if( _times == (THIS_HZ-1) )
	{
		g_hd1650.time.ss++;
		if( g_hd1650.time.ss >= 60 )
		{
			g_hd1650.time.mm++;
			g_hd1650.time.ss = 0;
		}
		if( g_hd1650.time.mm >= 60 )
		{
			g_hd1650.time.hh = (g_hd1650.time.hh+1) % 24;
			g_hd1650.time.mm = 0;
		}

		if ( g_hd1650.time.mode )
		{
			static unsigned char colon_flag=0;
			unsigned char buf[4]={0};
			colon_flag ^= 0xff;
			buf[0] = g_hd1650.time.hh / 10 + '0';
			buf[1] = g_hd1650.time.hh % 10 + '0';
			buf[2] = g_hd1650.time.mm / 10 + '0';
			buf[3] = g_hd1650.time.mm % 10 + '0';

			if( colon_flag )
				g_dot_flag |= 0x01;
			else
				g_dot_flag &= 0xFE;
			hd1650_show(buf, g_dot_flag);
		}
	}

	/* for key */
	{
		static unsigned char last_key = 0;
		unsigned char key;
		hd1650_getkey(&key);
		if( key != 0x2e )
		{
#if 1
			unsigned char tmp = key;
			/* for repeat key process */
			if ( last_key == tmp )
			{
				tmp &= 0xFE;
//				printk("recv repeat key : %x \n", last_key);
			}
#endif

			if ( !can_write() )
			{ 
				g_hd1650.keyrpos++;
				g_hd1650.keyrpos %= KEY_LEN;
				printk("%s:get key:%x rpos=%d wpos=%d overflow\n", __FUNCTION__, tmp, g_hd1650.keyrpos, g_hd1650.keywpos);
			}
			put_key(tmp);

			wake_up_interruptible(&key_waitq);
		}

		last_key = key;
	}

	/* unlock */
	up( &mysem );


}

/*  for roll display & get key */
static struct timer_list fp_timer;
static void timer_handler(unsigned long arg)
{
	/* set next timer, put to this ,the clock will accurate */
	fp_timer.expires += HZ/THIS_HZ;
	add_timer(&fp_timer);

	/* schedule work */
	schedule_work( &fp_work );
}

static void set_time(fp_time_t *ptime)
{
	if( !ptime || ptime->hh > 24 || ptime->mm > 60 || ptime->ss > 60 )
		return;
	if( down_interruptible( &mysem ) )
		return;
	memcpy(&g_hd1650.time, ptime, sizeof(fp_time_t));
	up( &mysem );
}

/* rw proc entry */
static int proc_fp_time_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int len = 0;
	len = sprintf(buf, "time=%d:%d:%d mode=%d\n", g_hd1650.time.hh, g_hd1650.time.mm, g_hd1650.time.ss,
			g_hd1650.time.mode);
	return len;
}

static int proc_fp_time_write(struct file *filep, const char *buf, unsigned long count, void *data)
{
	int hh=0, mm=0, ss=0, mode = 0;
	count = (count>32)?32:count;
	sscanf(buf, "%d:%d:%d %d\n", &hh, &mm, &ss, &mode);
	printk("hh=%d mm=%d ss=%d mode =%d\n", hh, mm, ss, mode);

	if( hh > 24 || mm > 60 || ss > 60 )
	{
		printk("invalid time\n");
		return 0;
	}

	if( down_interruptible( &mysem ) )
		return 0;
	g_hd1650.time.hh = hh;
	g_hd1650.time.mm = mm;
	g_hd1650.time.ss = ss;
	g_hd1650.time.mode = mode;
	up( &mysem );
	return count;
}

static int proc_fp_doc_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int len = 0;
	len = sprintf(buf, "dot=%d len:%s colon:%s\n", g_dot_flag, 
			g_dot_flag&0x02?"on":"off", g_dot_flag&0x01?"on":"off");
	return len;
}

static int proc_fp_doc_write(struct file *filep, const char *buf, unsigned long count, void *data)
{
	g_dot_flag = simple_strtol(buf, NULL, 0);
	if ( down_interruptible( &mysem ) )
		return 0;
	hd1650_show(g_hd1650.disp_buf, g_dot_flag);
	g_hd1650.disppos = 0;
	g_hd1650.time.mode = 0;
	up( &mysem );

	return count;
}

static int fp_open(struct inode *inode, struct file *filp)
{
    printk("%s out\n", __FUNCTION__);
    return 0;
}

static int fp_close(struct inode *inode, struct file *filp)
{
    printk("%s out\n", __FUNCTION__);
    return 0;
}

static ssize_t fp_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned char key=0;
	if( !buf || count < 0 )
		return -1;

	if( !can_read() )
	{
//		printk("no keyvalid. keyrpos=%d keywpos=%d. wait..\n", g_hd1650.keyrpos, g_hd1650.keywpos);
		do{
			if( wait_event_interruptible(key_waitq, can_read() ) )
			{
				/* this is must, becase some time user will kill this process */
				/* then it will do while1 */
				return -ERESTARTSYS; 
			}
//			printk("wait evnent out\n");
		}while( ! can_read() );
	}
	key = get_key();

#if 0
	/* atomatic access hd1650 */
	if( down_interruptible( &mysem ) )
    {
        printk("error:%s %d down_interruptible\n",__FUNCTION__, __LINE__);
        return -ERESTARTSYS;
    }
	hd1650_getkey(&key);
    up( &mysem );
#endif

	put_user(key, buf);
	printk("%s:get key 0x%x repeat:%s\n", __FUNCTION__, key, (key&0x01)?"no":"yes");
	return 1;
}

static ssize_t fp_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	if(count < 0 || !buf )
		return -1;

#if 0
	/*  do not open this */
	{
		int i = 0;
		printk("%s : count=%d\n", __FUNCTION__, count);
		for(; i < count; i++)
		{
			printk("%02x %c\n", g_hd1650.disp_buf[i], g_hd1650.disp_buf[i]);
		}
	}
#endif
/* atomatic access hd1650 */
	if( down_interruptible( &mysem ) )
    {
        printk("error:%s %d down_interruptible\n",__FUNCTION__, __LINE__);
        return -ERESTARTSYS;
    }
	memset(g_hd1650.disp_buf, 0, DISP_BUF_LEN);
	count = count > DISP_BUF_LEN ? DISP_BUF_LEN:count;
	g_hd1650.displen = count;
	g_hd1650.disppos = 0;
	g_hd1650.time.mode = 0;
	copy_from_user(g_hd1650.disp_buf, buf, count);
#if 1
	/*  for debug */
	printk("%s : count=%d str=%s #over\n", __FUNCTION__, count, g_hd1650.disp_buf);
#endif
	hd1650_show(g_hd1650.disp_buf, g_dot_flag);
    up( &mysem );

	return count;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static long fp_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
#else
static int fp_ioctl(struct inode *inode, struct file * file, unsigned int cmd, unsigned long arg)
#endif
{
	switch( cmd )
	{
		case IOCTL_FP_SET_DOT:
			if( down_interruptible( &mysem ) )
				return -ERESTARTSYS;
			g_dot_flag = *((unsigned char *)arg);
			up( &mysem );
			break;

		case IOCTL_FP_SET_TIME:
			{
				fp_time_t tmp;
				copy_from_user(&tmp, (fp_time_t *)arg, sizeof(fp_time_t));
				set_time( &tmp );
			}
			break;

		case IOCTL_FP_GET_TIME:
			copy_to_user( (unsigned char *)arg, &g_hd1650.time, sizeof(fp_time_t) );
			break;

		default:
			return -ENOTTY;
	}
	
	return 0;
}

static struct file_operations fp_hd1650_fops= {
    .owner = THIS_MODULE,
    .open = fp_open,
    .release = fp_close,
    .read = fp_read,
    .write = fp_write,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
    .unlocked_ioctl = fp_ioctl,
#else
    .ioctl = fp_ioctl,
#endif
};

static __init int fp_init(void)
{
	int ret = -1;
	struct timeval tv;

    sema_init(&mysem, 1);/* same with : init_MUTEX(&sem) */
	/* register char dev */
	ret = register_chrdev(major, devname, &fp_hd1650_fops);
    if( ret != 0 )
    {
        printk("Error : registe_chrdev %d\n", ret);
        goto OUT;
    }

	memset(&g_hd1650, 0, sizeof(g_hd1650));
	/* init rtc time */
	do_gettimeofday(&tv);
	if( tv.tv_sec > 3600 )
		g_hd1650.time.hh = (tv.tv_sec / 60 / 60)%24;
	if( tv.tv_sec > 60)
		g_hd1650.time.mm = (tv.tv_sec / 60) % 60;
	g_hd1650.time.ss = tv.tv_sec % 60;
	printk("rtc time=%d:%d:%d tv.tv_sec=%lu\n", g_hd1650.time.hh, g_hd1650.time.mm, g_hd1650.time.ss, tv.tv_sec);

	/* init work */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	INIT_WORK(&fp_work, fp_work_handler);
#else
	INIT_WORK(&fp_work, fp_work_handler, &g_hd1650);
#endif

	/*  init timer */
	init_timer(&fp_timer);
	fp_timer.data = 0;
	fp_timer.function = timer_handler;
	fp_timer.expires = jiffies + HZ/5;
	add_timer(&fp_timer);

	/*  clear display */
	hd1650_platform_init();
	hd1650_init();
	g_hd1650.time.mode = 1;

	/*  creat proc entry */
	proc_fp_dir = proc_mkdir("fp", NULL);
	proc_fp_time = create_proc_entry("time", 0644, proc_fp_dir);
	proc_fp_time->read_proc = proc_fp_time_read;
	proc_fp_time->write_proc = proc_fp_time_write;

	proc_fp_dot = create_proc_entry("dot", 0644, proc_fp_dir);
	proc_fp_dot->read_proc = proc_fp_doc_read;
	proc_fp_dot->write_proc = proc_fp_doc_write;

	printk("%s init over\n", __FUNCTION__);
OUT:
	return ret;
}

static __exit void fp_exit(void)
{
#if 0
	del_timer(&fp_timer);
#else
	/* this can confirm the timer will not be called */
	del_timer_sync(&fp_timer);
#endif
	/*  clear display */
	hd1650_init();

    unregister_chrdev(major, devname);

	/*remove proc entry */
	remove_proc_entry("dot", proc_fp_dir);
	remove_proc_entry("time", proc_fp_dir);
	remove_proc_entry("fp", NULL);
	printk("%s exit over\n", __FUNCTION__);
}

module_init(fp_init);
module_exit(fp_exit);

MODULE_LICENSE("ipanel fp-hd1650 F49v01 zhaocq@ipanel.cn");

