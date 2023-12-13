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

    typedef struct
    {
        uint8_t ChLen;              //  CHUNK_MAX_PLD_LENGTH_XXXX indicator
        uint16_t ChunkIdx;          // Chunk index
        uint8_t ChunkPayload[1024]; // Chunk data payload
    } __attribute__((packed)) UARTChunk;
    typedef enum
    {
        UART_CMD_GET_BL_VERSION,     // Get bootloader version
        UART_CMD_GET_APP_VERSION,    // Get application version
        UART_CMD_ENTER_BOOTLOADER,   // Enter bootloader mode
        UART_CMD_CHECK_SPACE,        // Check available space
        UART_CMD_VERIFY_FILE_PARAMS, // Verify file parameters (signature, CRC, size, MD5, type)
        UART_CMD_END_SESSION,        // Close the session
    } COMMAND_CASES;

    // Function to check available space by writing a temporary file
    // return 1 if space is  available otherwise -1
    int check_space_by_writing_temp_file(size_t requiredSize);

    void processMasterCommand(uint8_t cmd_type);
    
    uint8_t encode_bootloader_version(uint8_t major, uint8_t minor);
    // Funcation takes a pointer to start of chunk in frame and chunk length
    int StoreDataIntoFile(uint8_t *ChunkStartPtr, uint16_t ChunkLength);

    uint32_t decode_chunk_payload_max_size(uint8_t ChLen);
    uint8_t encode_chunk_payload_max_size(uint32_t PLD_LENGTH_XXXX);
    // Function to open a file, calculate its CRC32 and length
    int calculate_file_crc_and_length(const char *filename, uint32_t *crc, uint32_t *length);

    // function that reads a binary file and stores its contents in a buffer
    char *read_binary_file(const char *filename, size_t *size);

    // Close Binary File
    void close_binary_file();
#ifdef __cplusplus
}
#endif
#endif // UTILITIES_LAYER_HEADER_H_
