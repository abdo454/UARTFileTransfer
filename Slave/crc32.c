#include <stdbool.h>
#include <stdlib.h>
#include "checksum.h"

static void init_crc32_tab(void);

static bool crc_tab32_init = false;
static uint32_t crc_tab32[256];

/*
 *
 * The function crc_32() calculates in one pass the common 32 bit CRC value for
 * a byte string that is passed to the function together with a parameter
 * indicating the length.
 */

uint32_t crc_32(const unsigned char *input_str, size_t num_bytes)
{

	uint32_t crc;
	uint32_t tmp;
	uint32_t long_c;
	const unsigned char *ptr;
	size_t a;

	if (!crc_tab32_init)
		init_crc32_tab();

	crc = CRC_START_32;
	ptr = input_str;

	if (ptr != NULL)
		for (a = 0; a < num_bytes; a++)
		{

			long_c = 0x000000FFL & (uint32_t)*ptr;
			tmp = crc ^ long_c;
			crc = (crc >> 8) ^ crc_tab32[tmp & 0xff];

			ptr++;
		}

	crc ^= 0xffffffffL;

	return crc & 0xffffffffL;

} /* crc_32 */

/*
 * uint32_t update_crc_32_char(uint32_t crc, unsigned char c);

 *
 * The function update_crc_32() calculates a new CRC-32 value based on the
 * previous value of the CRC and the next byte of the data to be checked.
 */

uint32_t update_crc_32_char(uint32_t crc, unsigned char c)
{
	uint32_t tmp;
	uint32_t long_c;
	crc ^= 0xffffffffL;
	long_c = 0x000000ffL & (uint32_t)c;

	if (!crc_tab32_init)
		init_crc32_tab();

	tmp = crc ^ long_c;
	crc = (crc >> 8) ^ crc_tab32[tmp & 0xff];
	crc ^= 0xffffffffL;
	return crc & 0xffffffffL;
}

/*
 * update  crc for a byte string that is passed to the function together with a parameter
 * indicating the length.
 * if this is first call,=assign zero to crc_> crc_=0
 */
uint32_t crc32_update(uint32_t crc_, unsigned char *input_str, size_t num_bytes)
{

	uint32_t crc;
	uint32_t tmp;
	uint32_t long_c;
	const unsigned char *ptr;
	size_t a;

	if (!crc_tab32_init)
		init_crc32_tab();
	crc = crc_ ^ 0xffffffffL;

	ptr = input_str;

	if (ptr != NULL)
		for (a = 0; a < num_bytes; a++)
		{

			long_c = 0x000000FFL & (uint32_t)*ptr;
			tmp = crc ^ long_c;
			crc = (crc >> 8) ^ crc_tab32[tmp & 0xff];

			ptr++;
		}

	crc ^= 0xffffffffL;

	return crc & 0xffffffffL;
} /* update_crc_32 */

/*
 * static void init_crc32_tab( void );
 *
 * For optimal speed, the CRC32 calculation uses a table with pre-calculated
 * bit patterns which are used in the XOR operations in the program. This table
 * is generated once, the first time the CRC update routine is called.
 */

static void init_crc32_tab(void)
{

	uint32_t i;
	uint32_t j;
	uint32_t crc;

	for (i = 0; i < 256; i++)
	{

		crc = i;

		for (j = 0; j < 8; j++)
		{

			if (crc & 0x00000001L)
				crc = (crc >> 1) ^ CRC_POLY_32;
			else
				crc = crc >> 1;
		}

		crc_tab32[i] = crc;
	}

	crc_tab32_init = true;

} /* init_crc32_tab */
