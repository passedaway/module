/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  test fp-hd1650.c
 *
 *        Version:  1.0
 *        Created:  2012年11月15日 15时33分38秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhao, Changqing (NO), changqing.1230@163.com
 *        Company:  iPanel TV inc.
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>


typedef struct {
	unsigned char hh;
	unsigned char mm;
	unsigned char ss;
	unsigned char mode; 
	/*mode:
	 * 0: do not display
	 * 1: show hh:mm
	 * other:not support, do mode 1
	 */
}fp_time_t;

#define MY_IOC_MAGIC    'Q'
#define IOCTL_FP_SET_DOT	_IOW(MY_IOC_MAGIC, 0, int)
#define IOCTL_FP_SET_TIME	_IOW(MY_IOC_MAGIC, 1, fp_time_t)
#define IOCTL_FP_GET_TIME	_IOR(MY_IOC_MAGIC, 2, fp_time_t)

int main(void)
{
	int fd = 0;
	fp_time_t fp_time;
	int times = 10;

	unsigned char key = 0;
	int dot = 0;

	const char *num = "0123456789";
	const char *str = "linux hello";

	fd = open("/dev/fp", O_RDWR);
	if( fd < 0 )
	{
		printf("error : open /dev/fp error.\n");
		return 0;
	}

	printf("1.test show num\n");
	printf("  set dot on\n");
	dot = 1;
	ioctl(fd, IOCTL_FP_SET_DOT, &dot);
	write(fd, num, strlen(num));

	printf("2.test show string\n");
	printf("  set led on\n");
	getchar();
	dot = 3;
	ioctl(fd, IOCTL_FP_SET_DOT, &dot);
	write(fd, str, strlen(str));

	printf("3.set time\n");
	printf("  set led & dot off\n");
	getchar();
	dot = 0;
	ioctl(fd, IOCTL_FP_SET_DOT, &dot);
	ioctl(fd, IOCTL_FP_GET_TIME, &fp_time);
	printf("get time: %d:%d:%d mode=%d\n", fp_time.hh, fp_time.mm, fp_time.ss, fp_time.mode);
	printf("set time: 11:59:55 mode=1\n");
	fp_time.hh = 11;
	fp_time.mm = 59;
	fp_time.ss = 55;
	fp_time.mode = 1;
	ioctl(fd, IOCTL_FP_SET_TIME, &fp_time);

	printf("4.get key\n");
	getchar();
	while(times--)
	{
		printf("wait key...\n");
		read(fd, &key, 1);
		printf("key = %d  repeat : %s.\n", key, key&0x1?"no":"yes");
	}

	return 0;
}

