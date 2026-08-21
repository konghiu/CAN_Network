/*
 * mcp2515.h
 *
 *  Created on: Jul 9, 2026
 *      Author: THIS PC
 */

#ifndef INC_MCP2515_H_
#define INC_MCP2515_H_

#include "spi.h"

#define BIT_MODIFY    			0b00000101
#define RESET					0b11000000
#define READ					0b00000011
#define READ_STATUS				0b10100000
#define RX_STATUS				0b10110000
#define READ_RX_BUFFER			0b10010000
#define RX_BUFFER_B0			0b000
#define RX_BUFFER_B0_D0			0b010
#define RX_BUFFER_B1			0b100
#define RX_BUFFER_B1_D0			0b110
#define RTS						0b10000000
#define TXB2					0b100
#define TXB1					0b010
#define TXB0					0b001
#define WRITE					0b00000010
#define LOAD_TX_BUFFER			0b01000000
#define LOAD_TXB0SIDH			0b000
#define LOAD_TXB0D0				0b001
#define LOAD_TXB1SIDH			0b010
#define LOAD_TXB1D0				0b011
#define LOAD_TXB2SIDH			0b100
#define LOAD_TXB2D0				0b101

// control register summary
#define BFPCTRL					0x0C
#define TXRTSCTRL				0x0D
#define CANSTAT					0x0E
#define CANCTRL					0x0F
#define TEC						0x1C
#define	REC						0x1D
#define	CNF3					0x28
#define	CNF2					0x29
#define	CNF1					0x2A
#define	CANINTE					0x2B
#define	CANINTF					0x2C
#define EFLG					0x2D
#define	RXB0CTRL				0x60
#define	RXB1CTRL				0x70

#define	TXB0CTRL				0x30
#define TXB0SIDH				0x31
#define TXB0SIDL				0x32
#define TXB0EID8				0x33
#define TXB0EID0				0x34
#define TXB0DLC					0x35
#define TXB0Dm					0x36

// rx
#define RXB0CTRL				0x60
#define RXB1CTRL				0x70
#define BFPCTRL					0x0C

#define RXB0SIDH				0x61
#define RXB0SIDL				0x62
#define RXB0EID8				0x63
#define RXB0EID0				0x64
#define RXB0DLC					0x65
#define RXB0Dm					0x66

// mask
#define RXM0SIDH				0x20
#define RXM0SIDL				0x21
#define RXM0EID8				0x22
#define RXM0EID0				0x23

#define RXM1SIDH				0x24
#define RXM1SIDL				0x25
#define RXM1EID8				0x26
#define RXM1EID0				0x27

// filter
#define RXF0SIDH				0x00
#define RXF0SIDL				0x01
#define RXF0EID8				0x02
#define RXF0EID0				0x03

#define RXF1SIDH				0x04
#define RXF1SIDL				0x05
#define RXF1EID8				0x06
#define RXF1EID0				0x07

#define RXF2SIDH				0x08
#define RXF2SIDL				0x09
#define RXF2EID8				0x0A
#define RXF2EID0				0x0B

#define RXF3SIDH				0x10
#define RXF3SIDL				0x11
#define RXF3EID8				0x12
#define RXF3EID0				0x13

#define RXF4SIDH				0x14
#define RXF4SIDL				0x15
#define RXF4EID8				0x16
#define RXF4EID0				0x17

#define RXF5SIDH				0x18
#define RXF5SIDL				0x19
#define RXF5EID8				0x1A
#define RXF5EID0				0x1B
// interrupt

void mcp_rts(spi_s *spi, uint8_t request);
void mcp_reset(spi_s *spi);
void mcp_bit_modify(spi_s *spi, uint8_t address, uint8_t mask, uint8_t data);
void mcp_write_byte(spi_s *spi, uint8_t address, uint8_t tx_data);
void mcp_read_byte(spi_s *spi, uint8_t address, uint8_t *rx_data);
void mcp_read_buffer(spi_s *spi, uint8_t offset, uint8_t *rx_buffer);
void mcp_load_buffer(spi_s *spi, uint8_t offset, uint8_t *buffer, uint8_t size);
void mcp_activate_rx(spi_s *spi);
void mcp_activate_tx(spi_s *spi);
void mcp_set_mask_standard(spi_s *spi);
void mcp_set_mask_extend(spi_s *spi);
void mcp_set_filter_standard(spi_s *spi, uint8_t SIDH, uint8_t SIDL);
void mcp_set_filter_extend(spi_s *spi, uint8_t SIDH, uint8_t SIDL, uint8_t EID16, uint8_t EID8, uint8_t EID0);
void mcp_set_ID_standard(spi_s *spi, uint8_t SIDH, uint8_t SIDL);
void mcp_set_ID_extend(spi_s *spi, uint8_t SIDH, uint8_t SIDL, uint8_t EID16, uint8_t EID8, uint8_t EID0);
void mcp_send(spi_s *spi, uint8_t *buffer, uint8_t size);
#endif /* INC_MCP2515_H_ */
