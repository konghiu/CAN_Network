/*
 * mcp2515.c
 *
 *  Created on: Aug 19, 2026
 *      Author: THIS PC
 */


#include "spi.h"
#include "mcp2515.h"

void mcp_rts(spi_s *spi, uint8_t request) {
	uint8_t cmd[1] = {RTS | request};
	spi_select(spi);
	spi_transmit(spi, cmd, 1, 100);
	spi_unselect(spi);
}

void mcp_reset(spi_s *spi) {
	uint8_t cmd[1] = {RESET};
	spi_select(spi);
	spi_transmit(spi, cmd, 1, 100);
	spi_unselect(spi);
}


void mcp_bit_modify(spi_s *spi, uint8_t address, uint8_t mask, uint8_t data){
    uint8_t cmd[4];
    cmd[0] = BIT_MODIFY;
    cmd[1] = address;
    cmd[2] = mask;
    cmd[3] = data;

    spi_select(spi);
    spi_transmit(spi, cmd, 4, 100);
    spi_unselect(spi);
}

void mcp_write_byte(spi_s *spi, uint8_t address, uint8_t tx_data) {
	uint8_t cmd[3] = {WRITE, address, tx_data};
	spi_select(spi);
	spi_transmit(spi, cmd, 3, 100);
	spi_unselect(spi);
}

void mcp_read_byte(spi_s *spi, uint8_t address, uint8_t *rx_data) {
	uint8_t cmd[2] = {READ, address};
	spi_select(spi);
	spi_transmit(spi, cmd, 2, 100);
	spi_receive(spi, rx_data, 1, 100);
	spi_unselect(spi);
}

void mcp_read_buffer(spi_s *spi, uint8_t offset, uint8_t *rx_buffer) {
	uint8_t cmd[1] = {READ_RX_BUFFER | offset};
	spi_select(spi);
	spi_transmit(spi, cmd, 1, 100);
	spi_receive(spi, rx_buffer, 13, 100);
	spi_unselect(spi);

}

void mcp_load_buffer(spi_s *spi, uint8_t offset, uint8_t *buffer, uint8_t size) {
	uint8_t cmd[1] = {LOAD_TX_BUFFER | offset};
	spi_select(spi);
	spi_transmit(spi, cmd, 1, 100);
	spi_transmit(spi, buffer, size, 100);
	spi_unselect(spi);
}



void mcp_activate_rx(spi_s *spi) {
	mcp_reset(spi);
    HAL_Delay(10);
	mcp_write_byte(spi, CANCTRL, 0x80); // configuration mode

	mcp_write_byte(spi, RXB0CTRL, 0x00);
	mcp_write_byte(spi, BFPCTRL, 0b00000101);
	mcp_write_byte(spi, CANINTE, 0x01);

	mcp_write_byte(spi, RXM0SIDH, 0x00);
	mcp_write_byte(spi, RXM0SIDL, 0x00);
	mcp_write_byte(spi, RXM0EID8, 0x00);
	mcp_write_byte(spi, RXM0EID0, 0x00);

	mcp_write_byte(spi, CNF1, 0x03);
	mcp_write_byte(spi, CNF2, 0x83);
	mcp_write_byte(spi, CNF3, 0x03);
	mcp_write_byte(spi, CANCTRL, 0x00);
	HAL_Delay(10);
	mcp_write_byte(spi, EFLG, 0x00);
	mcp_write_byte(spi, CANINTF, 0x00);
}

void mcp_activate_tx(spi_s *spi) {
	mcp_reset(spi);
    HAL_Delay(10);
	mcp_write_byte(spi, CANCTRL, 0x80); // configuration mode
	mcp_write_byte(spi, CNF1, 0x03);
	mcp_write_byte(spi, CNF2, 0x83);
	mcp_write_byte(spi, CNF3, 0x03);
	mcp_write_byte(spi, CANCTRL, 0x00);
	delay_ms(10);
}

void mcp_set_mask_standard(spi_s *spi) {
	mcp_write_byte(spi, CANCTRL, 0x80);
	mcp_write_byte(spi, RXM0SIDH, 0xFF);
	mcp_write_byte(spi, RXM0SIDL, 0xE0);
	mcp_write_byte(spi, CANCTRL, 0x00);
	delay_ms(5);
}

void mcp_set_mask_extend(spi_s *spi) {
	mcp_write_byte(spi, CANCTRL, 0x80);
	mcp_write_byte(spi, RXM0SIDH, 0xFF);
	mcp_write_byte(spi, RXM0SIDL, 0xEB);
	mcp_write_byte(spi, RXM0EID8, 0xFF);
	mcp_write_byte(spi, RXM0EID0, 0xFF);
	mcp_write_byte(spi, CANCTRL, 0x00);
	delay_ms(5);
}

void mcp_set_filter_standard(spi_s *spi, uint8_t SIDH, uint8_t SIDL) {
	static uint8_t TXBnSIDL;
	TXBnSIDL = SIDL<<5;
	mcp_write_byte(spi, CANCTRL, 0x80);
	mcp_write_byte(spi, RXF0SIDH, SIDH);
	mcp_write_byte(spi, RXF0SIDL, TXBnSIDL);
	mcp_write_byte(spi, CANCTRL, 0x00);
	delay_ms(5);
}

void mcp_set_filter_extend(spi_s *spi, uint8_t SIDH, uint8_t SIDL, uint8_t EID16, uint8_t EID8, uint8_t EID0) {
	static uint8_t TXBnSIDL;
	if(SIDL>7 || EID16>2) return;
	mcp_write_byte(spi, CANCTRL, 0x80);
	TXBnSIDL = (SIDL<<5) |  (0x01<<3) | EID16;
	mcp_write_byte(spi, RXF0SIDH, SIDH);
	mcp_write_byte(spi, RXF0SIDL, TXBnSIDL);
	mcp_write_byte(spi, RXF0EID8, EID8);
	mcp_write_byte(spi, RXF0EID0, EID0);
	mcp_write_byte(spi, CANCTRL, 0x00);
	delay_ms(5);
}

void mcp_set_ID_standard(spi_s *spi, uint8_t SIDH, uint8_t SIDL) {
	static uint8_t TXBnSIDL;
	TXBnSIDL = SIDL<<5;
	mcp_write_byte(spi, TXB0SIDH, SIDH);
	mcp_write_byte(spi, TXB0SIDL, TXBnSIDL);
}

void mcp_set_ID_extend(spi_s *spi, uint8_t SIDH, uint8_t SIDL, uint8_t EID16, uint8_t EID8, uint8_t EID0) {
	static uint8_t TXBnSIDL;
	if(SIDL>7 || EID16>2) return;
	TXBnSIDL = (SIDL<<5) |  (0x01<<3) | EID16;
	mcp_write_byte(spi, TXB0SIDH, SIDH);
	mcp_write_byte(spi, TXB0SIDL, TXBnSIDL);
	mcp_write_byte(spi, TXB0EID8, EID8);
	mcp_write_byte(spi, TXB0EID0, EID0);
}

void mcp_send(spi_s *spi, uint8_t *buffer, uint8_t size) {
	if(size>8) return;
	mcp_write_byte(spi, TXB0DLC, size);
	mcp_load_buffer(spi, LOAD_TXB0D0, buffer, size);
	mcp_rts(spi, TXB0);
}
