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
#include "log.h"
#include "main.h"
#define TAG "main"

volatile bool quitApp = false;

extern uint8_t uart_buf[MAX_UART_FRAME_SIZE];
BINARY_FILE_INFO binaryinfo;

Version BL_Version = {
    .major = BL_MAJOR_VERSION,
    .minor = BL_MINOR_VERSION};

int main(int argc, char *argv[])
{
    /* 1. Open Serial Port */
    int ret = (int)openDevice("/dev/ttyUSB0", 2000000);
    if (ret <= 0)
    {
        LOG_ERROR("faild open serial port");
        return EXIT_FAILURE;
    }

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
        printf("frame type %02x\n", frame->type);
        switch (frame->type)
        {
        case UART_CMD_FRAME:
            processMasterCommand(frame->data);
            break;
        case UART_HEADER_FRAME:
            BINARY_FILE_INFO *binaryinfo_ptr = (BINARY_FILE_INFO *)&frame->data;
            binaryinfo.crc32 = binaryinfo_ptr->crc32;
            binaryinfo.size = binaryinfo_ptr->size;
            Write_Info_to_Master(MY_ID, UART_RESPOND_ACK);
            break;
        case UART_DATA_FRAME:
            UART_RSPONSE resp = UART_RESPOND_ACK;
            if (StoreDataIntoFile(&frame->data, frame->len) <= 0)
                resp = UART_RESPOND_NACK;
            Write_Info_to_Master(MY_ID, resp);
        default:
            break;
        }
    }
    return EXIT_SUCCESS;
}

void processMasterCommand(uint8_t cmd_type)
{
    printf("command type:%02x\n", cmd_type);

    switch (cmd_type)
    {
    case UART_CMD_GET_BL_VERSION:
        uint8_t BL_version = encode_bootloader_version(BL_MAJOR_VERSION, BL_MINOR_VERSION);
        Write_Info_to_Master(MY_ID, BL_version);
        printf("BL_version:%02x\n", BL_version);
        break;
    case UART_CMD_GET_APP_VERSION:
        //
        break;
    case UART_CMD_ENTER_BOOTLOADER:
        Write_Info_to_Master(MY_ID, UART_RESPOND_ACK); // I'm already in bootloader mode
        break;
    case UART_CMD_CHECK_SPACE:
        UART_RSPONSE resp = UART_RESPOND_ACK;
        if (check_space_by_writing_temp_file((size_t)binaryinfo.size) <= 0)
            resp = UART_RESPOND_NACK;
        Write_Info_to_Master(MY_ID, resp);
        break;
    default:
        break;
    }
}