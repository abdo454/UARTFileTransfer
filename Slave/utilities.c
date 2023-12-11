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

BINARY_FILE_INFO binaryinfo;
typedef enum
{
    UART_CMD_GET_BL_VERSION,     // Get bootloader version
    UART_CMD_GET_APP_VERSION,    // Get application version
    UART_CMD_ENTER_BOOTLOADER,   // Enter bootloader mode
    UART_CMD_CHECK_SPACE,        // Check available space
    UART_CMD_VERIFY_FILE_PARAMS, // Verify file parameters (signature, CRC, size, MD5, type)
    UART_CMD_FLASH_APP,          // Write application to flash memory
    UART_CMD_END_SESSION,        // Close the session
} COMMAND_CASES;

void processMasterCommand(uint8_t cmd_type)
{

    switch (cmd_type)
    {
    case UART_CMD_GET_BL_VERSION:
        break;
    case UART_CMD_ENTER_BOOTLOADER:
        write_respond_to_Master(MY_ID, UART_RESPOND_ACK); // I'm already in bootloader mode
        break;
    case UART_CMD_CHECK_SPACE:
        write_respond_to_Master(MY_ID, UART_RESPOND_ACK); // I'm already in bootloader mode
        break;
    default:
        break;
    }
}

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
            success = 0; // Not enough space or an error occurred
            break;
        }
        writtenSize += toWrite;
    }

    fclose(tempFile);
    remove(TEMP_FILE_PATH); // Clean up

    return success;
}
