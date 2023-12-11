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
        uint8_t major;
        uint8_t minor;
    } Version;
    typedef struct
    {
        uint32_t crc32;
        uint32_t size;
    } __attribute__((packed)) BINARY_FILE_INFO;
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

    // Function to check available space by writing a temporary file
    // return 1 if space is  available otherwise -1
    int check_space_by_writing_temp_file(size_t requiredSize);

    void processMasterCommand(uint8_t cmd_type);
    uint8_t encode_bootloader_version(uint8_t major, uint8_t minor);

#ifdef __cplusplus
}
#endif
#endif // UTILITIES_LAYER_HEADER_H_
