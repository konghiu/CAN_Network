/*
 * enc28j60.h
 *
 *  Created on: Aug 21, 2026
 *      Author: THIS PC
 */

#ifndef SRC_ENC28J60_H_
#define SRC_ENC28J60_H_

/*
 * enc28j60.h
 *
 *  Created on: Mar 9, 2026
 *      Author: THIS PC
 */

#ifndef INC_ENC28J60_H_
#define INC_ENC28J60_H_

#include "spi.h"

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

void enc_cs_select(spi_s *enc);
void enc_cs_unselect(spi_s *enc);
void enc_write_byte(spi_s *enc, uint8_t address, uint8_t data);
void enc_read_byte(spi_s *enc, uint8_t address, uint8_t *byte);
void enc_write_buffer(spi_s *enc, uint8_t *buffer, uint8_t size);
void enc_write_phy(spi_s *enc, uint8_t address, uint16_t value);
void enc_set_bits(spi_s *enc, uint8_t address, uint8_t byte);
void enc_clear_bits(spi_s *enc, uint8_t address, uint8_t byte);
void enc_read_buffer(spi_s *enc, uint8_t *buffer, uint16_t size);
void enc_reset(spi_s *enc);
void enc_init(spi_s *enc);
void enc_transmit(spi_s *enc, uint8_t *data, uint8_t size);

#endif /* INC_ENC28J60_H_ */

#endif /* SRC_ENC28J60_H_ */
