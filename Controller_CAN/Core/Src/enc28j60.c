/*
 * enc28j60.c
 *
 *  Created on: Aug 21, 2026
 *      Author: THIS PC
 */


#include "enc28j60.h"
#include "spi.h"

void enc_write_byte(spi_s *enc, uint8_t address, uint8_t data) {
	uint8_t cmd[2] = {WRITE_CONTROL | address, data};
	spi_select(enc);
	spi_transmit(enc, cmd, 2, 100);
	spi_unselect(enc);
}

void enc_read_byte(spi_s *enc, uint8_t address, uint8_t *byte) {
	uint8_t cmd[1] = {READ_CONTROL | address};
	spi_select(enc);
	spi_transmit(enc, cmd, 1, 100);
	spi_receive(enc, byte, 1, 100);
	spi_unselect(enc);
}

void enc_write_buffer(spi_s *enc, uint8_t *buffer, uint8_t size) {
	uint8_t cmd[1] = {WRITE_BUFFER};
	spi_select(enc);
	spi_transmit(enc, cmd, 1, 100);
	spi_transmit(enc, buffer, size, 100);
	spi_unselect(enc);
}

void enc_write_phy(spi_s *enc, uint8_t address, uint16_t value) {
	enc_write_byte(enc, MIREGADR, address);
	enc_write_byte(enc, MIWRL, value&0xFF);
	enc_write_byte(enc, MIWRH, value>>8);
}

void enc_set_bits(spi_s *enc, uint8_t address, uint8_t byte) {
	uint8_t cmd[2] = {BIT_FIELD_SET | address, byte};
	spi_select(enc);
	spi_transmit(enc, cmd, 2, 100);
	spi_unselect(enc);
}

void enc_clear_bits(spi_s *enc, uint8_t address, uint8_t byte) {
	uint8_t cmd[2] = {BIT_FIELD_CLEAR | address, byte};
	spi_select(enc);
	spi_transmit(enc, cmd, 2, 100);
	spi_unselect(enc);
}

void enc_read_buffer(spi_s *enc, uint8_t *buffer, uint16_t size) {
    uint8_t cmd = 0x3A;

    spi_select(enc);
    spi_transmit(enc, &cmd, 1, 10);
    spi_receive(enc, buffer, size, 100);
    spi_unselect(enc);
}

void enc_reset(spi_s *enc) {
	uint8_t cmd[1] = {SOFT_RESET};
	spi_select(enc);
	spi_transmit(enc, cmd, 1, 100);
	spi_unselect(enc);
}

void enc_init(spi_s *enc) {
    uint16_t max_length = 1518;

    enc_reset(enc);
    HAL_Delay(20);

    enc_clear_bits(enc, ECON1, 0x03); // bank 0

    enc_write_byte(enc, ERXSTL, 0x00);
    enc_write_byte(enc, ERXSTH, 0x0C);
    enc_write_byte(enc, ERXNDL, 0xFF);
    enc_write_byte(enc, ERXNDH, 0x1F);


    enc_clear_bits(enc, ECON1, 0x03); // bank 1
	enc_set_bits(enc, ECON1, 0x01);
	enc_write_byte(enc, ERXFCON, 0xA0);

    enc_clear_bits(enc, ECON1, 0x03);
    enc_set_bits(enc, ECON1, 0x02); // bank 2

    enc_write_byte(enc, MACON1, 0x0D);

    enc_write_byte(enc, MACON3, 0x31);
    enc_write_byte(enc, MACON4, 0x40);

    enc_write_byte(enc, MAMXFLL, max_length & 0xFF);
    enc_write_byte(enc, MAMXFLH, max_length >> 8);

    enc_write_byte(enc, MABBIPG, 0x12);
    enc_write_byte(enc, MAIPGL, 0x12);
    enc_write_byte(enc, MAIPGH, 0x0C);

    enc_set_bits(enc, ECON1, 0x03); // bank 3

    enc_write_byte(enc, MAADR1, 0x21);
    enc_write_byte(enc, MAADR2, 0x55);
    enc_write_byte(enc, MAADR3, 0x44);
    enc_write_byte(enc, MAADR4, 0x33);
    enc_write_byte(enc, MAADR5, 0x22);
    enc_write_byte(enc, MAADR6, 0x11);

    enc_clear_bits(enc, ECON1, 0x03);
    enc_set_bits(enc, ECON1, 0x04);

    enc_write_byte(enc, EIE, 0xC1);
}

void enc_transmit(spi_s *enc, uint8_t *data, uint8_t size) {
	uint8_t frame[64] = {
		0x0e,
		0xff,0xff,0xff,0xff,0xff,0xff, // dest
		0x21,0x55,0x44,0x33,0x22,0x11, // source
		0x08,0x06,
	};
	frame[15] = data[0];
	frame[16] = data[1];
	frame[17] = data[2];
	enc_clear_bits(enc, ECON1, 0x03); // bank 0

	enc_write_byte(enc, ETXSTL, 0x00);
	enc_write_byte(enc, ETXSTH, 0x00);

	enc_write_byte(enc, EWRPTL, 0x00);
	enc_write_byte(enc, EWRPTH, 0x00);

	enc_write_buffer(enc, frame, 18);

	enc_write_byte(enc, ETXNDL, 17);
	enc_write_byte(enc, ETXNDH, 0);
	enc_set_bits(enc, ECON1, 0x08);

}
