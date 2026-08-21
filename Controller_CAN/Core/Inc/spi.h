/*
 * spi.h
 *
 *  Created on: Aug 19, 2026
 *      Author: THIS PC
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

#include "main.h"

typedef struct {
	SPI_TypeDef *spi_x;
	GPIO_TypeDef *port_x;
	uint16_t pin_x;
} spi_s;

uint8_t spi_transmit(spi_s *spi, uint8_t *buffer, uint8_t size, uint16_t max_time);
uint8_t spi_receive(spi_s *spi, uint8_t *buffer, uint8_t size, uint16_t max_time);
void spi_select(spi_s *spi);
void spi_unselect(spi_s *spi);


#endif /* INC_SPI_H_ */
