/*
 * =====================================================================================
 *
 *       Filename:  omni_socket.h
 *
 *    Description:  socket in kernel
 *
 *        Version:  1.0
 *        Created:  04/28/2013 10:15:04 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Changqing,Zhao ,changqing.1230@163.com
 *        Company:  None
 *
 * =====================================================================================
 */

#ifndef _OMNI_SOCKET_H_
#define _OMNI_SOCKET_H_

#include <linux/mm.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/inet.h>
//#include <linux/inet_common.h>
#include <net/sock.h>
#include <linux/if.h>

int sock_create_tcp(struct socket **sock);
int sock_create_udp(struct socket **sock);

int sock_create_tcpserver(struct socket **sock, const char *server_ipaddr, const int server_port);
int sock_create_udpserver(struct socket **sock, const char *server_ipaddr, const int server_port);

int sock_create_multicast_writer(struct socket **sock, const char *local_if, const char *local_ip, const int local_port);
int sock_create_multicast_reader(struct socket **sock, const char *mc_ipaddr, const int mc_port, const char *local_if);

int sock_create_srcmulticast_writer(struct socket **sock, const char *local_if, const char *local_ip, const int local_port);
int sock_create_srcmulticast_reader(struct socket **sock, const char *mc_ipaddr, const int mc_port, const char *local_if, const char *src_ipaddr);


void sock_make_sockaddr_in(const char *ipaddr, const int port, struct sockaddr_in *sockaddr_out);

int sock_read(struct socket *sock, void *buf, int len, struct sockaddr_in *sockaddr);
int sock_write(struct socket *sock, void *buf, int len, struct  sockaddr_in *sockaddr);

int sock_tcpsrv_waitfor_cli(struct socket *server_sock, struct socket **client_sock, int flags);
int sock_tcpcli_connect_srv(struct socket *client, const char *server_ip, const int server_port);
int sock_udpcli_connect_srv(struct socket *client, const char *server_ip, const int server_port);

/* just extern
 * because this is same with kernel
 * */
extern void sock_release(struct socket *sock);

#endif

