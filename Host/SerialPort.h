#ifndef SERIALPORT_H
#define SERIALPORT_H

// Used for TimeOut operations
#include <sys/time.h>

// Include for Linux
#if defined(__linux__) || defined(__APPLE__)
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <termios.h>
#include <string.h>
// File control definitions
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#endif

/*! To avoid unused parameters */
#define UNUSED(x) (void)(x)

/**
 * number of serial data bits
 */
typedef enum
{
    SERIAL_DATABITS_5,  /**< 5 databits */
    SERIAL_DATABITS_6,  /**< 6 databits */
    SERIAL_DATABITS_7,  /**< 7 databits */
    SERIAL_DATABITS_8,  /**< 8 databits */
    SERIAL_DATABITS_16, /**< 16 databits */
} SerialDataBits;

/**
 * number of serial stop bits
 */
typedef enum
{
    SERIAL_STOPBITS_1,   /**< 1 stop bit */
    SERIAL_STOPBITS_1_5, /**< 1.5 stop bits */
    SERIAL_STOPBITS_2,   /**< 2 stop bits */
} SerialStopBits;

/**
 * type of serial parity bits
 */
typedef enum
{
    SERIAL_PARITY_NONE, /**< no parity bit */
    SERIAL_PARITY_EVEN, /**< even parity bit */
    SERIAL_PARITY_ODD,  /**< odd parity bit */
    SERIAL_PARITY_MARK, /**< mark parity */
    SERIAL_PARITY_SPACE /**< space bit */
} SerialParity;

// Open a device
char openDevice(const char *Device, const unsigned int Bauds);
int Open_serial_port(const char *s, const unsigned int baudrate);
// Close the current device
void closeDevice();

//___________________________________________
// ::: Read/Write operation on characters :::

// Write a char
char writeChar(char);

// Read a char (with timeout)
char readChar(char *pByte, const unsigned int timeOut_ms);

//________________________________________
// ::: Read/Write operation on strings :::

// Write a string
char writeString(const char *String);

// Read a string (with timeout)
int readString(char *receivedString,
               char finalChar,
               unsigned int maxNbBytes,
               const unsigned int timeOut_ms);

// _____________________________________
// ::: Read/Write operation on bytes :::

// Write an array of bytes
char writeBytes(const void *Buffer, const unsigned int NbBytes);

// Read an array of byte (with timeout)
int readBytes(void *buffer, unsigned int maxNbBytes, const unsigned int timeOut_ms, unsigned int sleepDuration_us);

// _________________________
// ::: Special operation :::

// Empty the received buffer
char flushReceiver();

// Return the number of bytes in the received buffer
int available();

// _________________________
// ::: Access to IO bits :::

// Set CTR status (Data Terminal Ready, pin 4)
bool DTR(bool status);
bool setDTR();
bool clearDTR();

// Set RTS status (Request To Send, pin 7)
bool RTS(bool status);
bool setRTS();
bool clearRTS();

// Get RI status (Ring Indicator, pin 9)
bool isRI();

// Get DCD status (Data Carrier Detect, pin 1)
bool isDCD();

// Get CTS status (Clear To Send, pin 8)
bool isCTS();

// Get DSR status (Data Set Ready, pin 9)
bool isDSR();

// Get RTS status (Request To Send, pin 7)
bool isRTS();

// Get CTR status (Data Terminal Ready, pin 4)
bool isDTR();

#if defined(_WIN32) || defined(_WIN64)
// Handle on serial device
HANDLE hSerial;
// For setting serial port timeouts
COMMTIMEOUTS timeouts;
#endif

// Constructor
void timeOut();

// Init the timer
void initTimer();

// Return the elapsed time since initialization
unsigned long int elapsedTime_ms();

#endif // SerialPort_H
