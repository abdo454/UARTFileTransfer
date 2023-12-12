/**
 * @file main.c
 * @author Abdo Daood (abdo.daood94@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-12-12
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "../Slave/serialport_layer.h"
#include "../Slave/utilities.h"
#include "../Slave/log.h"
#include "../Slave/checksum.h"
#include "main.h"
#define TAG "main"

volatile bool quitApp = false;
char msg_buf[1024];
extern uint8_t uart_buf[MAX_UART_FRAME_SIZE];
BINARY_FILE_INFO binaryinfo;

Version BL_Version = {
    .major = 0,
    .minor = 0};
#define SLAVE_ID_01 0x01

int main(int argc, char *argv[])
{
    log_set_level(INFO_LOG_LEVEL);
    /* 1. Open Serial Port */
    int ret = (int)openDevice("/dev/ttyUSB1", 2000000);
    if (ret <= 0)
    {
        LOG_ERROR("failed to open serial port");
        return EXIT_FAILURE;
    }

    /* 2. Set WatchDog timer */
    /* 3. Open the binary file , calculate its CRC32 and length*/

    size_t size_ = 0;
    char *file_contents = read_binary_file("temp.bin", &size_);
    if (!file_contents)
    {
        LOG_ERROR("Error reading binary file");
        return EXIT_FAILURE;
    }
    binaryinfo.size = size_;
    binaryinfo.crc32 = crc_32((uint8_t *)file_contents, size_);
    sprintf(msg_buf, "file parms: crc32:%08X , size : %dB\n", binaryinfo.crc32, binaryinfo.size);
    LOG_INFO(msg_buf);

    /* 4. Start While loop */
    static uint8_t updateState = 0;
    uint8_t Slave_ID = SLAVE_ID_01;
    UARTFrame *Uart_Buf = (UARTFrame *)uart_buf;
    UARTChunk tempUARTChunk;
    while (!quitApp)
    {
        printf("updateState %d\n", updateState);
        switch (updateState)
        {
        case 0: // ask slave to enter bootloader App
            Write_Command_to_Slave(Slave_ID, UART_CMD_ENTER_BOOTLOADER);
            if (tryGetResquestFromSlave(Slave_ID) <= 0)
                break;
            LOG_INFO("get response from Slave\n");
            if (Uart_Buf->data == UART_RESPOND_ACK)
                updateState = 1;
            break;
        case 1: // send file info(size and crc32)
            Write_Info_to_Slave(Slave_ID, UART_HEADER_FRAME, (uint8_t *)&binaryinfo, sizeof(binaryinfo));
            if (tryGetResquestFromSlave(Slave_ID) <= 0)
                break;
            if (Uart_Buf->data == UART_RESPOND_ACK)
                updateState = 2;
            break;
        case 2: // send file as chunks
            tempUARTChunk.ChLen = encode_chunk_payload_max_size(CHUNK_MAX_PLD_LENGTH_XXXX);
            tempUARTChunk.ChunkIdx = 0;
            uint16_t bytes_sent = 0;
            uint16_t chunk_size = (bytes_sent + CHUNK_MAX_PLD_LENGTH_XXXX <= binaryinfo.size) ? CHUNK_MAX_PLD_LENGTH_XXXX : (binaryinfo.size - bytes_sent);
            if (chunk_size > sizeof(tempUARTChunk.ChunkPayload))
                LOG_ERROR("Buffer Overflow : Check your Code !!\n");
            memcpy(tempUARTChunk.ChunkPayload, file_contents + bytes_sent, chunk_size);
            uint16_t dataSize2Send = sizeof(tempUARTChunk.ChLen) + sizeof(tempUARTChunk.ChunkIdx) + chunk_size;
            Write_Info_to_Slave(Slave_ID, UART_DATA_FRAME, (uint8_t *)&tempUARTChunk, dataSize2Send);
            // Wait for Ack from the slave
            if (tryGetResquestFromSlave(Slave_ID) <= 0)
                break;
            if (Uart_Buf->data != UART_RESPOND_ACK)
                break;
            tempUARTChunk.ChunkIdx++;
            bytes_sent += chunk_size;
            if (bytes_sent >= binaryinfo.size)
                updateState = 3;
            break;
        case 3: // ask slave to check CRC32 , File size , File ELF Header
            Write_Command_to_Slave(Slave_ID, UART_CMD_VERIFY_FILE_PARAMS);
            if (tryGetResquestFromSlave(Slave_ID) <= 0)
                break;
            if (Uart_Buf->data == UART_RESPOND_ACK)
                updateState = 4;
            break;
        case 4: // ask slave to Write application to flash memory
            Write_Command_to_Slave(Slave_ID, UART_CMD_FLASH_APP);
            if (tryGetResquestFromSlave(Slave_ID) <= 0)
                break;
            if (Uart_Buf->data == UART_RESPOND_ACK)
                updateState = 5;
            break;
        case 5: // ask slave to END the Session
            Write_Command_to_Slave(Slave_ID, UART_CMD_END_SESSION);
            updateState = 6;
            break;
        case 6: // ask slave to END the Session
            LOG_INFO("File Updated Successfully\n")
            goto end_while_loop;
            break;
        default:
            break;
        }
        usleep(1000 * 1000); // 100msec sleep for some reason
    }
end_while_loop:
    free(file_contents);
    LOG_INFO("--------------App Finished--------------")
    return EXIT_SUCCESS;
}
