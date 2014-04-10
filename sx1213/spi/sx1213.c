#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>		/* register_chrdev */
#include <linux/slab.h>		/* kmalloc */
#include <asm/uaccess.h>	/* container_of */
#include <linux/fs.h>
#include <linux/version.h>	/* LINUX_VERSION macro */


#define SX1213_MAJOR		123

#define SX1213_AUTHOR	"zhaocq@ipanel.cn"
#define SX1213_DESC		"This is 7230 SPI Sx1213"
#define SX1213_VERSION	"V1.0 2012-02-15"

#define PHYS_TO_K0(x)   ((x) | 0x80000000)
#define PHYS_TO_K1(x)   ((x) | 0xa0000000)
#define REG(a)  			(*((volatile unsigned long *)(a)))
#define SPI_PIN_MUX_REG		0x10404130

#define SPI_BASE			0x10413200
#define SPCR0_LSB			0
#define SPCR0_MSB			1
#define SPCR1_LSB			2
#define SPCR1_MSB			3
#define NEWQP				4
#define ENDQP				5
#define SPCR2				6
#define MSPI_STATUS			7
#define CPTQP				8

#define SPI_TX_BASE			0x10413240
#define TXRAM0				0
#define TX_MAX_SIZE			32
#define SPI_RX_BASE			0x104132C0
#define RXRAM0				0
#define RX_MAX_SIZE			32

#define CDRAM_BASE			0x10413340
#define CDRAM_MAX_SIZE		16

#define SPI_END_BASE		0x10413380
#define WRITE_LOCK			0
#define DISABLE_FLUSH_GEN	1

#define SPI_REG(base, offset)		REG( PHYS_TO_KEY0( (base) + (4*(offset) ) ) )


static void set_spi_pin_mux(void);
static void spi_default_config(void);
static int sx1213_en_config(void);
static int sx1213_en_data(void);


/*****************implementing********************/
void set_spi_pin_mux(void)
{
	unsigned long data = REG(PHYS_TO_K0(SPI_PIN_MUX_REG));

	printk("= data = 0x%08x\n", data);

	data &= ~0x0FFFFF00;
	data |= 0x5636600;
	printk("= data = 0x%08x\n", data);
	REG(PHYS_TO_K0(SPI_PIN_MUX_REG)) = data;
}

void spi_default_config(void)
{
}

static int sx1213_open(struct inode *inode, struct file *filp)
{
	unsigned int devnum;

	printk("= sx1213_open\n");

	devnum = MINOR(inode->i_rdev);
	if( devnum == 0 ){
		/* init sx1213 data spi bus */
		/*
		to dos
		*/
		printk("=sx1213 open data \n");
	}else if( devnum == 1 ){
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
static long sx1213_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
#else
static int sx1213_ioctl(struct inode *inode, struct file * file, unsigned int cmd, unsigned long arg)
#endif
{
	printk("=sx1213_ioctl\n");
	return 0;
}

static ssize_t sx1213_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
	printk("=sx1213_read\n");
	return 0;
}

static ssize_t sx1213_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
	printk("=sx1213_write\n");
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
