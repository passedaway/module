#include <linux/module.h>
#include "spi.h"

#define PHYS_TO_K0(x)   ((x) | 0x80000000)
#define PHYS_TO_K1(x)   ((x) | 0xa0000000)

#define REG(a)  			(*((volatile unsigned long *)(a)))

#define SPI_PIN_MUX_REG		0x10404130
#define SPI_ODEN_REG		0x10406660
#define SPI_DATA_REG		0x10406664
#define SPI_DIR_REG			0x10406668

#define MOSI_PIN			11
#define SCK_PIN				13
#define MISO_PIN			14
#define DATA_PIN			12
#define CONFIG_PIN			15

#define SPI_REG(a) 	REG(PHYS_TO_K0(a))
#define KEEP_CLK()			usleep(0x800)

/* dir */
#define SPI_CLK_OUT()		set_gpio_data(SPI_DIR_REG, SCK_PIN, 0)
#define SPI_MOSI_OUT()		set_gpio_data(SPI_DIR_REG, MOSI_PIN, 0)
#define SPI_DATA_OUT()		set_gpio_data(SPI_DIR_REG, DATA_PIN, 0)
#define SPI_CONFIG_OUT()	set_gpio_data(SPI_DIR_REG, CONFIG_PIN, 0)
#define SPI_MISO_IN()		set_gpio_data(SPI_DIR_REG, MISO_PIN, 1)
/* data H / L */
#define SPI_CLK_H()				set_gpio_data(SPI_DATA_REG, SCK_PIN, 1)
#define SPI_MOSI_H()			set_gpio_data(SPI_DATA_REG, MOSI_PIN, 1)
#define SPI_DATA_H()			set_gpio_data(SPI_DATA_REG, DATA_PIN, 1)
#define SPI_CONFIG_H()			set_gpio_data(SPI_DATA_REG, CONFIG_PIN, 1)
#define SPI_CLK_L()				set_gpio_data(SPI_DATA_REG, SCK_PIN, 0)
#define SPI_MOSI_L()			set_gpio_data(SPI_DATA_REG, MOSI_PIN, 0)
#define SPI_DATA_L()			set_gpio_data(SPI_DATA_REG, DATA_PIN, 0)
#define SPI_CONFIG_L()			set_gpio_data(SPI_DATA_REG, CONFIG_PIN, 0)
#define GET_SPI_DATA()			get_gpio_data(SPI_DATA_REG, MISO_PIN)

/***********************  gpio operation  ********************************/
int set_gpio_data(unsigned long reg, int offset, int data)
{
	unsigned long tmp = SPI_REG(reg);
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

	SPI_REG(reg) = tmp;
	return data;
}

int get_gpio_data(unsigned long reg, int offset)
{
	unsigned long tmp = SPI_REG(reg);
	return (tmp&(1<<offset)) ? 1 : 0;
}


/*************************    spi operation   ****************************/
static void usleep(unsigned long times)
{
	volatile unsigned long i = times;
	for(; i != 0; i-- )
	;
}

unsigned char spi_read(unsigned char *data)
{
	char i = 0;
	for(; i < 8; i++)
	{
		SPI_CLK_L();
		KEEP_CLK();
		SPI_CLK_H();
#if 0
		if( GET_SPI_DATA() )
			*data |= 1;
		*data <<= 1;
#else
		*data <<= 1;
		if( GET_SPI_DATA() )
			*data |= 1;
#endif
		KEEP_CLK();
	}
	SPI_CLK_L();
	return *data;
}

void spi_write(unsigned char data)
{
	char i = 0;

	for(; i < 8; i++)
	{
		SPI_CLK_L();
		if( data & 0x80 )
		{
			SPI_MOSI_H();
		}else{
			SPI_MOSI_L();
		}
		data <<= 1;
		KEEP_CLK();

		SPI_CLK_H();
		KEEP_CLK();
	}
	SPI_CLK_L();
}

/*****************sx1213 driver********************/
unsigned char read_register(unsigned char address)
{
	unsigned char data = 0;
	address = ( ((address<<1)&0x7E) | 0x40 );

	SPI_DATA_H();
	SPI_CONFIG_L();
	KEEP_CLK();
	spi_write(address);
	spi_read(&data);
	KEEP_CLK();
	SPI_CONFIG_H();
	KEEP_CLK();
/*
	printk("read_register address 0x%02x data 0x%02x \n", address, data);
*/
	return data;
}

void write_register(unsigned char address, unsigned char data)
{
	address = ( (address<<1)&0x3E );
/*
	printk("write_register address 0x%02x data 0x%02x \n", address, data);
*/
	SPI_DATA_H();
	SPI_CONFIG_L();
	KEEP_CLK();
	spi_write(address);
	spi_write(data);
	KEEP_CLK();
	SPI_CONFIG_H();
	KEEP_CLK();
}

unsigned char receive_byte(void)
{
    unsigned char data = 0;

	KEEP_CLK();
    SPI_CONFIG_H();
    SPI_DATA_L();
    KEEP_CLK();
    spi_read(&data);
    KEEP_CLK();
    SPI_DATA_H();
    KEEP_CLK();
    return data;
}



void set_spi_pin_mux(void)
{
	unsigned long data = REG(PHYS_TO_K0(SPI_PIN_MUX_REG));

	data &= ~0x0FFFFF00;
	/*
	//set to spi
	data |= 0x5636600;
	*/
	REG(PHYS_TO_K0(SPI_PIN_MUX_REG)) = data;
}


void spi_default_config(void)
{
	SPI_CLK_OUT();
	SPI_MOSI_OUT();
	SPI_MISO_IN();
	SPI_CONFIG_OUT();
	SPI_DATA_OUT();

	set_gpio_data(SPI_ODEN_REG, MOSI_PIN, 0);
	set_gpio_data(SPI_ODEN_REG, MISO_PIN, 0);
	set_gpio_data(SPI_ODEN_REG, SCK_PIN, 0);
	set_gpio_data(SPI_ODEN_REG, DATA_PIN, 0);
	set_gpio_data(SPI_ODEN_REG, CONFIG_PIN, 0);

	SPI_CONFIG_H();
	SPI_DATA_H();
	SPI_CLK_L();
	SPI_MOSI_L();
}


