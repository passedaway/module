#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>		/* register_chrdev */
#include <linux/slab.h>		/* kmalloc */
#include <asm/uaccess.h>	/* container_of */
#include <linux/fs.h>
#include <linux/version.h>	/* LINUX_VERSION macro */
#include "spi.h"

#define SX1213_MAJOR		123
#define SX1213_DATA_MINOR	0
#define SX1213_CONFIG_MINOR	1

#define SX1213_AUTHOR	"zhaocq@ipanel.cn"
#define SX1213_DESC		"This is 7230 SPI Sx1213"
#define SX1213_VERSION	"V1.0 2012-02-15"

static int sx1213_open(struct inode *inode, struct file *filp)
{
	unsigned int devnum;

	printk("= sx1213_open\n");

	devnum = MINOR(inode->i_rdev);
	filp->private_data = (unsigned int *)devnum;
	if( devnum == SX1213_DATA_MINOR ){
		/* init sx1213 data spi bus */
		/*
		to dos
		*/
		printk("=sx1213 open data \n");
	}else if( devnum == SX1213_CONFIG_MINOR ){
		/* init sx1213 config spi bus */
		/*
		to dos
		*/
		printk("=sx1213 open config \n");
	}else{
		goto ERR;
	}

	return 0;

ERR:
	return -EFAULT;
}

static int sx1213_close(struct inode *inode, struct file *filp)
{
	printk("=sx1213_close\n");
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static long sx1213_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
#else
static int sx1213_ioctl(struct inode *inode, struct file * filp, unsigned int cmd, unsigned long arg)
#endif
{
	unsigned int devnum ;
	devnum = (unsigned int)filp->private_data;

	printk("=sx1213_ioctl\n");
	return 0;
}

static ssize_t sx1213_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
	unsigned int devnum ;
	unsigned char *k_buf;
	/*
	printk("=sx1213_read\n");
	*/
	devnum = (unsigned int)filp->private_data;

	spi_default_config();
	if( devnum == SX1213_CONFIG_MINOR)
	{
		unsigned char data = 0;
		/*
		printk("=sx1213_read config register\n");
		*/
		k_buf = (unsigned char *)kmalloc(2, GFP_KERNEL);
		if( k_buf == NULL )
		{
			printk("sx1213_write error no mem!\n");
			return 0;
		}
		copy_from_user(k_buf, buf, 1);
		data = read_register(k_buf[0]);
		copy_to_user(buf, &data, 1);
		kfree(k_buf);
		return 1;
	}else if( devnum == SX1213_DATA_MINOR){
		int i = 0;
		unsigned char *_tmp;


		printk("=sx1213_read data\n");

		if( !buf )
			return -1;

		if( count > 64 )
			return -1;

		k_buf = (unsigned char *)kmalloc(count, GFP_KERNEL);
		memset(k_buf, 0, count);
		if( !k_buf )
		{
			printk("Error : [%s][%s][%d] no mem!\n", __FILE__, __FUNCTION__, __LINE__);
			return -2;
		}
		_tmp = k_buf;

		for( ; i < count; i++)
		{
			*_tmp = receive_byte();
			printk("\nKernel[%s] :  0x%02x\n", __FUNCTION__, *_tmp);
			_tmp++;
		}

		copy_to_user(buf, k_buf, count);
		kfree(k_buf);
		return count;
	}

	return 0;
}

static ssize_t sx1213_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
	unsigned int devnum ;
	unsigned char *k_buf;

/*
	printk("=sx1213_write\n");
	*/
	devnum = (unsigned int)filp->private_data;

	spi_default_config();

	if( devnum == SX1213_CONFIG_MINOR)
	{
		/*
		printk("=sx1213_write config register\n");
		*/
		k_buf = (unsigned char *)kmalloc(2, GFP_KERNEL);
		if( k_buf == NULL )
		{
			printk("sx1213_write error no mem!\n");
			return 0;
		}
		copy_from_user(k_buf, buf, 2);
		write_register(k_buf[0], k_buf[1]);
		kfree(k_buf);
		return 1;
	}else if( devnum == SX1213_DATA_MINOR){
		printk("=sx1213_write data: no such methord!\n");
	}
	return 0;
}

static struct file_operations sx1213_fops = {
	.owner = THIS_MODULE,
	.read  = sx1213_read,
	.write = sx1213_write,
	.open = sx1213_open,
	.release = sx1213_close,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
    .unlocked_ioctl = sx1213_ioctl,
#else
    .ioctl = sx1213_ioctl,
#endif

};

static int __init sx1213_driver_init(void)
{
	printk("=sx1213_driver_init\n");
	set_spi_pin_mux();

	if( register_chrdev(SX1213_MAJOR, "sx1213 spi", &sx1213_fops) )
	{
		printk("register sx1213 modules error!\n");
		return -1;
	}

	return 0;
}
module_init(sx1213_driver_init);

static void __exit sx1213_driver_exit(void)
{
	#if 0
	if( unregister_chrdev(SX1213_MAJOR, "sx1213 spi") )
	{
		printk("unregister sx1213 modules error!\n");
	}
	#else
	unregister_chrdev(SX1213_MAJOR, "sx1213 spi");
	#endif

	printk("=sx1213_driver_exit\n");
}
module_exit(sx1213_driver_exit);


MODULE_AUTHOR(SX1213_AUTHOR);
MODULE_DESCRIPTION(SX1213_DESC);
MODULE_VERSION(SX1213_VERSION);
MODULE_LICENSE("GPL");
