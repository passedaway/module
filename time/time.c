/*
 * =====================================================================================
 *
 *       Filename:  time.c
 *
 *    Description:  test kernel time 
 *
 *        Version:  1.0
 *        Created:  2013-06-04 11:42:01 
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
#include <linux/time.h>


static int __init _test_init(void)
{
	struct timeval tv;
	struct timespec ts;
	unsigned long mktime_res = 0;
	unsigned long js = 0;

	do_gettimeofday(&tv);
	printk("do_gettimeofday:tv.tv_sec = %lu  tv_usec = %lu\n", tv.tv_sec, tv.tv_usec);

	ts = current_kernel_time();
#if 0
	printk("current_kernel_time:ts.ts_sec = %lu  ts_nsec = %lu\n", ts.ts_sec, ts.ts_nsec);
#else
	printk("current_kernel_time:ts.ts_sec = %lu  ts_nsec = %lu\n", ts.tv_sec, ts.tv_nsec);
#endif

	mktime_res = mktime(2013, 6, 4, 14, 16, 0);
	printk("mktime:20130604 14:14:00 mktime_res=%ld\n", mktime_res);

#define DAY_IN_SECONDS (24*3600)
#define TIME_ZONE8 (3600*8)
	js = tv.tv_sec % DAY_IN_SECONDS;
	js += TIME_ZONE8;
	printk("js = %lu\n", js);
	return 0;
}

static void __exit _test_exit(void)
{

}

module_init(_test_init);
module_exit(_test_exit);

MODULE_LICENSE("GPL");

