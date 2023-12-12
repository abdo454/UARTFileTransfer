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
#include "checksum.h"

extern BINARY_FILE_INFO binaryinfo;
FILE *BinFile = NULL;

int check_space_by_writing_temp_file(size_t requiredSize)
{
    if (BinFile == NULL)
    {
        BinFile = fopen(BINARY_FILE_PATH, "r+b"); // Open for reading and writing. The file must exist.
        if (BinFile == NULL)
        {
            // If the file doesn't exist, you might want to open it with "w+b" instead
            BinFile = fopen(BINARY_FILE_PATH, "w+b");
            if (BinFile == NULL)
            {
                LOG_ERROR("Error opening file");
                return -1;
            }
        }
    }

    char buffer[1024] = {0};
    size_t writtenSize = 0;

    while (writtenSize < requiredSize)
    {
        // Seek to the desired offset
        if (fseek(BinFile, writtenSize, SEEK_SET) != 0)
        {
            LOG_ERROR("Error seeking in file");
            return -2;
        }
        size_t toWrite = (requiredSize - writtenSize) < sizeof(buffer) ? (requiredSize - writtenSize) : sizeof(buffer);
        if (fwrite(buffer, 1, toWrite, BinFile) != toWrite)
        {
            LOG_ERROR("Error writing to file");
            return -3; // Not enough space or an error occurred
        }
        writtenSize += toWrite;
    }

    // fclose(BinFile);
    // remove(TEMP_FILE_PATH); // Clean up

    // success
    return 1;
}

// Function takes a File , a buffer, the size of the buffer, and the desired offset
static int write_file_with_offset(FILE *file, const uint8_t *buf, size_t bufSize, long offset)
{
    if (file == NULL)
    {
        LOG_ERROR("Error opening file");
        return -1;
    }
    // Seek to the desired offset
    if (fseek(file, offset, SEEK_SET) != 0)
    {
        LOG_ERROR("Error seeking in file");
        return -2;
    }

    // Write the buffer to the file
    if (fwrite(buf, 1, bufSize, file) != bufSize)
    {
        LOG_ERROR("Error writing to file");
        return -3;
    }
    // success
    return 1;
}
/** The Structure of UART Frame When TYPE=UART_DATA_FRAME
 *  ________________________________________________________________________________________
 * | SOF_L | SOF_H | ID | TYPE | ChunkLength | ChLen | ChunkIdx | ChunkPayload  | CRC | EOF |
 * |   1B  |   1B  | 1B |  1B  |   2B        |   1B  |     2B   |     N*B       | 4B  | 1B  |
 * ------------------------------------------------------------------------------------------
 *                                           ^___CHUNK Header___^_CHUNK Payload_^
 *                                           ^________________CHUNK____________^
 *                 ^____________________________CRC____________________________^
 */

/* ChLen : Indicates to the Maximum Length of ChunkData*/
typedef enum
{
    CHUNK_MAX_PLD_LENGTH_128B = 0x00, // MAX CHUNK Payload Length 128 bytes
    CHUNK_MAX_PLD_LENGTH_256B,        // MAX CHUNK Payload Length 256 bytes
    CHUNK_MAX_PLD_LENGTH_512B,        // MAX CHUNK Payload Length 512 bytes
    CHUNK_MAX_PLD_LENGTH_1024B        // MAX CHUNK Payload Length 1024 bytes (1 KB)
} ChunkMaxDataLength;

int StoreDataIntoFile(uint8_t *ChunkStartPtr, uint16_t ChunkLength)
{

    UARTChunk *UARTChunkPtr = (UARTChunk *)ChunkStartPtr;
    uint16_t offsetIdx = UARTChunkPtr->ChunkIdx;

    uint16_t ChunkPayloadLength = ChunkLength - sizeof(UARTChunkPtr->ChLen) - sizeof(UARTChunkPtr->ChunkIdx);
    uint32_t ChunkStepConstant = decode_chunk_payload_max_size(UARTChunkPtr->ChLen);

    uint16_t offsetAddress = offsetIdx * ChunkStepConstant;
    return write_file_with_offset(BinFile, UARTChunkPtr->ChunkPayload, ChunkPayloadLength, offsetAddress);
}
uint32_t decode_chunk_payload_max_size(uint8_t ChLen)
{

    uint32_t ChunkStepConstant = 0;
    switch (ChLen)
    {
    case CHUNK_MAX_PLD_LENGTH_128B:
        ChunkStepConstant = 128;
        break;
    case CHUNK_MAX_PLD_LENGTH_256B:
        ChunkStepConstant = 256;
        break;
    case CHUNK_MAX_PLD_LENGTH_512B:
        ChunkStepConstant = 512;
        break;
    case CHUNK_MAX_PLD_LENGTH_1024B:
        ChunkStepConstant = 1024;
        break;
    default:
        ChunkStepConstant = 256;
        break;
    }
    return ChunkStepConstant;
}
uint8_t encode_chunk_payload_max_size(uint32_t PLD_LENGTH_XXXX)
{

    uint8_t ChLen = 0;
    switch (PLD_LENGTH_XXXX)
    {
    case 128:
        ChLen = CHUNK_MAX_PLD_LENGTH_128B;
        break;
    case 256:
        ChLen = CHUNK_MAX_PLD_LENGTH_256B;
        break;
    case 512:
        ChLen = CHUNK_MAX_PLD_LENGTH_512B;
        break;
    case 1024:
        ChLen = CHUNK_MAX_PLD_LENGTH_1024B;
        break;
    default:
        ChLen = CHUNK_MAX_PLD_LENGTH_256B;
        break;
    }
    return ChLen;
}

uint8_t encode_bootloader_version(uint8_t major, uint8_t minor)
{
    // Ensure that major and minor versions fit into 4 bits
    major &= 0x0F; // Mask major to fit in 4 bits
    minor &= 0x0F; // Mask minor to fit in 4 bits

    // Combine major and minor into one byte (major in high 4 bits, minor in low 4 bits)
    return (major << 4) | minor;
}

// Function to open a file, calculate its CRC32 and length
int calculate_file_crc_and_length(const char *filename, uint32_t *crc, uint32_t *length)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        // Error opening file
        return -1;
    }

    // Determine the file length
    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Calculate CRC32
    unsigned char buffer[1024];
    size_t bytesRead;
    *crc = 0;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        *crc = crc32_update(*crc, buffer, bytesRead);
    }

    fclose(file);
    return 0;
}

char *read_binary_file(const char *filename, size_t *size)
{
    FILE *file;
    char *buffer;
    size_t file_size;

    // Open the file in binary read mode
    file = fopen(filename, "rb");
    if (!file)
    {
        LOG_ERROR("Error opening file");
        return NULL;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file); // Go back to the start of the file

    // Allocate memory for the file contents
    buffer = (char *)malloc(file_size);
    if (!buffer)
    {
        fclose(file);
        LOG_ERROR("Memory allocation failed\n");
        return NULL;
    }

    // Read the file into the buffer
    if (fread(buffer, 1, file_size, file) != file_size)
    {
        fclose(file);
        free(buffer);
        LOG_ERROR("Error reading file\n");
        return NULL;
    }

    // Close the file
    fclose(file);

    // Write the size of the file to the output parameter
    if (size)
    {
        *size = file_size;
    }

    return buffer;
}