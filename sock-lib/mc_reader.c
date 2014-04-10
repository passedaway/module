/*
 * =====================================================================================
 *
 *       Filename:  udpcli.c
 *
 *    Description:  test udpcli in kernel mode
 *
 *        Version:  1.0
 *        Created:  04/28/2013 11:47:27 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Changqing,Zhao (NO), changqing.1230@163.com
 *        Company:  None
 *
 * =====================================================================================
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/net.h>

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/signal.h>

#include "omni_socket.h"

#define server_ip "192.168.228.128"
#define server_port 8080
#define multicast_ip "224.0.0.1"
#define multicast_port 8080

static struct socket *udpcli_sock;
static struct task_struct *_udpcli_thread;
static int _udpcli_running = 0;

static int _udpcli_thread_func(void *args)
{
	char buf[128] = "hello,world";
	int ret= 0;
	struct sockaddr_in cliaddr;

	/* in kthread_create, it blocks all signal. so open it at here */
	allow_signal(SIGKILL);
	allow_signal(SIGTERM);

#if 0
	if( -1 == sock_udpcli_connect_srv(udpcli_sock, "172.20.1.35", 8080) )
	{
		printk("error: connect to server error\n");
		goto out;
	}
#endif
	_udpcli_running  = 1;
	set_current_state(TASK_INTERRUPTIBLE);

#if 0
	if ( 0 > sock_tcpcli_connect_srv(udpcli_sock, server_ip, server_port) )
	{
		printk("%s: connect server error\n", __FUNCTION__);
		goto out;
	}
#endif
	//sock_make_sockaddr_in(multicast_ip, multicast_port, &cliaddr);
	memset(&cliaddr, 0, sizeof(cliaddr));
	while( !kthread_should_stop() && !signal_pending(current) )
	{
#if 0
		if( -1 == sock_write(udpcli_sock, buf, sizeof(buf), &cliaddr) )
		{
			printk("error: cannot send message\n");
			goto out;
		}
#endif

#if 1
		if ( 0 < (ret = sock_read(udpcli_sock, buf, sizeof(buf), &cliaddr)) )
		{
			printk("%s: recv %x:%x len=%d %s\n", __FUNCTION__, cliaddr.sin_addr.s_addr, cliaddr.sin_port, ret, buf);
			//sock_write(udpcli_sock, buf, ret, NULL);
		}else{
			printk("%s: recv error\n", __FUNCTION__);
			if( ret == 0 )
			{
				printk("%s: disconnet. restart or exit\n", __FUNCTION__);
				goto out;
			}
		}
#endif

		ssleep(1);
	}

out:
	_udpcli_running  = 0;
	printk("%s: exit\n", __FUNCTION__);
	return 0;
}

static __init int udpcli_init(void)
{
	if( -1 == sock_create_multicast_reader(&udpcli_sock, multicast_ip, \
				multicast_port, "eth0") )
	{
		printk("error: cannot create udp server\n");
		goto out;
	}

	_udpcli_thread = kthread_run(_udpcli_thread_func, NULL, "sock_thread");
	printk("module init over\n");
	return 0;
out:
	printk("module init success. but sock error\n");
	return 0;
}

static __exit void udpcli_exit(void)
{
	if( (1 == _udpcli_running) &&  _udpcli_thread ) 
	{
		printk("call kthread_stop\n");
#if 0
		/* 3.6.6 kernel has this  */
		kill_pid(find_vpid(_udpcli_thread->pid), SIGKILL, 0);
#elif 1
		/* 3.2.o kernel use this */
#endif
		kthread_stop(_udpcli_thread);
	}

	if( udpcli_sock )
	{
		sock_release(udpcli_sock);
	}

	printk("module exit.\n");
}

module_init(udpcli_init);
module_exit(udpcli_exit);

