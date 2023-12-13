/**
 * @file main.c
 * @author Abdo Daood (abdo.daood94@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-12-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "serialport_layer.h"
#include "utilities.h"
#include "checksum.h"
#include "log.h"
#include "main.h"
#define TAG "main"

volatile bool quitApp = false;

extern uint8_t uart_buf[MAX_UART_FRAME_SIZE];
BINARY_FILE_INFO binaryinfo;

Version BL_Version = {
    .major = BL_MAJOR_VERSION,
    .minor = BL_MINOR_VERSION};

char msg_buf[1024];
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <UART_port> <UART_baudrate>\n", argv[0]);
        return 1;
    }

    // argv[1] is the UART port
    const char *uart_port = argv[1];

    // argv[2] is the UART baud rate
    int uart_baudrate = atoi(argv[2]);
    /* 1. Open Serial Port */
    int ret = (int)openDevice(uart_port, uart_baudrate);
    if (ret <= 0)
    {
        LOG_ERROR("faild open serial port");
        return EXIT_FAILURE;
    }
    printf("-----------------------------------\n");
    printf("UART port: %s\n", uart_port);
    printf("UART Baudrate: %d bps\n", uart_baudrate);
    printf("-----------------------------------\n\n");

    /* 2. Set WatchDog timer */

    /* 3. Start While loop */
    LOG_INFO("Start Listening to Master Requests");
    while (!quitApp)
    {
        // check watchdog timout

        if ((ret = tryGetResquestFromMaster(MY_ID)) <= 0)
            continue;
        // watchdog reset

        UARTFrame *frame = (UARTFrame *)uart_buf;
        switch (frame->type)
        {
        case UART_CMD_FRAME:
            processMasterCommand(frame->data);
            break;
        case UART_HEADER_FRAME:
            BINARY_FILE_INFO *binaryinfo_ptr = (BINARY_FILE_INFO *)&frame->data;
            binaryinfo.crc32 = binaryinfo_ptr->crc32;
            binaryinfo.size = binaryinfo_ptr->size;
            sprintf(msg_buf, "Firmware info: size %d , crc32 %08X", binaryinfo.size, binaryinfo.crc32);
            LOG_INFO(msg_buf);
            Write_Info_to_Master(MY_ID, UART_RESPOND_ACK);
            break;
        case UART_DATA_FRAME:
            UART_RSPONSE resp = UART_RESPOND_ACK;
            if (StoreDataIntoFile(&frame->data, frame->len) <= 0)
                resp = UART_RESPOND_NACK;
            Write_Info_to_Master(MY_ID, resp);
            sprintf(msg_buf, "Recivied Chunk[%d]", *(uint16_t *)((uint8_t *)(&frame->data) + 1));
            LOG_INFO(msg_buf);
            break;
        default:
            break;
        }
    }
    LOG_INFO("--------------App Finished--------------")
    return EXIT_SUCCESS;
}

void processMasterCommand(uint8_t cmd_type)
{

    switch (cmd_type)
    {
    case UART_CMD_GET_BL_VERSION:
        uint8_t BL_version = encode_bootloader_version(BL_MAJOR_VERSION, BL_MINOR_VERSION);
        Write_Info_to_Master(MY_ID, BL_version);
        sprintf(msg_buf, "CMD_GET_BL_VERSION:%02X", BL_version);
        LOG_INFO(msg_buf);
        break;
    case UART_CMD_GET_APP_VERSION:
        BL_Version.major = 4;
        sprintf(msg_buf, "CMD_GET_APP_VERSION:%02X", 0x00); // TOSET
        LOG_INFO(msg_buf);
        //
        break;
    case UART_CMD_ENTER_BOOTLOADER:
        Write_Info_to_Master(MY_ID, UART_RESPOND_ACK); // I'm already in bootloader mode
        sprintf(msg_buf, "CMD_GET_ENTER_BOOTLOADER");
        LOG_INFO(msg_buf);
        break;
    case UART_CMD_CHECK_SPACE:
        UART_RSPONSE resp = UART_RESPOND_ACK;
        if (check_space_by_writing_temp_file((size_t)binaryinfo.size) <= 0)
            resp = UART_RESPOND_NACK;
        Write_Info_to_Master(MY_ID, resp);
        sprintf(msg_buf, "CMD_GET_CHECK_SPACE : %s", (resp == UART_RESPOND_ACK ? "ACK" : "NACK"));
        LOG_INFO(msg_buf);
        break;
    case UART_CMD_VERIFY_FILE_PARAMS:
        close_binary_file();
        size_t temp_file_size = 0;
        char *temp_file_contents = read_binary_file(BINARY_FILE_PATH, &temp_file_size);
        if (!temp_file_contents)
        {
            LOG_ERROR("Error reading binary file");
            Write_Info_to_Master(MY_ID, UART_RESPOND_NACK);
            break;
        }
        uint32_t temp_file_crc32 = crc_32((uint8_t *)temp_file_contents, temp_file_size);
        resp = (temp_file_crc32 == binaryinfo.crc32 && temp_file_size == binaryinfo.size)
                   ? UART_RESPOND_ACK
                   : UART_RESPOND_NACK;
        sprintf(msg_buf, "CMD_VERIFY_FILE_PARAMS : %s", (resp == UART_RESPOND_ACK ? "ACK" : "NACK"));
        LOG_INFO(msg_buf);
        Write_Info_to_Master(MY_ID, resp);
        free(temp_file_contents);
        break;
    case UART_CMD_END_SESSION:
        quitApp = true;
        break;
    default:
        break;
    }
}