#ifndef _SPI_GPIO_H_
#define _SPI_GPIO_H_

int set_gpio_data(unsigned long reg, int offset, int data);
int get_gpio_data(unsigned long reg, int offset);

void set_spi_pin_mux(void);

void spi_default_config(void);

int sx1213_en_config(void);
int sx1213_en_data(void);

void spi_write(unsigned char data);
unsigned char spi_read(unsigned char *data);

void write_register(unsigned char address, unsigned char data);
unsigned char read_register(unsigned char address);

unsigned char receive_byte(void);

#endif