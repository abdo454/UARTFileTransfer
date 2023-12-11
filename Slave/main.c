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
extern BINARY_FILE_INFO binaryinfo;

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
            write_respond_to_Master(MY_ID, UART_RESPOND_ACK);
            break;
        case UART_DATA_FRAME:
        default:
            break;
        }
    }
    return EXIT_SUCCESS;
}