#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/signal.h>


#define TS_OSC          5000 // Quartz Osc wake up time, max 5 ms
#define TS_FS           800 // Freq Synth wake-up time from OS, max 800 us
#define TS_RE           500 // Receiver wake-up time from FS, max 500 us

typedef enum{
	M_SLEEP		= 0,
	M_STANDBY 	= 0x20,
	M_FS		= 0x40,
	M_RX		= 0x60,
}MODE_e;


static int sx1213_cfg_fd, sx1213_data_fd;

static unsigned char read_register(unsigned char address);
static void write_register(unsigned char address, unsigned char data);

static int sx1213_init(void);
/* read 13 register , check /fifoempty bit
* ret : 0 is empty
* 1 have data, can read
*/
static int sx1213_status(void);
static unsigned char receive_byte(void);
/* return realy receive bytes*/
static int receive_bytes(unsigned char *buf, int num);

static void signal_handler(int sig_id);

#if 0
unsigned char  sx1213_regs[] = { /* SX1213 configuration registers values*/
/* Main Coniguration Register - MCParam 0-0x0b 0 - 11*/
		0X0F,	/* 0 sleep mode */
		0XA4, 	/* 1 */
		0X03, 	/* 2 */
		0X07, 	/* 3 */
		0X1F, 	/* 4 */

		0XC6, 	/* 5 */
		0X6B, 	/* 6 */
    	0X2A, 	/* 7 */
    	0X1E, 	/* 8 */
    	0X77, 	/* 9 */
    	0X2F, 	/* 10 */
    	0X19, 	/* 11 */


	/* Interrupt Configuration Register - IRQParam 0x0c - 0xe  12 - 14*/
    	0X0F, 	/* 12 fifo size-tresh*/
    	0X80, 	/* 13 irq param1*/
    	0X0B, 	/* 14 irq param2*/

	/* RSSIRrqThres 0xf (do not write) 15*/ /* never see it in sx1213 datasheet*/
		0x00, 	/* 15 */

  	/* Receiver Configuration Register - RXParam  0x10 - 0x15 16 - 21*/
    	0XA3, 	/* 16  rx param1*/
   		0X38, 	/* 17  rx param2*/
    	0X18, 	/* 18  rx param3 sync byte sizes*/
		0X04, 	/* 19  */
   		0X00, 	/* 20  rssi value*/
    	0X00, 	/* 21  rx param6*/

	/* Sync Word -SYNCParam  0x16 - 0x19 22 - 25*/
    	0XFE, 	/* 22  sync byte1*/
    	0X00, 	/* 23  sync byte2*/
    	0X00, 	/* 24  sync byte3*/
    	0X00, 	/* 25  sync byte4*/

	/* tx param 0x1a (do not write ) 26*/ /* never see it in sx1213 datasheet*/
		0x72, 	/* 26  tx param*/

    /* Oscillator Parameters - OSCParam 0x1b  27*/
   		0XBC, 	/* 27  osc param*/

	/* Packet Handling Parameters - PKTParam 0x1c - 0x1f  28 - 31*/
    	0X01, 	/* 28 Pkt param1*/
    	0X00, 	/* 29 node addr*/
    	0X48, 	/* 30 Pkt param3*/
    	0X00 	/* 31 pkt param4*/
    	/* 15 19 20 26 is not should be write*/
};
#else
/* ref reg config*/
unsigned char  sx1213_regs[] = { /* SX1213 configuration registers values*/
/* Main Coniguration Register - MCParam 0-0x0b 0 - 11*/
		0X0F,	/* 0 sleep mode */
		0XA4, 	/* 1 */
		0X03, 	/* 2 */
		0X07, 	/* 3 */
		0X1F, 	/* 4 */

		0XC6, 	/* 5 */
		0X6B, 	/* 6 */
    	0X2A, 	/* 7 */
    	0X1E, 	/* 8 */
    	0X77, 	/* 9 */
    	0X2F, 	/* 10 */
    	0X19, 	/* 11 */


	/* Interrupt Configuration Register - IRQParam 0x0c - 0xe  12 - 14*/
    	0X0F, 	/* 12 fifo size-tresh*/
    	//0X01, 	/* 13 irq param1*/
    	0x81,
    	0X0B, 	/* 14 irq param2*/

	/* RSSIRrqThres 0xf (do not write) 15*/ /* never see it in sx1213 datasheet*/
		0x00, 	/* 15 */

  	/* Receiver Configuration Register - RXParam  0x10 - 0x15 16 - 21*/
    	0XA3, 	/* 16  rx param1*/
   		0X38, 	/* 17  rx param2*/
    	0X18, 	/* 18  rx param3 sync byte sizes*/
		0X04, 	/* 19  */
   		0X00, 	/* 20  rssi value*/
    	0X00, 	/* 21  rx param6*/

	/* Sync Word -SYNCParam  0x16 - 0x19 22 - 25*/
    	0XFE, 	/* 22  sync byte1*/
    	0X00, 	/* 23  sync byte2*/
    	0X00, 	/* 24  sync byte3*/
    	0X00, 	/* 25  sync byte4*/

	/* tx param 0x1a (do not write ) 26*/ /* never see it in sx1213 datasheet*/
		0x72, 	/* 26  tx param*/

    /* Oscillator Parameters - OSCParam 0x1b  27*/
   		0XBC, 	/* 27  osc param*/

	/* Packet Handling Parameters - PKTParam 0x1c - 0x1f  28 - 31*/
    	0X01, 	/* 28 Pkt param1*/
    	0X00, 	/* 29 node addr*/
    	0X49, 	/* 30 Pkt param3*/
    	0X00 	/* 31 pkt param4*/
    	/* 15 19 20 26 is not should be write*/
};
#endif

void set_mode(unsigned char mode)
{
	static unsigned char last_mode = M_SLEEP;

	if( mode == last_mode )
		return;

	if( mode == M_SLEEP )
	{
		unsigned char data = sx1213_regs[0];
		data &= 0x1F;
		data |= mode;
		write_register(0, data);
	}

	if( mode == M_STANDBY )
	{
		unsigned char data = sx1213_regs[0];
		data &= 0x1F;
		data |= mode;

		write_register(0, data);
		usleep(500*1000);
	}

	if( mode == M_FS )
	{
		unsigned char data = sx1213_regs[0];

		if( last_mode == M_SLEEP )
		{
			data &= 0x1F;
			data |= M_STANDBY;
			write_register(0, data);
			usleep(500*1000);

			last_mode = M_STANDBY;
		}

		if( last_mode == M_STANDBY )
		{
			data &= 0x1F;
			data |= M_FS;
			write_register(0, data);
			usleep(500*1000);
		}
	}

	if( mode == M_RX )
	{
		unsigned char data = sx1213_regs[0];

		if( last_mode == M_SLEEP )
		{
			data &= 0x1F;
			data |= M_STANDBY;
			write_register(0, data);
			usleep(500*1000);

			last_mode = M_STANDBY;
		}

		if( last_mode == M_STANDBY )
		{
			data &= 0x1F;
			data |= M_FS;
			write_register(0, data);
			usleep(500*1000);

			last_mode = M_FS;
		}

		if( last_mode == M_FS )
		{
			data &= 0x1F;
			data |= M_RX;
			write_register(0, data);
			usleep(500*1000);
		}
	}

	last_mode = mode;
}

void write_register(unsigned char address, unsigned char data)
{
	if( sx1213_cfg_fd )
	{
		unsigned char buf[2];
		buf[0] = address;
		buf[1] = data;
		write(sx1213_cfg_fd, buf, 2);
#if 1
		printf("write  0x%02x  0x%02x\n", buf[0], buf[1]);
#endif
	}
}

unsigned char read_register(unsigned char address)
{
	if( sx1213_cfg_fd )
	{
		unsigned char data[2];
		data[0] = address;
		read(sx1213_cfg_fd, data, 1);
		return data[0];
	}

	return -1;
}

unsigned char receive_byte(void)
{
	if( sx1213_data_fd )
	{
		unsigned char data = 0;
		read(sx1213_data_fd, &data, 1);
		return data;
	}

	return 0;
}


int receive_bytes(unsigned char *buf, int num)
{
	if( !buf )
		return -1;

	if( num > 64 )
		return -1;

	if( sx1213_data_fd )
	{
		int i = 0;

		printf("[%s][%d]\n", __FUNCTION__, __LINE__);
		for( ; i < num ; i++)
		{
			if( sx1213_status() )
			{
				*buf = receive_byte();
				buf++;
			}
			else
				break;
		}

		return i;
	}

	return -1;
}

int sx1213_init(void)
{
	int i = 0;

	/* open sx1213 config */
	if( !sx1213_cfg_fd )
	{
		sx1213_cfg_fd = open( "/dev/sx1213_config", O_RDWR);
		if(sx1213_cfg_fd < 0 )
		{
			printf("Error : open /dev/sx1213_config!\n");
			return -1;
		}
	}

	/* open sx1213 data */
	if( !sx1213_data_fd )
	{
		sx1213_data_fd = open( "/dev/sx1213_data", O_RDWR);
		if(sx1213_data_fd < 0 )
		{
			printf("Error : open /dev/sx1213_data!\n");
			return -1;
		}
	}

	/* write sx1213 config regs */
	for( i = 0; i <  32; i++)
	{
		if( i==20 )
			continue;

		write_register(i, sx1213_regs[i]);
	}

	/*set in sleep mode*/
	set_mode(M_SLEEP);

	/* success */
	return 0;
}

int sx1213_status(void)
{
	if( sx1213_cfg_fd )
	{
		unsigned char reg = 13, fifo_empty = 0;
		read(sx1213_cfg_fd, &reg, 1);

		printf("[%s][%d] 13 0x%02x", __FUNCTION__, __LINE__, reg);
		fifo_empty = reg & 0x01;

		return fifo_empty;
	}

	/* cannot read */
	return 0;
}


void signal_handler(int sig_id)
{
	printf("\n[%s] [%d] signo = %d\n", __FUNCTION__, __LINE__, sig_id);

	set_mode(M_SLEEP);

	if( sx1213_cfg_fd )
		close(sx1213_cfg_fd);
	if( sx1213_data_fd )
		close(sx1213_data_fd);

	exit(0);
}

int main(int argc, char **argv)
{
	unsigned char buf[2];
	int i, ret;
	unsigned char regs[32];

	/* program init, when CTRL+C , goto signal_handler() to exit */
	signal( SIGINT, signal_handler);

	/* 1.init sx1213 */
	if( sx1213_init() )
	{
		printf("Error : init sx1213 error!\n");
		return -1;
	}

	/*for debug*/
#if 1
	printf("read back\n");
	for( i = 0; i < 32; i++)
	{
		buf[0] = (unsigned char)i;
		regs[i] = read_register(buf[0]);
		printf("%d    0x%02x   0x%02x\n", i,  buf[0], regs[i]);
	}
#endif

	/*2. set sx1213 in Rx mode */
	set_mode(M_RX);

	/* 3. wait for CRC_OK interrupt */


	/* 4. go to Stby mode */
	/* read payload bytes from FIFO until /fifoempty goes low. */

	/* 5. go to sleep mode */
	/* exit */

	printf("do while 1\n");
	while(1)
	{
		#if 0
		for( i = 0; i < 32; i++)
		{
			usleep(1000);

			buf[0] = i;
			read(sx1213_cfg_fd, buf, 1);

			if( regs[i] != buf[0] )
			{
				printf("\nreg index: %d  new value: 0x%02x   old value: 0x%02x\n",
					i, buf[0], regs[i]);
				regs[i] = buf[0];
			}

			printf(".");
			fflush(stdout);
		}

		#else

		buf[0] = 13;
		read(sx1213_cfg_fd, buf, 1);
		//printf("buf 0x%02x 0x%02x\n", buf[0], buf[1]);
    	if(buf[0])
    	{
    		unsigned char fifo_full, fifo_empty;

    		fifo_full = ((unsigned char )buf[0] & 0x02)>>1;
    		fifo_empty = ((unsigned char )buf[0] & 0x01);

    		printf("FIFO  full %d   empty %d   data = 0x%02x\n", fifo_full, fifo_empty, buf[0]);
    		fflush(stdout);

    		if( fifo_empty )
    		{
    			unsigned char rx_data[4];

    			set_mode(M_STANDBY);

    			memset(rx_data, 0, sizeof(rx_data));
    			#if 0
    			ret = receive_bytes(rx_data,4);
    			if( ret > 0 )
    			{
    				int i;
    				printf("receive data:\n");
    				for( i = 0; i < ret; i++)
    				{
    					printf("0x%02x ", rx_data[i]);
    				}
					printf("    ;   ");
    				for(i = 0; i < ret; i++)
    				{
    					printf("%c ", rx_data[i]);
    				}

    				printf("\n");
    				fflush(stdout);
    			}
    			#else
    			rx_data[0] = receive_byte();
    			printf("receive data:\n0x%02x ; %c\n", rx_data[0], rx_data[0]);
    			#endif
    			usleep(200*1000);

				set_mode(M_RX);
    		}
    	}
    	usleep(200*1000);

    	#endif
	}

	/* end of while 1, by Ctrl+C */

	return 0;
}
