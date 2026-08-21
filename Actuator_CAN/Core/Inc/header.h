/*
 * header.h
 *
 *  Created on: Aug 19, 2026
 *      Author: THIS PC
 */

#ifndef INC_HEADER_H_
#define INC_HEADER_H_

#define MAGIC_NUMBER 			0x03012004

#define HEADER_ADDRESS_FIX		0x08004000
#define APP_START_ADDR			0x08004400

typedef struct {
	uint32_t ota_flag;
	uint32_t magic;
	uint32_t Size;
	uint32_t Crc;
	uint32_t version;
} header_s;

#endif /* INC_HEADER_H_ */
