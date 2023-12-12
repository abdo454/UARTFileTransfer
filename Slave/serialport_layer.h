/**
 * @file serialport_layer.h
 * @author abdo daood  (abdo.daood94@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-12-11
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SERIALPORT_LAYER_HEADER_H_
#define SERIALPORT_LAYER_HEADER_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "serialport.h"
#include "stdint.h"
#define UART_TIMEOUT_MILLISECONDS 100
#define MAX_UART_DATA_PAYLOAD_SIZE (1024 + 3)                                        /* max count of data in the frame that master will send ("1024" in case CHUNK_MAX_PLD_LENGTH_1024B , "3" = UARTChunk:[uint8_t ChLen+uint16_t ChunkIdx]; */
#define UART_FRAME_OVERHEAD_BYTES 11                                                 /* including sof_l,sof_h,id,type,length,crc32,eof*/
#define MAX_UART_FRAME_SIZE (MAX_UART_DATA_PAYLOAD_SIZE + UART_FRAME_OVERHEAD_BYTES) /*total maximum size of a UART frame */

    typedef enum
    {
        UART_SOF_L = 0xAA, /* UART_START_OF_FRAME_LOW_BYTE */
        UART_SOF_H = 0x69, /* UART_START_OF_FRAME_HIGH_BYTE */
        UART_EOF_H = 0xBB, /* UART_END_OF_FRAME_BYTE */
    } FRAME_CONSTANT_;

    typedef enum
    {
        UART_CMD_FRAME = 0x00,    // Frame containing a command
        UART_HEADER_FRAME = 0x01, // Frame containing file information (CRC, length, MD5 sum)
        UART_DATA_FRAME = 0x02,   // Frame containing a chunk of file data
    } UARTFrameType;

    typedef enum
    {
        UART_RESPOND_ACK = 0x00,
        UART_RESPOND_NACK = 0x55,

    } UART_RSPONSE;
    /*
     *  _______________________________________________________
     * | SOF_L | SOF_H | ID | TYPE | LENGTH | DATA | CRC | EOF |
     * |   1B  |   1B  | 1B |  1B  |   2B   | N*B  | 4B  | 1B  |
     * ---------------------------------------------------------
     *                 ^____________CRC____________^
     */

    typedef struct
    {
        uint8_t sof_low;  // Start of Frame, Low Byte
        uint8_t sof_high; // Start of Frame, High Byte
        uint8_t id;       // Slave Device ID
        uint8_t type;     // Frame Type
        uint16_t len;     // Payload Length
        uint8_t data;     // Data Payload
        uint32_t crc;     // CRC Checksum
        uint8_t eof;      // End of Frame
    } __attribute__((packed)) UARTFrame;

    inline char openSerialPort(const char *Device, const unsigned int Bauds)
    {
        return openDevice(Device, Bauds);
    }
    int tryGetResquestFromMaster(uint8_t my_ID);
    int tryGetResquestFromSlave(uint8_t Slave_ID);

    int Write_Command_to_Slave(uint8_t Slave_ID, uint8_t cmd);
    int Write_Info_to_Slave(uint8_t Slave_ID, uint8_t InfoType, uint8_t *data, uint16_t length);
    int Write_Info_to_Master(uint8_t Slave_ID, uint8_t data);
#ifdef __cplusplus
}
#endif

#endif // SERIALPORT_LAYER_HEADER_H_