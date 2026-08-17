/*
 * enc28j60.h
 *
 *  Created on: Mar 9, 2026
 *      Author: THIS PC
 */

#ifndef INC_ENC28J60_H_
#define INC_ENC28J60_H_

// instruction
#define READ_CONTROL					0b00000000
#define READ_BUFFER						0b00111010
#define WRITE_CONTROL					0b01000000
#define WRITE_BUFFER					0b01111010
#define BIT_FIELD_SET					0b10000000
#define BIT_FIELD_CLEAR					0b10100000
#define SOFT_RESET						0b11111111

// address
#define EIE								0x1B
#define EIR								0x1C
#define ESTAT							0x1D
#define ECON2							0x1E
#define ECON1							0x1F

// bank 0
#define ERDPTL							0x00
#define ERDPTH							0x01
#define EWRPTL							0x02
#define EWRPTH							0x03
#define ETXSTL							0x04
#define ETXSTH							0x05
#define ETXNDL							0x06
#define ETXNDH							0x07
#define ERXSTL							0x08
#define ERXSTH							0x09
#define ERXNDL							0x0A
#define ERXNDH							0x0B
#define ERXRDPTL						0x0C
#define ERXRDPTH						0x0D
#define ERXWRPTL						0x0E
#define ERXWRPTH						0x0F
#define EDMASTL							0x10
#define EDMASTH							0x11
#define EDMANDL							0x12
#define EDMANDH							0x13
#define EDMADSTL						0x14
#define EDMADSTH						0x15
#define EDMACSL							0x16
#define EDMACSH							0x17

// bank 1
#define EHT0							0x00
#define EHT1							0x01
#define EHT2							0x02
#define EHT3							0x03
#define EHT4							0x04
#define EHT5							0x05
#define EHT6							0x06
#define EHT7							0x07
#define EPMM0							0x08
#define EPMM1							0x09
#define EPMM2							0x0A
#define EPMM3							0x0B
#define EPMM4							0x0C
#define EPMM5							0x0D
#define EPMM6							0x0E
#define EPMM7							0x0F
#define EPMCSL							0x10
#define EPMCSH							0x11
#define EPMOL							0x14
#define EPMOH							0x15
#define ERXFCON							0x18
#define EPKTCNT							0x19

// bank 2
#define MACON1							0x00
#define MACON3							0x02
#define MACON4							0x03
#define MABBIPG							0x04
#define MAIPGL							0x06
#define MAIPGH							0x07
#define MACLCON1						0x08
#define MACLCON2						0x09
#define MAMXFLL							0x0A
#define MAMXFLH							0x0B
#define MICMD							0x12
#define MIREGADR						0x14
#define MIWRL							0x16
#define MIWRH							0x17
#define MIRDL							0x18
#define MIRDH							0x19

// bank 3
#define MAADR5							0x00
#define MAADR6							0x01
#define MAADR3							0x02
#define MAADR4							0x03
#define MAADR1							0x04
#define MAADR2							0x05
#define EBSTSD							0x06
#define EBSTCON							0x07
#define EBSTCSL							0x08
#define EBSTCSH							0x09
#define MISTAT							0x0A
#define EREVID							0x12
#define ECOCON							0x15
#define EFLOCON							0x17
#define EPAUSL							0x18
#define EPAUSH							0x19


// PHY
#define PHCON1							0x00
#define PHSTAT1							0x01
#define PHID1							0x02
#define PHID2							0x03
#define PHCON2							0x10
#define PHSTAT2							0x11
#define PHIE							0x12
#define PHIR							0x13
#define PHLCON							0x14

typedef struct enc {
	SPI_HandleTypeDef *spi;
	GPIO_TypeDef *port;
	uint16_t pin;
} enc_def;

void enc_cs_select(struct enc ethernet) {
	HAL_GPIO_WritePin(ethernet.port, ethernet.pin, 0);
}

void enc_cs_unselect(struct enc ethernet) {
	HAL_GPIO_WritePin(ethernet.port, ethernet.pin, 1);
}


void enc_write_byte(struct enc ethernet, uint8_t address, uint8_t data) {
	uint8_t cmd[2] = {WRITE_CONTROL | address, data};
	enc_cs_select(ethernet);
	HAL_SPI_Transmit(ethernet.spi, cmd, 2, 100);
	enc_cs_unselect(ethernet);
}

void enc_read_byte(struct enc ethernet, uint8_t address, uint8_t *byte) {
	uint8_t cmd[1] = {READ_CONTROL | address};
	enc_cs_select(ethernet);
	HAL_SPI_Transmit(ethernet.spi, cmd, 1, 100);
	HAL_SPI_Receive(ethernet.spi, byte, 1, 100);
	enc_cs_unselect(ethernet);
}

void enc_write_buffer(struct enc ethernet, uint8_t *buffer, uint8_t size) {
	uint8_t cmd[1] = {WRITE_BUFFER};
	enc_cs_select(ethernet);
	HAL_SPI_Transmit(ethernet.spi, cmd, 1, 100);
	HAL_SPI_Transmit(ethernet.spi, buffer, size, 100);
	enc_cs_unselect(ethernet);
}

void enc_write_phy(struct enc ethernet, uint8_t address, uint16_t value) {
	enc_write_byte(ethernet, MIREGADR, address);
	enc_write_byte(ethernet, MIWRL, value&0xFF);
	enc_write_byte(ethernet, MIWRH, value>>8);
}

void enc_set_bits(struct enc ethernet, uint8_t address, uint8_t byte) {
	uint8_t cmd[2] = {BIT_FIELD_SET | address, byte};
	enc_cs_select(ethernet);
	HAL_SPI_Transmit(ethernet.spi, cmd, 2, 100);
	enc_cs_unselect(ethernet);
}

void enc_clear_bits(struct enc ethernet, uint8_t address, uint8_t byte) {
	uint8_t cmd[2] = {BIT_FIELD_CLEAR | address, byte};
	enc_cs_select(ethernet);
	HAL_SPI_Transmit(ethernet.spi, cmd, 2, 100);
	enc_cs_unselect(ethernet);
}

void enc_read_buffer(struct enc ethernet, uint8_t *buffer, uint16_t size) {
    uint8_t cmd = 0x3A;

    enc_cs_select(ethernet);
    HAL_SPI_Transmit(ethernet.spi, &cmd, 1, 10);
    HAL_SPI_Receive(ethernet.spi, buffer, size, 100);
    enc_cs_unselect(ethernet);
}

void enc_reset(struct enc ethernet) {
	uint8_t cmd[1] = {SOFT_RESET};
	enc_cs_select(ethernet);
	HAL_SPI_Transmit(ethernet.spi, cmd, 1, 100);
	enc_cs_unselect(ethernet);
}

void enc_init(struct enc ethernet) {
    uint16_t max_length = 1518;

    enc_reset(ethernet);
    HAL_Delay(20);

    enc_clear_bits(ethernet, ECON1, 0x03); // bank 0

    enc_write_byte(ethernet, ERXSTL, 0x00);
    enc_write_byte(ethernet, ERXSTH, 0x0C);
    enc_write_byte(ethernet, ERXNDL, 0xFF);
    enc_write_byte(ethernet, ERXNDH, 0x1F);


    enc_clear_bits(ethernet, ECON1, 0x03); // bank 1
	enc_set_bits(ethernet, ECON1, 0x01);
	enc_write_byte(ethernet, ERXFCON, 0xA0);

    enc_clear_bits(ethernet, ECON1, 0x03);
    enc_set_bits(ethernet, ECON1, 0x02); // bank 2

    enc_write_byte(ethernet, MACON1, 0x0D);

    enc_write_byte(ethernet, MACON3, 0x31);
    enc_write_byte(ethernet, MACON4, 0x40);

    enc_write_byte(ethernet, MAMXFLL, max_length & 0xFF);
    enc_write_byte(ethernet, MAMXFLH, max_length >> 8);

    enc_write_byte(ethernet, MABBIPG, 0x12);
    enc_write_byte(ethernet, MAIPGL, 0x12);
    enc_write_byte(ethernet, MAIPGH, 0x0C);

    enc_set_bits(ethernet, ECON1, 0x03); // bank 3

    enc_write_byte(ethernet, MAADR1, 0x21);
    enc_write_byte(ethernet, MAADR2, 0x55);
    enc_write_byte(ethernet, MAADR3, 0x44);
    enc_write_byte(ethernet, MAADR4, 0x33);
    enc_write_byte(ethernet, MAADR5, 0x22);
    enc_write_byte(ethernet, MAADR6, 0x11);

    enc_clear_bits(ethernet, ECON1, 0x03);
    enc_set_bits(ethernet, ECON1, 0x04);

    enc_write_byte(ethernet, EIE, 0xC1);
}

void enc_transmit(struct enc ethernet, uint8_t *data, uint8_t size) {
	uint8_t frame[64] = {
		0x0e,
		0xff,0xff,0xff,0xff,0xff,0xff, // dest
		0x21,0x55,0x44,0x33,0x22,0x11, // source
		0x08,0x06,
	};
	frame[15] = data[0];
	frame[16] = data[1];
	frame[17] = data[2];
	enc_clear_bits(ethernet, ECON1, 0x03); // bank 0

	enc_write_byte(ethernet, ETXSTL, 0x00);
	enc_write_byte(ethernet, ETXSTH, 0x00);

	enc_write_byte(ethernet, EWRPTL, 0x00);
	enc_write_byte(ethernet, EWRPTH, 0x00);

	enc_write_buffer(ethernet, frame, 18);

	enc_write_byte(ethernet, ETXNDL, 17);
	enc_write_byte(ethernet, ETXNDH, 0);
	enc_set_bits(ethernet, ECON1, 0x08);

}


#endif /* INC_ENC28J60_H_ */
