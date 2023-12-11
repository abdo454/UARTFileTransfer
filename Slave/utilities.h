/**
 * @file utilities.h
 * @author abdo daood  (abdo.daood94@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-12-11
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef UTILITIES_LAYER_HEADER_H_
#define UTILITIES_LAYER_HEADER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

    typedef struct
    {
        uint32_t crc32;
        uint32_t size;
    } __attribute__((packed)) BINARY_FILE_INFO;

    // Function to check available space by writing a temporary file
    int check_space_by_writing_temp_file(size_t requiredSize);
    void processMasterCommand(uint8_t cmd_type);
#ifdef __cplusplus
}
#endif
#endif // UTILITIES_LAYER_HEADER_H_
