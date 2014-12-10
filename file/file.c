/*
 * =====================================================================================
 *
 *       Filename:  test_file.c
 *
 *    Description:  test kernel file
 *
 *        Version:  1.0
 *        Created:  05/21/2013 08:31:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  echo "Q2hhbmdxaW5nLFpoYW8K" | base64 -d  (tain)
 *			Email:	echo "Y2hhbmdxaW5nLjEyMzBAMTYzLmNvbQo=" | base64 -d 
 *        Company:  FreedomIsNotFree.com
 *
 * =====================================================================================
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include "omni_file.h"


static char buf[] = "hello, world.\n I am kernel.\n";

#if 0
static int __init _test_file_init(void)
{
	struct file *fp;
	mm_segment_t fs;
	loff_t pos;

	int ret = 0;
	char buf1[128] = {0};

	printk("%s init\n", __FUNCTION__);

	fp = filp_open("/home/tain/test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
	if( IS_ERR(fp) )
	{
		printk("open file error\n");
		return 0;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;

	ret = vfs_write(fp, buf, sizeof(buf), &pos);
	printk("write buf ret=%d pos=%lld %s \n", ret, pos, buf);

	pos = 0;
	ret = vfs_read(fp, buf1, sizeof(buf1), &pos);
	printk("read buf1 ret=%d pos=%lld %s\n", ret, pos, buf1);

	filp_close(fp, NULL);
	set_fs(fs);
	return 0;
}
#endif

char *ptest = 0;
static int size = 0x100000;
module_param(size, int, S_IRUGO);

static int __init _test_file_init(void)
{
#if 0
	omni_file_t fd;
	char buf1[128] = {0};
	int ret = 0;

	fd = omni_file_open("/home/tain/test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
	if( IS_ERR(fd) )
	{
		printk("open file error\n");
		return 0;
	}

	ret = omni_file_write(fd, buf, sizeof(buf));
	printk("write ret=%d buf = %s\n", ret, buf);

	omni_file_lseek(fd, 5, SEEK_SET);

	ret = omni_file_read(fd, buf1, sizeof(buf1));
	printk("read ret=%d buf = %s\n", ret, buf1);

	omni_file_close(fd);
	fd = NULL;

	ret = omni_mkdir("/opt/123456", 0777);
	printk("mkdir /opt/123456 ret=%d\n", ret);
#else
	if( size < 0x100000 )
		size <<= 20;
	printk("test malloc: 0x%x\n", size);
	ptest = kmalloc(size, GFP_KERNEL);
	if( ptest != NULL )
	{
		printk("ptest = %p size=0x%x MB\n", ptest, size>>20);
		kfree(ptest);
	}

	printk("test over.\n");

#endif
	return 0;
}

static void __exit _test_file_exit(void)
{

}

module_init(_test_file_init);
module_exit(_test_file_exit);

MODULE_LICENSE("GPL");

