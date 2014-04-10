#include <linux/module.h>

#include <linux/notifier.h>
#include <linux/reboot.h>

static int do_omni_halt(struct notifier_block *notifier, 
			unsigned long what, void *data)
{
	printk(KERN_EMERG "%s: %lu \n", __FUNCTION__, what);
	return 0;
}

static struct notifier_block reboot_notifier = {
	.notifier_call = do_omni_halt,
	.priority = 0,
};

static int __init mod_init(void)
{
	register_reboot_notifier(&reboot_notifier);
	return 0;
}

static void __exit mod_exit(void)
{
	unregister_reboot_notifier(&reboot_notifier);
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");

