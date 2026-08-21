/*
 * spi.c
 *
 *  Created on: Aug 19, 2026
 *      Author: THIS PC
 */

#include "main.h"
#include "spi.h"


uint8_t spi_transmit(spi_s *spi, uint8_t *buffer, uint8_t size, uint16_t max_time) {
	uint8_t i = 0, dummy;
	uint16_t timeout;
	(void)dummy;
	while(i < size) {
		timeout = max_time;
		while(((spi->spi_x)->SR&0b10) == 0) { // kiem tra tx buffer
			if(timeout-- == 0) return 0;
			delay_ms(1);
		}
		(spi->spi_x)->DR = buffer[i]; // ghi 1 byte vao DR
		i++;
	}

	timeout = max_time;
	while(((spi->spi_x)->SR&0b10) == 0) { // kiem tra tx buffer
		if(timeout-- == 0) return 0;
		delay_ms(1);
	}

	timeout = max_time;
	while((spi->spi_x)->SR&(0x01<<7)) { // kiem tra bit busy
		if(timeout-- == 0) return 0;
		delay_ms(1);
	}
	dummy = (uint8_t)(spi->spi_x)->DR;
	dummy = (uint8_t)(spi->spi_x)->SR;
	return 1;
}

uint8_t spi_receive(spi_s *spi, uint8_t *buffer, uint8_t size, uint16_t max_time) {
	uint16_t timeout = max_time;
	while(size) {
		timeout = max_time;
		(spi->spi_x)->DR = 0x00;
		while(((spi->spi_x)->SR&0b01) == 0) { // kiem tra rx buffer
			if(timeout-- == 0) return 0;
			delay_ms(1);
		}
		*buffer++ = (uint8_t)(spi->spi_x)->DR;
		size--;
	}
	timeout = max_time;
	while((spi->spi_x)->SR&(0x01<<7)) { // kiem tra bit busy
		if(timeout-- == 0) return 0;
		delay_ms(1);
	}
	return 1;
}
void spi_select(spi_s *spi) {
	(spi->port_x)->ODR &= ~(spi->pin_x);
}

void spi_unselect(spi_s *spi) {
	(spi->port_x)->ODR |= spi->pin_x;
}
