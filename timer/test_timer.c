#include <linux/module.h>
#include <linux/timer.h>

#include <linux/kernel.h>

#include <linux/param.h>
#include <linux/jiffies.h>
#include <linux/moduleparam.h>

struct timer_list mytimer;

int delay = 100;
int times = 10;
module_param(delay, int, 0);
module_param(times, int, 0);

static void timer_func(unsigned long arg)
{
	times--;
	if( times == 0 )
		return;

	printk("%s %9li\n", __FUNCTION__, jiffies);
	
	mytimer.expires += delay;
	add_timer(&mytimer);
	printk("%s add one times=%d\n", __FUNCTION__, times);
}

static int __init mod_init(void)
{
	init_timer( &mytimer );

	mytimer.data = 0;
	mytimer.function = timer_func;
	mytimer.expires = jiffies + delay;

	add_timer(&mytimer);

	printk("mod_init over.\n");
	/*  !!!!this is must!!!! */
	return 0;
}

static void __exit mod_exit(void)
{
	del_timer_sync( &mytimer );
	printk("mod_exit over. do nothing\n");
}

module_init(mod_init);
module_exit(mod_exit);


