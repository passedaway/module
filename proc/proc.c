#include <linux/proc_fs.h>
#include <linux/types.h>
#include <linux/module.h>
/*copy to user, copy from user*/
#include <asm/uaccess.h>

/* module param */
static int debug = 0;
module_param(debug, int, 0);
#if 0
/*
* use macro to debug
*/
#define DEBUG 1
#if DEBUG
#define PDBG(fmt, args...)  printk("[%s][%d]:"fmt, __FUNCTION__, __LINE__, ## args)
#else
#define PDBG(fmt, args...)
#endif

#endif


#define PDBG(fmt, args...)  do{\
							if(debug){\
								printk("[%s][%d]:"fmt, __FUNCTION__, __LINE__, ## args);\
							}\
							}while(0)


static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_r_file, *proc_rw_file;

int g_num = 0;
static char g_buf[64]= {0};

/* read_proc_t
* int (*read_proc)(char *page, char **start, off_t offset, int count, int *eof, void *data);
*
*/
static int proc_rfile_read(char *buf, char **start, off_t offset,
                            int count, int *eof, void *data)
{
    int len = 0;
    unsigned long j = jiffies;
    static int times;
    PDBG("in\n");
    len = sprintf(buf, "[%ld] %d g_num = %d\n", j, times++, g_num);
    PDBG("out\n");
    return len;
}


/* read_proc_t */
static int proc_rw_read_proc(char *buf, char **start, off_t offset,
                             int count, int *eof, void *data)
{
    int len;
    PDBG("in\n");
    len = sprintf(buf, "g_num = %d \ng_buf=%s\n", g_num, g_buf);
    PDBG("out\n");
    return len;
}

/* a simple write funciton like this */
static int proc_rw_write_proc(struct file *filep, const char *buffer,
                                unsigned long count, void *data )
{
    PDBG("in count=%lu\n",count);
    count = (count>64?64:count);
    if( copy_from_user(g_buf, buffer, count))
    {
        printk("Error:copy form user error.\n");
        count = -1;
        goto OUT;
    }

  //  g_num = atoi(_buf);
  	g_num = simple_strtol(g_buf, NULL, 0);

    PDBG("out\n");

OUT:
    return count;
}

/* module init */
static int __init mod_init(void)
{
    PDBG("in\n");
	printk("debug = %d\n", debug);
   proc_dir = proc_mkdir("zhaocq", NULL);
   if( proc_dir == NULL )
	{
		printk("Error : cannot create proc dir\n");
		return -1;
	}

	proc_r_file = create_proc_read_entry("readfile", 0444, NULL,
	                proc_rfile_read, NULL);
    if( proc_r_file == NULL)
    {
        printk("Error : cannot creat proc_r_file.\n");
		return -1;
    }

    proc_rw_file = create_proc_entry("rwfile", 0644, proc_dir);
    if( proc_rw_file==NULL)
	{
        printk("Error : cannot creat proc_rw_file.\n");
		return -1;
	}
	proc_rw_file->read_proc = proc_rw_read_proc;
	proc_rw_file->write_proc = proc_rw_write_proc;

   PDBG("out\n");
   return 0;
}

/* module exit */
static void __exit mod_exit(void)
{
    PDBG("in\n");

    /*
    first args is file name,
    second is path ,if null is /proc
    */
    remove_proc_entry("readfile", NULL);
    remove_proc_entry("rwfile", proc_dir);
	remove_proc_entry("zhaocq", NULL);
    PDBG("out\n");
}


module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("zhaocq");

