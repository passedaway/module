/*
 * =====================================================================================
 *
 *       Filename:  gpio_i2c.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2012年11月14日 18时30分32秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhao, Changqing ,changqing.1230@163.com
 *        Company:  iPanel TV inc.
 *
 * =====================================================================================
 */
#ifndef _GPIO_I2C_H_
#define _GPIO_I2C_H_

/*  this broadcom-7231 implemention  */
#define PHYS_TO_K0(x)   ((x) | 0x80000000)
#define PHYS_TO_K1(x)   ((x) | 0xa0000000)

#define REG(a)  			(*((volatile unsigned long *)(PHYS_TO_K0(a))))

/* this is brcm data*/
#define CLK_PIN		0
#define SDA_PIN		1

#define CLK_ODEN 0x10408c40	
#define SDA_ODEN 0x10408c40	
#define CLK_DATA 0x10408c44
#define SDA_DATA 0x10408c44	
#define CLK_DIR	 0x10408c48
#define SDA_DIR	 0x10408c48
#define SDA_IRQ	 0x10408c54
#define CLK_IRQ	 0x10408c54
#define CLK_PIN_MUX	0x10408508
#define SDA_PIN_MUX	0x1040850c

/***********************  gpio operation  ********************************/
static int set_gpio_data(unsigned long reg, int offset, int data)
{
	unsigned long tmp = REG(reg);
	if( data )
	{
		if( tmp & (1<<offset) )
			return 1;
		else
			tmp |= 1<< offset;
	}else{
		if( tmp & (1<<offset) )
			tmp &= ~(1<<offset);
		else
			return 0;
	}

	REG(reg) = tmp;
	return data;
}

static inline int get_gpio_data(unsigned long reg, int offset)
{
	unsigned long tmp = REG(reg);
	return (tmp&(1<<offset)) ? 1 : 0;
}

/*****************************************************************************
每个平台须需实现下面的几个宏，可以参考该demo文件里的实现
实现以下几个宏之后，还需要根据具体的平台调整延时时间
*****************************************************************************/
#define CLK_OUT()	set_gpio_data(CLK_DIR, CLK_PIN, 0)
#define CLK_H()		set_gpio_data(CLK_DATA, CLK_PIN, 1)
#define CLK_L()		set_gpio_data(CLK_DATA, CLK_PIN, 0)

#define SDA_OUT()   set_gpio_data(SDA_DIR, SDA_PIN, 0)
#define SDA_IN()	set_gpio_data(SDA_DIR, SDA_PIN, 1)

#define SDA_H()		set_gpio_data(SDA_DATA, SDA_PIN, 1)
#define SDA_L()		set_gpio_data(SDA_DATA, SDA_PIN, 0)
#define GET_SDA()	get_gpio_data(SDA_DATA, SDA_PIN)

#include <linux/delay.h>
#define DELAY_BUILD()	udelay(1)
#define DELAY()			udelay(10)

/*****************************local function *********************************/
/*****************************DO NOT MODIFY*********************************/
static void i2c_start(void)
{
	CLK_OUT();
	SDA_OUT();

	SDA_H();
	DELAY();
	CLK_H();
	DELAY_BUILD();
	SDA_L();
	DELAY();
}

static void i2c_stop(void)
{
	SDA_OUT();
	SDA_L();
	DELAY();

	CLK_H();
	DELAY_BUILD();
	SDA_H();
	DELAY();
}

/* MSB */
static void i2c_send(unsigned char data)
{
	unsigned char i = 0;
	for(; i < 8 ; i++)
	{
		CLK_L();
		DELAY_BUILD();
		if( data & 0x80 )
			SDA_H();
		else
			SDA_L();
		data <<= 1;
		DELAY();
		CLK_H();
		DELAY();
	}
}

static unsigned char i2c_recv(unsigned char *data)
{
	unsigned char i = 0, tmp=0;
	//SDA_L();
	SDA_IN();
	for(; i < 8 ; i++)
	{
		CLK_L();
		DELAY();
		CLK_H();
		DELAY_BUILD();
		tmp <<= 1;
		tmp |= GET_SDA();
		
		DELAY();
	}
	SDA_OUT();
	#if 0
	CLK_L();
	SDA_OUT();
	SDA_L();
	SDA_IN();
	DELAY();
	CLK_H();
	DELAY();
	#endif

	if( data )
		*data = tmp;
	return tmp;
}

static int i2c_get_ack(void)
{
	int i = 30;

	CLK_L();
	//SDA_OUT();
	//SDA_L();
	SDA_IN();
	//CLK_L();
	DELAY_BUILD();
	
	CLK_H();
	DELAY();
	while(GET_SDA() && i-- );
	//DELAY();
	CLK_L();
	SDA_OUT();

	return 1;/*!!!Fixme. this should return the right value, but sometimes the ack cannot get */
}

static int i2c_get_ack_getkey(void)
{
	int i = 30;

	CLK_L();
	//SDA_OUT();
	//SDA_L();
	SDA_IN();
	//CLK_L();
	DELAY_BUILD();
	
	CLK_H();
	DELAY();
	while(!GET_SDA() && i-- );
	CLK_L();
	SDA_OUT();

	return 1;/*!!!Fixme. this should return the right value, but sometimes the ack cannot get */
}

#endif

