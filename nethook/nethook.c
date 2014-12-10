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

#include "omni_file.h"

omni_file_t write_file;

static inline int _skb_len(struct sk_buff *skb)
{
	return skb->len + skb->mac_len;//mac-header len, + data-len(ip,transport,data)
}

static inline int _copy_skb(struct sk_buff *skb, unsigned char *buf)
{
	int copy = _skb_len(skb);

	memcpy(buf, skb->mac_header, copy);

	return copy;
}

static unsigned char g_buf[81920];
static int  _total_len = 0;
static unsigned char *pos = g_buf;

/* nf_hookfn */
static unsigned int _my_hook_ops_process(unsigned int hooknum,
		struct sk_buff *skb,
		const struct net_device *in,
		const struct net_device *out,
		int (*okfn)(struct sk_buff*))
{
	int len = 0;
#if 0
	if( skb )
	{
		unsigned char *mac_hdr = skb->data;
		printk("===len=%d data_len=%d truesize=%d ", skb->len, skb->data_len, skb->truesize);
		printk("src_mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
				*mac_hdr, *(mac_hdr+1), *(mac_hdr+2),
				*(mac_hdr+3), *(mac_hdr+4), *(mac_hdr+5));
		printk("skb->data=%p skb->tail=%p skb->head=%p\n", skb->data, skb->tail, skb->head);
		printk("network_header=%p tarnsport_header=%p mac_header=%p\n", skb->network_header,
				skb->transport_header, skb->mac_header);
	}
#else

	if( _total_len >= 71920 )
	{
		return NF_ACCEPT;
	}

		len = _skb_len(skb);
		*pos++ = len >> 8;
		*pos++ = len & 0x0ff;
		_copy_skb(skb, pos);
		pos += len;

		_total_len += (len + 2);
	//omni_file_write(write_file, g_buf, len+2);
#endif

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

static struct nf_hook_ops _my_test_hooks[] = {
#if 0
	{
		.hooknum = NF_INET_PRE_ROUTING,
		.hook = &_my_hook_ops_process,
		.owner = THIS_MODULE,
		.pf = PF_INET,
		.priority = NF_IP_PRI_FIRST + 4,
	},
#endif
#if 1
	{
		.hooknum = NF_INET_LOCAL_IN,
		.hook = &_my_hook_ops_process,
		.owner = THIS_MODULE,
		.pf = PF_INET,
		.priority = NF_IP_PRI_FIRST + 4,
	},
#endif
#if 0
	{
		.hooknum = NF_INET_LOCAL_OUT,
		.hook = &_my_hook_ops_process,
		.owner = THIS_MODULE,
		.pf = PF_INET,
		.priority = NF_IP_PRI_FIRST + 4,
	},
#endif
#if 0
	{
		.hooknum = NF_INET_POST_ROUTING,
		.hook = &_my_hook_ops_process,
		.owner = THIS_MODULE,
		.pf = PF_INET,
		.priority = NF_IP_PRI_FIRST + 4,
	},
#endif
};

static int __init _hook_test_init(void)
{
	write_file = omni_file_open("/opt/packge.dat", O_RDWR | O_TRUNC | O_CREAT, 666);

#if 0
	nf_register_hook(&_my_test_hook_ops);
#endif
	nf_register_hooks(_my_test_hooks, ARRAY_SIZE(_my_test_hooks));
	return 0;
}

static void __exit _hook_test_exit(void)
{
	nf_unregister_hook(&_my_test_hook_ops);
	if( write_file )
	{
		printk("test exit: total_len=%d\n", _total_len);
		omni_file_write(write_file, g_buf, _total_len);
		omni_file_close(write_file);
	}
}

module_init(_hook_test_init);
module_exit(_hook_test_exit);

