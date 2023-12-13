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
    if (argc < 4)
    {
        printf("Usage: %s <filename> <UART_port> <UART_baudrate>\n", argv[0]);
        return 1;
    }
    // argv[1] is the filename
    const char *binaryfilename = argv[1];

    // argv[2] is the UART port
    const char *uart_port = argv[2];

    // argv[3] is the UART baud rate
    int uart_baudrate = atoi(argv[3]);

    log_set_level(INFO_LOG_LEVEL);
    /* 1. Open Serial Port */
    int ret = (int)openDevice(uart_port, uart_baudrate);
    if (ret <= 0)
    {
        LOG_ERROR("failed to open serial port");
        return EXIT_FAILURE;
    }

    /* 2. Set WatchDog timer */
    /* ... */
    /* 3. Open the binary file , calculate its CRC32 and length*/

    size_t size_ = 0;
    char *file_contents = read_binary_file(binaryfilename, &size_);
    if (!file_contents)
    {
        LOG_ERROR("Error reading binary file");
        return EXIT_FAILURE;
    }
    binaryinfo.size = size_;
    binaryinfo.crc32 = crc_32((uint8_t *)file_contents, size_);
    printf("-----------------------------------\n");
    printf("File : \"%s\"\n", binaryfilename);
    printf("UART port: %s\n", uart_port);
    printf("UART Baudrate: %d bps\n", uart_baudrate);
    printf("Transmiting speed: %d Byte per Chunk\n", decode_chunk_payload_max_size(encode_chunk_payload_max_size(CHUNK_MAX_PLD_LENGTH_XXXX)));
    printf("File parms: crc32:%08X , size : %dB\n", binaryinfo.crc32, binaryinfo.size);
    printf("-----------------------------------\n\n");

    /* 4. Start While loop */
    static uint8_t updateState = 0;
    uint8_t Slave_ID = SLAVE_ID_01;
    UARTFrame *Uart_Buf = (UARTFrame *)uart_buf;
    UARTChunk tempUARTChunk;
    while (!quitApp)
    {
        switch (updateState)
        {
        case 0: // ask slave to enter bootloader App
            Write_Command_to_Slave(Slave_ID, UART_CMD_ENTER_BOOTLOADER);
            if (tryGetResquestFromSlave(Slave_ID) <= 0)
                break;
            if (Uart_Buf->data == UART_RESPOND_ACK)
                updateState = 1;
            LOG_INFO("Slave in Bootloader Mode");
            break;
            uint16_t bytes_sent = 0;
        case 1: // send file info(size and crc32)
            Write_Info_to_Slave(Slave_ID, UART_HEADER_FRAME, (uint8_t *)&binaryinfo, sizeof(binaryinfo));
            if (tryGetResquestFromSlave(Slave_ID) <= 0)
                break;
            if (Uart_Buf->data == UART_RESPOND_ACK)
                updateState = 2;
            sprintf(msg_buf, "Send file info: size %d , crc32 %08X", binaryinfo.size, binaryinfo.crc32);
            LOG_INFO(msg_buf);
            break;
        case 2: // Ask Slave to check space availabilty
            Write_Command_to_Slave(Slave_ID, UART_CMD_CHECK_SPACE);
            if (tryGetResquestFromSlave(Slave_ID) <= 0)
                break;
            tempUARTChunk.ChunkIdx = 0;
            bytes_sent = 0;
            if (Uart_Buf->data == UART_RESPOND_ACK)
                updateState = 3;
            else
                updateState = 10; // Unavailable space enough for the binary file !!!
            break;
        case 3: // send file as chunks
            tempUARTChunk.ChLen = encode_chunk_payload_max_size(CHUNK_MAX_PLD_LENGTH_XXXX);
            uint16_t chunk_length = decode_chunk_payload_max_size(tempUARTChunk.ChLen);
            uint16_t chunk_size = (bytes_sent + chunk_length <= binaryinfo.size) ? chunk_length : (binaryinfo.size - bytes_sent);
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
            sprintf(msg_buf, "Send Chunk[%d]", tempUARTChunk.ChunkIdx);
            tempUARTChunk.ChunkIdx++;
            bytes_sent += chunk_size;
            LOG_INFO(msg_buf);
            if (bytes_sent >= binaryinfo.size)
                updateState = 4;
            break;
        case 4: // ask slave to check CRC32 , File size , File ELF Header
            Write_Command_to_Slave(Slave_ID, UART_CMD_VERIFY_FILE_PARAMS);
            if (tryGetResquestFromSlave(Slave_ID) <= 0)
                break;
            if (Uart_Buf->data == UART_RESPOND_ACK)
                updateState = 5;
            break;
        case 5: // Ask Slave to end & exit from Bootloader App
            Write_Command_to_Slave(Slave_ID, UART_CMD_END_SESSION);
            /*here, There is no point in waiting for a response from the Slave;
             his response may be subject to noise.*/
            updateState = 6;
            break;
        case 6: // ask slave to END the Session
            LOG_INFO("File updated successfully.");
            LOG_INFO("You can safely reboot the slave device.");

            goto end_while_loop;
            break;
        case 10: // Slave device msg: :Unavilable enough space for binary file
            LOG_ERROR("Unavilable enough space for binary file !!!");
            // TODO process this case
            // ...
            goto end_while_loop;
            break;
        default:
            break;
        }
        usleep(50 * 1000); // 100msec sleep for some reason
    }
end_while_loop:
    free(file_contents);
    LOG_INFO("--------------App Finished--------------")
    return EXIT_SUCCESS;
}
