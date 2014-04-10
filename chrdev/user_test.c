#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>

/*  select, need 3 include files  */
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#define MY_IOC_MAGIC    'Q'
#define C_RESET     _IO(MY_IOC_MAGIC, 0)
#define C_GETRP     _IOR(MY_IOC_MAGIC, 1, int)
#define C_GETWP     _IOR(MY_IOC_MAGIC, 2, int)

int main(int argc, char **argv)
{
	int fd, ret=0;
	int out_data;

	fd_set fds_r;

	char buf[128];
	fd = open("/dev/mycdev", O_RDWR);
	if( fd < 0 )
	{
		printf("error : cannot open.\n");
		return 0;
	}

	FD_ZERO(&fds_r);

	while( 1 )
	{
#if 0
		/*  normal read /write test */
		printf("will read\n");
		ret = read(fd, buf, 128);
		printf("read success:%d\n",ret);
//		sleep(1);
		printf("test ioctl : C_GETRP\n");
		ret = ioctl(fd, C_GETRP, &out_data);
		printf(" ret = %d out_data=%08x\n", ret, out_data);
//		sleep(1);
		printf("test ioctl : C_GETWP\n");
		ret = ioctl(fd, C_GETWP, &out_data);
		printf(" ret = %d out_data=%08x\n", ret, out_data);
#endif

#if 1
		/* select driver poll test  */
		printf("before select\n");
		FD_SET(fd, &fds_r);
		ret = select(fd+1, &fds_r, NULL, NULL, NULL);
		printf("select over:ret=%d \n", ret);

		if( FD_ISSET(fd, &fds_r) )
		{
			printf("can read. exit\n");
			return 0;
		}
#endif
	}

	close(fd);
	return 0;
}
