/*
 * =====================================================================================
 *
 *       Filename:  unix_socket.c
 *
 *    Description:  test unix socket
 *
 *        Version:  1.0
 *        Created:  07/19/2013 10:31:45 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  echo "Q2hhbmdxaW5nLFpoYW8K" | base64 -d  (tain)
 *			Email:	echo "Y2hhbmdxaW5nLjEyMzBAMTYzLmNvbQo=" | base64 -d 
 *        Company:  FreedomIsNotFree.com
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char **argv)
{
	int fd = 0;
	struct sockaddr_un dst_addr;
	int ret = 0;
	char buf[128];

	memset(&dst_addr, 0, sizeof(dst_addr));
	dst_addr.sun_family = AF_LOCAL;
	strncpy(dst_addr.sun_path, " localSocketServer", 
			sizeof(dst_addr.sun_path)-1);
	dst_addr.sun_path[0] = 0;

	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if( fd < 0 )
	{
		printf("error: create socket\n");
		return -1;
	}
	if( argc > 1 )
	{
		/* server */
		bind( fd, (struct sockaddr*)&dst_addr, sizeof(dst_addr));

		while(1)
		{
			memset(buf, 0, 128);
			ret = recv(fd, buf, 128, 0);
			printf("recv : %d %s\n", ret, buf);
		}
	}else{
		//while(1)
		{
			const char *hel = "hello,world";
			ret = sendto(fd, hel, strlen(hel), 0, (struct sockaddr*)&dst_addr, sizeof(dst_addr));
			printf("send: %d %s\n", ret, hel);
		}

	}

	close(fd);

	return 0;
}

