
#ifndef DEF_LIBCRC_CHECKSUM_H
#define DEF_LIBCRC_CHECKSUM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    /*
     * #define CRC_POLY_xxxx
     *
     * The constants of the form CRC_POLY_xxxx define the polynomials for some well
     * known CRC calculations.
     */

#define CRC_POLY_32 0xEDB88320L

    /*
     * #define CRC_START_xxxx
     *
     * The constants of the form CRC_START_xxxx define the values that are used for
     * initialization of a CRC value for CRC32.
     */

#define CRC_START_32 0xFFFFFFFFL

    /*
     * Prototype list of global functions
     */

    uint32_t crc_32(const unsigned char *input_str, size_t num_bytes);
    uint32_t update_crc_32_char(uint32_t crc, unsigned char c);
    uint32_t crc32_update(uint32_t crc_, unsigned char *input_str, size_t num_bytes);
#ifdef __cplusplus
}
#endif

#endif // DEF_LIBCRC_CHECKSUM_H
