/*
 * =====================================================================================
 *
 *       Filename:  nethook.c
 *
 *    Description:  test net hook
 *
 *        Version:  1.0
 *        Created:  2013年04月14日 16时16分55秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhao, Changqing (NO), changqing.1230@163.com
 *        Company:  iPanel TV inc.
 *
 * =====================================================================================
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>

#include <linux/netfilter_ipv4.h>

/* nf_hookfn */
static unsigned int _my_hook_ops_process(unsigned int hooknum,
		struct sk_buff *skb,
		const struct net_device *in,
		const struct net_device *out,
		int (*okfn)(struct sk_buff*))
{
	printk("%s in & out\n", __FUNCTION__);
	if( skb )
	{
		int i = 0;
		unsigned char *mac_hdr = skb_mac_header(skb);
		printk("filter: mac_len = %d hdrlen = %d \n", skb->mac_len, skb->hdr_len);
		printk("hooknum=%d src_mac:%02x:%02x:%02x:%02x:%02x:%02x\n", hooknum, 
				*mac_hdr, *(mac_hdr+1), *(mac_hdr+2),
				*(mac_hdr+3), *(mac_hdr+4), *(mac_hdr+5));
	}

	return NF_ACCEPT;
}

/* defination in linux/netfilter.h */
static struct nf_hook_ops _my_test_hook_ops = {
	.hooknum = NF_INET_PRE_ROUTING,
	.hook = &_my_hook_ops_process,
	.owner = THIS_MODULE,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST + 4,
};

static int __init _hook_test_init(void)
{
	nf_register_hook(&_my_test_hook_ops);
	return 0;
}

static void __exit _hook_test_exit(void)
{
	nf_unregister_hook(&_my_test_hook_ops);
}

module_init(_hook_test_init);
module_exit(_hook_test_exit);

