#ifndef __POD_CRC_H
#define __POD_CRC_H

#if IS_LITTLE_ENDIAN
# define GET_BYTE(uint, shift) (((uint) >> (shift)) & 0xFF)
#else
# define GET_BYTE(uint, shift) (((uint) >> (24 - (shift))) & 0xFF)
#endif

#define CRC32_IEEE_POLY  0xedb88320
#define CRC32_CCITT_POLY 0x04c11db7
#define CRC32_START 0xffffffff
#define CRC32_TABLE 0xff

typedef enum crc_type_e
{
	CRC32_IEEE,
	CRC32_CCITT,
	CRC32_ZIP,
	CRC32A,
	CRC32B,
	CRC32C_CASTAGNOLIA,
	CRC32K_KOOPMAN_1_3_28,
	CRC32K_KOOPMAN_1_1_30,
	CRC32Q,
	CRC32_TYPE_SIZE,
} crc_type_t;

pod_number_t pod_crc(pod_byte_t* data, pod_size_t count);
#endif
