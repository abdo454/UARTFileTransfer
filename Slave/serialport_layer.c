#include "serialport_layer.h"
#include <stdint.h>
#include "log.h"
#include "checksum.h"
uint8_t uart_buf[MAX_UART_FRAME_SIZE];
extern int serial_fd;

// Function to handle RS-485 transmission enable
static void rs485_transmission_enable()
{
#ifdef RS_485_ENABLE
    gpio__RS485_set();
#endif
}

// Function to handle RS-485 transmission disable
static void rs485_transmission_disable()
{
#ifdef RS_485_ENABLE
    gpio__RS485_clear();
#endif
}


int write_respond_to_Master(uint8_t my_ID, UART_RSPONSE state)
{
    rs485_transmission_enable();

    UARTFrame frame = {
        .sof_low = UART_SOF_L,
        .sof_high = UART_SOF_H,
        .id = my_ID,
        .type = UART_DATA_FRAME,
        .len = 0x01,
        .data = state,
        .eof = UART_EOF_H};
    size_t len = sizeof(frame.id) + sizeof(frame.type) + sizeof(frame.len) + sizeof(frame.data);
    frame.crc = crc_32(&frame.id, len);
    writeBytes((uint8_t *)&frame, sizeof(frame));
    usleep(750);
    tcdrain(serial_fd);

    rs485_transmission_disable();
    return 0;
}

typedef enum
{
    FRAME_RECEIVE_SOF_LOW_BYTE = 0, // Start of Frame - Low Byte
    FRAME_RECEIVE_SOF_HIGH_BYTE,    // Start of Frame - High Byte
    FRAME_RECEIVE_DEVICE_ID,        // Device ID
    FRAME_RECEIVE_TYPE,             // Frame Type
    FRAME_RECEIVE_LENGTH_LOW_BYTE,  // Length of Data - Low Byte
    FRAME_RECEIVE_LENGTH_HIGH_BYTE, // Length of Data - High Byte
    FRAME_RECEIVE_DATA_CONTENT,     // Data Content
    FRAME_RECEIVE_CRC_BYTE_0,       // CRC Byte 0 (least significant byte)
    FRAME_RECEIVE_CRC_BYTE_1,       // CRC Byte 1
    FRAME_RECEIVE_CRC_BYTE_2,       // CRC Byte 2
    FRAME_RECEIVE_CRC_BYTE_3,       // CRC Byte 3 (most significant byte)
    FRAME_RECEIVE_EOF,              // End of Frame
} FRAME_RECEIVE_STATE;

int tryGetResquestFromMaster(uint8_t my_ID)
{
    uint16_t index = 0, length = 0, len_i = 0;
    uint8_t data = 0x00;
    uint32_t rec_crc32 = 0, calc_crc = 0;
    FRAME_RECEIVE_STATE switch_case = FRAME_RECEIVE_SOF_LOW_BYTE;
    while (1)
    {
        if (readBytes(&data, 1, UART_TIMEOUT_MILLISECONDS, 0) <= 0)
            return 0;

        switch (switch_case)
        {
        case FRAME_RECEIVE_SOF_LOW_BYTE:
            switch_case = (data == UART_SOF_L) ? FRAME_RECEIVE_SOF_HIGH_BYTE : FRAME_RECEIVE_SOF_LOW_BYTE;
            index = 0;
            break;
        case FRAME_RECEIVE_SOF_HIGH_BYTE:
            switch_case = (data == UART_SOF_H) ? FRAME_RECEIVE_DEVICE_ID : FRAME_RECEIVE_SOF_LOW_BYTE;
            break;
        case FRAME_RECEIVE_DEVICE_ID:
            if (data != my_ID)
                return -1;
            switch_case = FRAME_RECEIVE_TYPE;
            break;
        case FRAME_RECEIVE_TYPE:
            switch_case = FRAME_RECEIVE_LENGTH_LOW_BYTE;
            break;
        case FRAME_RECEIVE_LENGTH_LOW_BYTE:
            len_i = data;
            switch_case = FRAME_RECEIVE_LENGTH_HIGH_BYTE;
            break;
        case FRAME_RECEIVE_LENGTH_HIGH_BYTE:
            len_i += data << 8;
            if (len_i > MAX_UART_DATA_PAYLOAD_SIZE)
            {
                LOG_ERROR("Data Payload size is larger than expected");
                return -2;
            }
            length = len_i;
            switch_case = FRAME_RECEIVE_DATA_CONTENT;
            break;
        case FRAME_RECEIVE_DATA_CONTENT:
            if (--len_i <= 0)
                switch_case = FRAME_RECEIVE_CRC_BYTE_0;
            break;
        case FRAME_RECEIVE_CRC_BYTE_0:
            rec_crc32 = data;
            switch_case = FRAME_RECEIVE_CRC_BYTE_1;
            break;
        case FRAME_RECEIVE_CRC_BYTE_1:
            rec_crc32 += data << 8;
            switch_case = FRAME_RECEIVE_CRC_BYTE_2;
            break;
        case FRAME_RECEIVE_CRC_BYTE_2:
            rec_crc32 += data << 16;
            switch_case = FRAME_RECEIVE_CRC_BYTE_3;
            break;
        case FRAME_RECEIVE_CRC_BYTE_3:
            rec_crc32 += data << 24;
            calc_crc = crc_32(&uart_buf[2], length);
            switch_case = FRAME_RECEIVE_EOF;
            break;
        case FRAME_RECEIVE_EOF: // EOF
            if (calc_crc == rec_crc32)
                return 1;
            else
                return -3;
        default:
            break;
        }
        uart_buf[index++] = data;
    }
    return 0;
}
