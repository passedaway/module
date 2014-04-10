#include "gpio_i2c.h"

/*show define*/
/******************************************************************
  a
  __
f|__|b
e|__|c .dop  中间那段为g
d
*******************************************************************/

//数码管末一段的值
#define BA		0x1
#define BB		0x1<<1
#define BC		0x1<<2
#define BD		0x1<<3
#define BE		0x1<<4
#define BF		0x1<<5
#define BG		0x1<<6
#define BP		0x1<<7
 
#if 0
#define BA	0x1<<7
#define BB	0x1<<6
#define BC	0x1<<5
#define BD	0x1<<4
#define BE	0x1<<3
#define BF	0x1<<2
#define BG	0x1<<1
#define BP	0x1
#endif

//数码管显示的字符的值
#define TM_A		BF | BA | BB | BG | BE | BC
#define TM_B		BF | BE | BG | BD | BC
#define TM_C		BG | BE | BD
#define TM_D		BB | BC | BD | BE | BG
#define TM_E		BA | BF | BG | BE | BD
#define TM_G		BA | BF | BE | BD | BC
#define TM_F		BA | BF | BG | BE
#define TM_H		BF | BE | BG | BC
#define TM_I		BB | BC
#define TM_J		BB | BC | BD | BE
#define TM_K		BF | BG | BE | BC | BB
#define TM_L		BF | BE | BD
#define TM_M		0
#define TM_N		BE | BG | BC
#define TM_O		BG | BC | BD | BE
#define TM_P		BA | BB |BG | BF |BE
#define TM_Q		BF | BA | BB | BG | BC
#define TM_R		BE | BG
#define TM_S		BA | BF | BG | BC | BD
#define TM_T		BF | BG | BE | BD
#define TM_U		BF | BE | BD | BC | BB
#define TM_V		BE | BD | BC
#define TM_W		0
#define TM_X		0
#define TM_Y		0
#define TM_Z		0

#define TM_0		BA | BB | BC | BD | BE | BF
#define TM_1		BB | BC
#define TM_2		BA | BB | BG | BE | BD
#define TM_3		BA | BB | BC | BD | BG
#define TM_4		BF | BG | BB | BC
#define TM_5		BA | BF | BG | BC | BD
#define TM_6		BA | BF | BG | BE | BD | BC
#define TM_7		BF | BA | BB | BC
#define TM_8		BA | BB | BC | BD | BE | BF | BG
#define TM_9		BA | BB | BC | BD | BG | BF

static const unsigned char TM_NUM[] = {TM_0, TM_1, TM_2, TM_3, TM_4,
TM_5, TM_6, TM_7, TM_8, TM_9}; //定义键值

static unsigned char const TM_CHAR[] = {TM_A, TM_B, TM_C, TM_D, TM_E, TM_F, TM_G,
TM_H, TM_I, TM_J, TM_K, TM_L, TM_M, TM_N, 
TM_O, TM_P, TM_Q, TM_R, TM_S, TM_T, TM_U, 
TM_V, TM_W, TM_X, TM_Y, TM_Z};


/*************************These define no need to reimplement *****************/
/* this define copy from datasheet */
#define CMD_SYSOFF	0x4800
#define CMD_SYSON	0x4801
#define CMD_SLEEPOFF	0x4800
#define CMD_SLEEPON	0x4804
#define CMD_7SEGON	0x4809
#define CMD_8SEGON	0x4801

#define CMD_7SEGON1	0x4819
#define CMD_7SEGON2	0x4829
#define CMD_7SEGON3	0x4839
#define CMD_7SEGON4	0x4849
#define CMD_7SEGON5	0x4859
#define CMD_7SEGON6	0x4869
#define CMD_7SEGON7	0x4879
#define CMD_7SEGON8	0x4809

#define CMD_8SEGON1	0x4811
#define CMD_8SEGON2	0x4821
#define CMD_8SEGON3	0x4831
#define CMD_8SEGON4	0x4841
#define CMD_8SEGON5	0x4851
#define CMD_8SEGON6	0x4861
#define CMD_8SEGON7	0x4871
#define CMD_8SEGON8	0x4801

#define CMD_DIG0(x)	0x6800 | (x)
#define CMD_DIG1(x)	0x6A00 | (x)
#define CMD_DIG2(x)	0x6C00 | (x)
#define CMD_DIG3(x)	0x6E00 | (x)
#define CMD_GETKEY	0x4F00


/*************************No Need To Re Implement ****************************/
static unsigned char hd1650_sendcmd(unsigned short cmd);
static unsigned char asc2code(unsigned char src);

/*****************************global function *********************************/
/* for porting this code, platform init */
/*  this is mips - brcm 7231 implemetnion , other platform should modify this */
void hd1650_platform_init(void)
{
	/* init pin mux, some platform do not need this */
	/*  set pin mux */
	unsigned long temp = 0;
	temp = REG(CLK_PIN_MUX);
	temp &= 0x0fffffff;
	REG(CLK_PIN_MUX) = temp;

	temp = REG(SDA_PIN_MUX);
	temp &= 0xfffffff0;
	REG(SDA_PIN_MUX) = temp;

	/* set clk & sda out, and set clk-h sda-h */
	/*  set sda oden, dir, data */
	set_gpio_data(SDA_ODEN, SDA_PIN, 1);
	set_gpio_data(SDA_DIR, SDA_PIN, 0);
	set_gpio_data(SDA_DATA, SDA_PIN, 1);

	/* set clk oden, dir, data */
	set_gpio_data(CLK_ODEN, CLK_PIN, 1);
	set_gpio_data(CLK_DIR, CLK_PIN, 0);
	set_gpio_data(CLK_DATA, CLK_PIN, 1);

	/* now clk & sda is ready  */
}

/* do not modify this */
void hd1650_init(void)
{
	//hd1650_sendcmd(CMD_SYSON);
	hd1650_sendcmd(CMD_8SEGON);

	/* clear 4 segment */
	hd1650_sendcmd(CMD_DIG0(0x00));
	hd1650_sendcmd(CMD_DIG1(0x00));
	hd1650_sendcmd(CMD_DIG2(0x00));
	hd1650_sendcmd(CMD_DIG3(0x00));
}

void hd1650_show_each(unsigned char data, unsigned char pos,unsigned char dot_flag)
{
	unsigned char tmpData;
	tmpData = asc2code(data);
	switch(pos)
	{
		case 1:
			hd1650_sendcmd(CMD_DIG0(tmpData));
			break;
		case 2:
			if(dot_flag)
				hd1650_sendcmd(CMD_DIG1(tmpData|0x80));
			else
				hd1650_sendcmd(CMD_DIG1(tmpData&0x7f));
			break;
		case 3:
			hd1650_sendcmd(CMD_DIG2(tmpData));
			break;
		case 4:
			hd1650_sendcmd(CMD_DIG3(tmpData));
			break;
	}
}


void hd1650_show(unsigned char *data,unsigned char dot_flag)
{
	if( data )
	{
		hd1650_sendcmd(CMD_DIG0(asc2code(*data++)));
		if(dot_flag&0x01)
			hd1650_sendcmd(CMD_DIG1(asc2code(*data++)|0x80));
		else
			hd1650_sendcmd(CMD_DIG1(asc2code(*data++)));
		hd1650_sendcmd(CMD_DIG2(asc2code(*data++)));
		if(dot_flag&0x02)
			hd1650_sendcmd(CMD_DIG3(asc2code(*data)|0x80));
		else
			hd1650_sendcmd(CMD_DIG3(asc2code(*data)));
	}
}

unsigned char hd1650_getkey(unsigned char *key)
{
	unsigned char tmp = 0;

	
	tmp = hd1650_sendcmd( CMD_GETKEY );

	if((tmp & 0x40)== 0)
		tmp = 0x2e;

	if( key )
		*key = tmp;

	return tmp;
}



/*****************************local function implemention*********************************/

static unsigned char hd1650_sendcmd(unsigned short cmd)
{
	unsigned char tmp_data = cmd>>8;

	i2c_start();
	i2c_send(tmp_data);
	if(1 != i2c_get_ack() )
	{
		/* printf some error
		* hd1650 didnot send the ack
		*/
	}

	if( cmd == CMD_GETKEY )
	{
		i2c_recv(&tmp_data);
		if(1 != i2c_get_ack_getkey())
		{
			/* printf some error
			* hd1650 didnot send the ack
			*/
		}
	}else{
		tmp_data = cmd&0x0ff;
		i2c_send(tmp_data);
		if(1 != i2c_get_ack())
		{
			/* printf some error
			* hd1650 didnot send the ack
			*/
		}
	}

	
	i2c_stop();

	return tmp_data;/* just valid for the CMD_GETKEY */
}


static unsigned char asc2code(unsigned char src)
{
#if 0	
	/*  zhaocq */
	if(src <= 9)
		return TM_NUM[src];
	else 
#endif
		if(src >= '0' && src <= '9')
		return TM_NUM[src - '0'];
	else if(src >= 'a' && src <= 'z')
		return TM_CHAR[src - 'a'];
	else if(src >= 'A' && src <= 'Z')
		return TM_CHAR[src - 'A'];
	else
		return 0;
	
}

/*******END OF THE FILE *********/

