/*
 * mcp2515.h
 *
 *  Created on: Jul 4, 2026
 *      Author: THIS PC
 */

#ifndef INC_MCP2515_H_
#define INC_MCP2515_H_


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
#define CANINTE					0x2B
#define CANINTF					0x2C

typedef struct mcp_struct {
	uint16_t pin;
	GPIO_TypeDef *port;
	SPI_HandleTypeDef *spi;
} mcp_def;

void mcp_select(struct mcp_struct *can_def) {
	HAL_GPIO_WritePin(can_def->port, can_def->pin, 0);
}

void mcp_unselect(struct mcp_struct *can_def) {
	HAL_GPIO_WritePin(can_def->port, can_def->pin, 1);
}

void mcp_rts(struct mcp_struct *can_def, uint8_t request) {
	uint8_t cmd[1] = {RTS | request};
	mcp_select(can_def);
	HAL_SPI_Transmit(can_def->spi, cmd, 1, 100);
	mcp_unselect(can_def);
}

void mcp_reset(struct mcp_struct *can_def) {
	uint8_t cmd[1] = {RESET};
	mcp_select(can_def);
	HAL_SPI_Transmit(can_def->spi, cmd, 1, 100);
	mcp_unselect(can_def);
}

#define BIT_MODIFY    0x05

void mcp_bit_modify(struct mcp_struct *can_def, uint8_t address, uint8_t mask, uint8_t data){
    uint8_t cmd[4];
    cmd[0] = BIT_MODIFY;
    cmd[1] = address;
    cmd[2] = mask;
    cmd[3] = data;

    mcp_select(can_def);
    HAL_SPI_Transmit(can_def->spi, cmd, 4, 100);
    mcp_unselect(can_def);
}

void mcp_write_byte(struct mcp_struct *can_def, uint8_t address, uint8_t tx_data) {
	uint8_t cmd[3] = {WRITE, address, tx_data};
	mcp_select(can_def);
	HAL_SPI_Transmit(can_def->spi, cmd, 3, 100);
	mcp_unselect(can_def);
}

void mcp_read_byte(struct mcp_struct *can_def, uint8_t address, uint8_t *rx_data) {
	uint8_t cmd[2] = {READ, address};
	mcp_select(can_def);
	HAL_SPI_Transmit(can_def->spi, cmd, 2, 100);
	HAL_SPI_Receive(can_def->spi, rx_data, 1, 100);
	mcp_unselect(can_def);
}

void mcp_read_buffer(struct mcp_struct *can_def, uint8_t offset, uint8_t *rx_buffer) {
	uint8_t cmd[1] = {READ_RX_BUFFER | offset};
	mcp_select(can_def);
	HAL_SPI_Transmit(can_def->spi, cmd, 1, 100);
	HAL_SPI_Receive(can_def->spi, rx_buffer, 13, 100);
	mcp_unselect(can_def);

}

void mcp_load_buffer(struct mcp_struct *can_def, uint8_t offset, uint8_t *buffer, uint8_t size) {
	uint8_t cmd[1] = {LOAD_TX_BUFFER | offset};
	mcp_select(can_def);
	HAL_SPI_Transmit(can_def->spi, cmd, 1, 100);
	HAL_SPI_Transmit(can_def->spi, buffer, size, 100);
	mcp_unselect(can_def);
}



void mcp_activate_rx(struct mcp_struct *can_def) {
	mcp_reset(can_def);
    HAL_Delay(10);
	mcp_write_byte(can_def, CANCTRL, 0x80); // configuration mode

	mcp_write_byte(can_def, RXB0CTRL, 0x00);
	mcp_write_byte(can_def, BFPCTRL, 0b00000101);
	mcp_write_byte(can_def, CANINTE, 0x01);

	mcp_write_byte(can_def, RXM0SIDH, 0x00);
	mcp_write_byte(can_def, RXM0SIDL, 0x00);
	mcp_write_byte(can_def, RXM0EID8, 0x00);
	mcp_write_byte(can_def, RXM0EID0, 0x00);

	mcp_write_byte(can_def, CNF1, 0x03);
	mcp_write_byte(can_def, CNF2, 0x83);
	mcp_write_byte(can_def, CNF3, 0x03);
	mcp_write_byte(can_def, CANCTRL, 0x08);
	HAL_Delay(10);
	mcp_write_byte(can_def, EFLG, 0x00);
	mcp_write_byte(can_def, CANINTF, 0x00);
}

void mcp_activate_tx(struct mcp_struct *can_def) {
	mcp_reset(can_def);
    HAL_Delay(10);
	mcp_write_byte(can_def, CANCTRL, 0x80); // configuration mode
	mcp_write_byte(can_def, CNF1, 0x03);
	mcp_write_byte(can_def, CNF2, 0x83);
	mcp_write_byte(can_def, CNF3, 0x03);
	mcp_write_byte(can_def, CANCTRL, 0x00);
	HAL_Delay(10);
}

void mcp_set_mask_standard(struct mcp_struct *can_def) {
	mcp_write_byte(can_def, CANCTRL, 0x80);
	mcp_write_byte(can_def, RXM0SIDH, 0xFF);
	mcp_write_byte(can_def, RXM0SIDL, 0xE0);
	mcp_write_byte(can_def, CANCTRL, 0x00);
	HAL_Delay(5);
}

void mcp_set_mask_extend(struct mcp_struct *can_def) {
	mcp_write_byte(can_def, CANCTRL, 0x80);
	mcp_write_byte(can_def, RXM0SIDH, 0xFF);
	mcp_write_byte(can_def, RXM0SIDL, 0xEB);
	mcp_write_byte(can_def, RXM0EID8, 0xFF);
	mcp_write_byte(can_def, RXM0EID0, 0xFF);
	mcp_write_byte(can_def, CANCTRL, 0x00);
	HAL_Delay(5);
}

void mcp_set_filter_standard(struct mcp_struct *can_def, uint8_t SIDH, uint8_t SIDL) {
	static uint8_t TXBnSIDL;
	TXBnSIDL = SIDL<<5;
	mcp_write_byte(can_def, CANCTRL, 0x80);
	mcp_write_byte(can_def, RXF0SIDH, SIDH);
	mcp_write_byte(can_def, RXF0SIDL, TXBnSIDL);
	mcp_write_byte(can_def, CANCTRL, 0x00);
	HAL_Delay(5);
}

void mcp_set_filter_extend(struct mcp_struct *can_def, uint8_t SIDH, uint8_t SIDL, uint8_t EID16, uint8_t EID8, uint8_t EID0) {
	static uint8_t TXBnSIDL;
	if(SIDL>7 || EID16>2) return;
	mcp_write_byte(can_def, CANCTRL, 0x80);
	TXBnSIDL = (SIDL<<5) |  (0x01<<3) | EID16;
	mcp_write_byte(can_def, RXF0SIDH, SIDH);
	mcp_write_byte(can_def, RXF0SIDL, TXBnSIDL);
	mcp_write_byte(can_def, RXF0EID8, EID8);
	mcp_write_byte(can_def, RXF0EID0, EID0);
	mcp_write_byte(can_def, CANCTRL, 0x00);
	HAL_Delay(5);
}

void mcp_set_ID_standard(struct mcp_struct *can_def, uint8_t SIDH, uint8_t SIDL) {
	static uint8_t TXBnSIDL;
	TXBnSIDL = SIDL<<5;
	mcp_write_byte(can_def, TXB0SIDH, SIDH);
	mcp_write_byte(can_def, TXB0SIDL, TXBnSIDL);
}

void mcp_set_ID_extend(struct mcp_struct *can_def, uint8_t SIDH, uint8_t SIDL, uint8_t EID16, uint8_t EID8, uint8_t EID0) {
	static uint8_t TXBnSIDL;
	if(SIDL>7 || EID16>2) return;
	TXBnSIDL = (SIDL<<5) |  (0x01<<3) | EID16;
	mcp_write_byte(can_def, TXB0SIDH, SIDH);
	mcp_write_byte(can_def, TXB0SIDL, TXBnSIDL);
	mcp_write_byte(can_def, TXB0EID8, EID8);
	mcp_write_byte(can_def, TXB0EID0, EID0);
}

void mcp_send(struct mcp_struct *can_def, uint8_t *buffer, uint8_t size) {
	if(size>8) return;
	mcp_write_byte(can_def, TXB0DLC, size);
	mcp_load_buffer(can_def, LOAD_TXB0D0, buffer, size);
	mcp_rts(can_def, TXB0);
}


#endif /* INC_MCP2515_H_ */
