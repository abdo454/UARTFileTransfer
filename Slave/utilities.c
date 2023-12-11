/**
 * @file utilities.c
 * @author abdo daood (abdo.daood94@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-12-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "utilities.h"
#include "log.h"
#include "main.h"
#include "serialport_layer.h"

#define TEMP_FILE_PATH "/tmp/test_space.tmp"

extern BINARY_FILE_INFO binaryinfo;

int check_space_by_writing_temp_file(size_t requiredSize)
{
    FILE *tempFile = fopen(TEMP_FILE_PATH, "w");
    if (tempFile == NULL)
    {
        LOG_ERROR("Unable to create temporary file");
        return -1;
    }

    char buffer[1024] = {0};
    size_t writtenSize = 0;
    int success = 1;

    while (writtenSize < requiredSize)
    {
        size_t toWrite = (requiredSize - writtenSize) < sizeof(buffer) ? (requiredSize - writtenSize) : sizeof(buffer);
        if (fwrite(buffer, 1, toWrite, tempFile) != toWrite)
        {
            success = -2; // Not enough space or an error occurred
            break;
        }
        writtenSize += toWrite;
    }

    fclose(tempFile);
    remove(TEMP_FILE_PATH); // Clean up

    return success;
}

uint8_t encode_bootloader_version(uint8_t major, uint8_t minor)
{
    // Ensure that major and minor versions fit into 4 bits
    major &= 0x0F; // Mask major to fit in 4 bits
    minor &= 0x0F; // Mask minor to fit in 4 bits

    // Combine major and minor into one byte (major in high 4 bits, minor in low 4 bits)
    return (major << 4) | minor;
}
