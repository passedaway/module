/*
 * =====================================================================================
 *
 *       Filename:  debug.c
 *
 *    Description:  proc debug
 *
 *        Version:  1.0
 *        Created:  06/09/2013 02:00:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Changqing,Zhao (NO), changqing.1230163.com
 *        Company:  None
 *
 * =====================================================================================
 */

#include "config.h"
#include "mission.h"
#include "log.h"
#include "hb.h"

#include <linux/time.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#undef MODULE_NAME
#define MODULE_NAME	"debug"

static time_t _start_time;
static struct proc_dir_entry *_proc_root;
static struct proc_dir_entry *mission_entry, *interface_entry,
							 *runtime_entry, *state_entry,
							 *debug_entry;

char *ip2str(uint32_t ip)
{
	static char buf[2][16];
	static int i = 0;
	unsigned char *pstr = (unsigned char *)&ip;

	char *pbuf = buf[i];
	sprintf(pbuf, "%d.%d.%d.%d", pstr[0], pstr[1], pstr[2], pstr[3]);
	i = i?0:1;
	return pbuf;
}

uint16_t p2p(uint16_t port)
{
	return ((port&0xff)<< 8) | ((port>>8) & 0x0ff);
}

/* iterator */
extern struct list_head g_mission_list;
static void *ms_next(loff_t index)
{
	struct list_head *pl;
	mission_t *pms = NULL;
	loff_t i = 0;

	list_for_each(pl, &g_mission_list)
	{
		if( i++ >= index )
		{
			pms = container_of(pl, mission_t, list);
			goto exit;
		}
	}

exit:
	return pms;
}

static void *ms_seq_start(struct seq_file *m, loff_t *pos)
{
	return ms_next(*pos);
}

static void *ms_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	mission_t *pms = ms_next(*pos);
	(*pos)++;
	return pms;
}

static void ms_seq_stop(struct seq_file *m, void *v)
{
}

static int ms_seq_show(struct seq_file *m, void *v)
{
	int i = 0;
	linkid_t *plinkid;
	termid_t *pterm;
	user_t *puser;
	mission_t *pms = v;

	static const char *_ms_state[] = { "INIT", "READY", "START", "STOP", "CANCEL" };

	if( pms == NULL )
		return 0;

	seq_printf(m, "\nMid:0x%0x\n", pms->mid);
	seq_printf(m, "State:%s\n", _ms_state[pms->state]);
	seq_printf(m, "\tstart time:%d-%d-%d %d:%d:%d\n",
				pms->start_date.year, pms->start_date.month,
				pms->start_date.day, pms->start_time.hour,
				pms->start_time.minute, pms->start_time.second);
	seq_printf(m, "\t  end time:%d-%d-%d %d:%d:%d\n\n",
				pms->end_date.year, pms->end_date.month,
				pms->end_date.day, pms->end_time.hour,
				pms->end_time.minute, pms->end_time.second);

	seq_printf(m, "\t    user con type  :%d\n"
							 "\t    tern con type  :%d\n"
							 "\tfront user con type:%d\n"
							 "\tfront term con type:%d\n",
							 pms->user_con_type, pms->term_con_type,
							 pms->user_front_con_type,pms->term_front_con_type);

	seq_printf(m, "\n\tuser num:%d\n", pms->user_nums);
	puser = &pms->user[0];
	for(i=0; i < pms->user_nums; i++, puser++)
	{
		seq_printf(m, "\n\t\tmaster uac:0x%08X\n\t\t slave uac:0x%08X\n"
				"\t\t\tpacket type:%d\n"
				"\t\t\t\033[32m IP Type  %-16s %-8s %-16s %-8s\033[0m\n",
				puser->uac, puser->uac2, puser->packet_type,
				"Remote IP", "Port",
				"Local IP", "Port");

		seq_printf(m, "\t\t\t MM Back  %-16s %-8d %-16s %-8d\n",
				ip2str(puser->ipaddr.remote_ip), p2p(puser->ipaddr.remote_port),
				ip2str(puser->ipaddr.local_ip), p2p(puser->ipaddr.local_port));
		seq_printf(m, "\t\t\t MS Back  %-16s %-8d %-16s %-8d\n",
				ip2str(puser->ipaddr2.remote_ip), p2p(puser->ipaddr2.remote_port),
				ip2str(puser->ipaddr2.local_ip), p2p(puser->ipaddr2.local_port));
		seq_printf(m, "\t\t\tMM Front  %-16s %-8d %-16s %-8d\n",
				ip2str(puser->front_ipaddr.remote_ip), p2p(puser->front_ipaddr.remote_port),
				ip2str(puser->front_ipaddr.local_ip), p2p(puser->front_ipaddr.local_port));
		seq_printf(m, "\t\t\tMS Front  %-16s %-8d %-16s %-8d\n",
				ip2str(puser->front_ipaddr2.remote_ip), p2p(puser->front_ipaddr2.remote_port),
				ip2str(puser->front_ipaddr2.local_ip), p2p(puser->front_ipaddr2.local_port));
		seq_printf(m, "\t\t\t SM Back  %-16s %-8d %-16s %-8d\n",
				ip2str(puser->sipaddr.remote_ip), p2p(puser->sipaddr.remote_port),
				ip2str(puser->sipaddr.local_ip), p2p(puser->sipaddr.local_port));
		seq_printf(m, "\t\t\t SS Back  %-16s %-8d %-16s %-8d\n",
				ip2str(puser->sipaddr2.remote_ip), p2p(puser->sipaddr2.remote_port),
				ip2str(puser->sipaddr2.local_ip), p2p(puser->sipaddr2.local_port));
		seq_printf(m, "\t\t\tSM Front  %-16s %-8d %-16s %-8d\n",
				ip2str(puser->sfront_ipaddr.remote_ip), p2p(puser->sfront_ipaddr.remote_port),
				ip2str(puser->sfront_ipaddr.local_ip), p2p(puser->sfront_ipaddr.local_port));
		seq_printf(m, "\t\t\tSS Front  %-16s %-8d %-16s %-8d\n",
				ip2str(puser->sfront_ipaddr2.remote_ip), p2p(puser->sfront_ipaddr2.remote_port),
				ip2str(puser->sfront_ipaddr2.local_ip), p2p(puser->sfront_ipaddr2.local_port));
	}

	seq_printf(m, "\n\tterm num:%d\n", pms->term_nums);
	pterm = &pms->term[0];
	for(i=0; i<pms->term_nums; i++, pterm++)
	{
		seq_printf(m, "\t\tterm uac:0x%08x\n"
				"\t\t\t\033[32mIP Type  %-16s %-8s %-16s %-8s\033[0m\n",
				pterm->uac,
				"Remote IP", "Port",
				"Local IP", "Port");
		seq_printf(m, "\t\t\t   Back  %-16s %-8d %-16s %-8d\n",
				ip2str(pterm->ipaddr.remote_ip), p2p(pterm->ipaddr.remote_port),
				ip2str(pterm->ipaddr.local_ip), p2p(pterm->ipaddr.local_port));
		seq_printf(m, "\t\t\t  Front  %-16s %-8d %-16s %-8d\n",
				ip2str(pterm->front_ipaddr.remote_ip), p2p(pterm->front_ipaddr.remote_port),
				ip2str(pterm->front_ipaddr.local_ip), p2p(pterm->front_ipaddr.local_port));
		seq_printf(m, "\t\t\tS  Back  %-16s %-8d %-16s %-8d\n",
				ip2str(pterm->sipaddr.remote_ip), p2p(pterm->sipaddr.remote_port),
				ip2str(pterm->sipaddr.local_ip), p2p(pterm->sipaddr.local_port));
		seq_printf(m, "\t\t\tS Front  %-16s %-8d %-16s %-8d\n",
				ip2str(pterm->sfront_ipaddr.remote_ip), p2p(pterm->sfront_ipaddr.remote_port),
				ip2str(pterm->sfront_ipaddr.local_ip), p2p(pterm->sfront_ipaddr.local_port));
	}

	plinkid = &pms->linkid[0];
	seq_printf(m, "\n\tlinkid number:%d\n", pms->linkid_nums);
	for(i=0; i<pms->linkid_nums; i++, plinkid++)
	{
		seq_printf(m, "\t\tlinkid:0x%x\n", plinkid->linkid);
		seq_printf(m, "\t\t\tdata len:%d\n\t\t\trecv num:%d\n\t\t\tsend num:%d\n",
				plinkid->data_len, plinkid->recv_num, plinkid->send_num);
		seq_printf(m, "\t\t\tuser mask:0x%08x\n\t\t\tuser nums:%d\n",
				plinkid->user_mask, plinkid->userid_nums);
	}

	return 0;
}


static struct seq_operations ms_seq_ops = {
	.start	= ms_seq_start,
	.next	= ms_seq_next,
	.stop	= ms_seq_stop,
	.show	= ms_seq_show
};

static int ms_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &ms_seq_ops);
}

static struct file_operations ms_seq_file_ops = {
	.open		= ms_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static int _state_read(char *page, char **start, off_t off,
			int count, int *eof, void *data)
{
	int len = 0;
	extern int debug_ipaddr(char *page);
	len += sprintf(page+len, "   This UAC : 0x%08x\n", this_uac);
	len += sprintf(page+len, " SCHED MAIN : %s %d 0x%08x\n\n",
				SCHED_MAIN_MC_IP, SCHED_MAIN_MC_PORT, g_config.main_2_sched_uac);
	len += sprintf(page+len, " Mach state : %s\n",
			g_config.mach_state == ROLE_TYPE_MASTER_N?"Master":"Slave");
	len += sprintf(page+len, "  Mach Type : %s\n", g_config.mach_type==DEV_TYPE_ADSL?"ASDL":"ERROR");
	len += sprintf(page+len, "  hah cycle : %u\n  hah count : %u\n"
							 "  hah state : %u\n  msh cycle : %u\n"
							 "  msh count : %u\n   ms state : %u\n",
							 g_config.hah_cycle, g_config.hah_count,
							 g_config.hah_state, g_config.msh_cycle,
							 g_config.msh_count, g_config.ms_state);
	len += sprintf(page+len, "  save path : %s\n", g_config.local_save_path);

	len += debug_ipaddr(page+len);

	return len;
}

#if 0
typedef	int (read_proc_t)(char *page, char **start, off_t off,
			  int count, int *eof, void *data);
typedef	int (write_proc_t)(struct file *file, const char __user *buffer,
			   unsigned long count, void *data);
#endif
static int _if_read(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
	int len = 0;
	int i = 0;
	static const char *_eth_type_str[] = { "M", "HB", "HA", "S", "A", "B", "HB2" };

	len += sprintf(page+len, "type\tstate\tinterface\tip\n");
	for(i=0; i < ETH_TYPE_MAX; i++)
	{
		len += sprintf(page+len, "%4s\t %4d\t %8s\t%s\n",
				_eth_type_str[g_interface[i].type], g_interface[i].state,
				g_interface[i].interface, g_interface[i].ip_str);
	}

	for(i=0; i < 2; i++)
	{
		len += sprintf(page+len, "\ninterface: %s\n\ttype\t%-16s\tPort\n",
				g_hbif[i].interface, "IP");
		len += sprintf(page+len, "\tsend\t%-16s\t%5d\n",
				ip2str(g_hbif[i].send1_ip), p2p(g_hbif[i].send1_port));
		len += sprintf(page+len, "\tsend\t%-16s\t%5d\n",
				ip2str(g_hbif[i].send2_ip), p2p(g_hbif[i].send2_port));
		len += sprintf(page+len, "\trecv\t%-16s\t%5d\n",
				ip2str(g_hbif[i].recv_ip), p2p(g_hbif[i].recv_port));
	}

	return len;
}

static int _runtime_read(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
	int len = 0;
	struct timeval curtime;
	time_t runtime;
	int day, hour, min, sec;
	do_gettimeofday(&curtime);

	len += sprintf(page+len, "  start time : %8lu sec\n", _start_time);
	len += sprintf(page+len, "current time : %8lu sec\n", curtime.tv_sec);
	runtime = curtime.tv_sec - _start_time;
	day = runtime/(24*3600);
	hour = (runtime%(24*3600))/3600;
	min = (runtime%3600)/60;
	sec = runtime%60;
	len += sprintf(page+len, "    run time : %8lu sec\n\t %d day %d hour %d min %d sec\n",
			runtime, day, hour, min, sec);

	return len;
}

static int _debug_read(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
	int len = 0;
	len = sprintf(page, "debug=%d\n", debug);
	len += sprintf(page+len, "\nDebug Value:\n\t1 : local print valid\n"
			"\t2 : net print valid\n"
			"\t3 : both (net & local ) print valid\n"
			"\nwrite : echo 3 > this_file\n");
	return len;
}

static int _debug_write(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
	char flag = 0;
	if( count > 0 )
	{
		if( get_user(flag, buffer) )
			return -EFAULT;
		debug = flag - '0';
	}
	return count;
}

void debug_init(void)
{
	struct timeval curtime;
	do_gettimeofday(&curtime);
	_start_time = curtime.tv_sec;

	_proc_root = proc_mkdir("asdl", NULL);
	if( NULL == _proc_root )
	{
		dbg_all("error: create proc dir eeror\n");
		return;
	}

	runtime_entry = create_proc_read_entry("runtime", 0666, _proc_root,
			_runtime_read, NULL);
	if( NULL == runtime_entry )
	{
		dbg_all("create runtime proc entry error\n");
	}

	mission_entry = create_proc_entry("mission", 0666, _proc_root);
	if( NULL == mission_entry )
	{
		dbg_all("create mission proc entry error\n");
	}
	mission_entry->proc_fops = &ms_seq_file_ops;

	interface_entry = create_proc_read_entry("interface", 0666, _proc_root,
			_if_read, NULL);
	if( NULL == interface_entry )
	{
		dbg_all("create interface proc entry error\n");
	}

	state_entry = create_proc_read_entry("state", 0666, _proc_root,
			_state_read, NULL);
	if( NULL == state_entry )
	{
		dbg_all("create state proc entry error\n");
	}

	debug_entry = create_proc_entry("debug", 0666, _proc_root);
	if( NULL == debug_entry )
	{
		dbg_all("create debug proc entry error\n");
	}
	debug_entry->read_proc = _debug_read;
	debug_entry->write_proc = _debug_write;

	dbg_all("create debug entry over\n");
}

void debug_exit(void)
{
	remove_proc_entry("runtime", _proc_root);
	remove_proc_entry("mission", _proc_root);
	remove_proc_entry("interface", _proc_root);
	remove_proc_entry("state", _proc_root);
	remove_proc_entry("debug", _proc_root);

	remove_proc_entry("asdl", NULL);
	dbg_all("remove debug entry over\n");
}

