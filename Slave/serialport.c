#include "serialport.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

int serial_fd;

//_____________________________________
// ::: Constructors and destructors :::

// Read a string (no timeout)
static int readStringNoTimeOut(char *String, char FinalChar, unsigned int MaxNbBytes);

// Current DTR and RTS state (can't be read on WIndows)
// static bool currentStateRTS;
// static bool currentStateDTR;

// Used to store the previous time (for computing timeout)
static struct timeval previousTime;

void closeDevice();

//_________________________________________
// ::: Configuration and initialization :::

/*!
     \brief Open the serial port
     \param Device : Port name (COM1, COM2, ... for Windows ) or (/dev/ttyS0, /dev/ttyACM0, /dev/ttyUSB0 ... for linux)
     \param Bauds : Baud rate of the serial port.

                \n Supported baud rate for Windows :
                        - 110
                        - 300
                        - 600
                        - 1200
                        - 2400
                        - 4800
                        - 9600
                        - 14400
                        - 19200
                        - 38400
                        - 56000
                        - 57600
                        - 115200
                        - 128000
                        - 256000

               \n Supported baud rate for Linux :\n
                        - 110
                        - 300
                        - 600
                        - 1200
                        - 2400
                        - 4800
                        - 9600
                        - 19200
                        - 38400
                        - 57600
                        - 115200
                        - 230400
                        - 460800
                        - 921600
                        - 1000000
                        - 1152000
                        - 1500000
                        - 2000000
                        - 2500000
                        - 3000000
                        - 3500000
                        - 4000000
     \param Databits : Number of data bits in one UART transmission.

            \n Supported values: \n
                - SERIAL_DATABITS_5 (5)
                - SERIAL_DATABITS_6 (6)
                - SERIAL_DATABITS_7 (7)
                - SERIAL_DATABITS_8 (8)
                - SERIAL_DATABITS_16 (16) (not supported on Unix)

     \param Parity: Parity type

            \n Supported values: \n
                - SERIAL_PARITY_NONE (N)
                - SERIAL_PARITY_EVEN (E)
                - SERIAL_PARITY_ODD (O)
                - SERIAL_PARITY_MARK (MARK) (not supported on Unix)
                - SERIAL_PARITY_SPACE (SPACE) (not supported on Unix)
    \param Stopbit: Number of stop bits

            \n Supported values:
                - SERIAL_STOPBITS_1 (1)
                - SERIAL_STOPBITS_1_5 (1.5) (not supported on Unix)
                - SERIAL_STOPBITS_2 (2)

     \return 1 success
     \return -1 device not found
     \return -2 error while opening the device
     \return -3 error while getting port parameters
     \return -4 Speed (Bauds) not recognized
     \return -5 error while writing port parameters
     \return -6 error while writing timeout parameters
     \return -7 Databits not recognized
     \return -8 Stopbits not recognized
     \return -9 Parity not recognized
  */
char openDevice(const char *Device, const unsigned int Bauds)
{
#if defined(_WIN32) || defined(_WIN64)
    // Open serial port
    hSerial = CreateFileA(Device, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, /*FILE_ATTRIBUTE_NORMAL*/ 0, 0);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
            return -1; // Device not found

        // Error while opening the device
        return -2;
    }

    // Set parameters

    // Structure for the port parameters
    DCB dcbSerialParams;
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    // Get the port parameters
    if (!GetCommState(hSerial, &dcbSerialParams))
        return -3;

    // Set the speed (Bauds)
    switch (Bauds)
    {
    case 110:
        dcbSerialParams.BaudRate = CBR_110;
        break;
    case 300:
        dcbSerialParams.BaudRate = CBR_300;
        break;
    case 600:
        dcbSerialParams.BaudRate = CBR_600;
        break;
    case 1200:
        dcbSerialParams.BaudRate = CBR_1200;
        break;
    case 2400:
        dcbSerialParams.BaudRate = CBR_2400;
        break;
    case 4800:
        dcbSerialParams.BaudRate = CBR_4800;
        break;
    case 9600:
        dcbSerialParams.BaudRate = CBR_9600;
        break;
    case 14400:
        dcbSerialParams.BaudRate = CBR_14400;
        break;
    case 19200:
        dcbSerialParams.BaudRate = CBR_19200;
        break;
    case 38400:
        dcbSerialParams.BaudRate = CBR_38400;
        break;
    case 56000:
        dcbSerialParams.BaudRate = CBR_56000;
        break;
    case 57600:
        dcbSerialParams.BaudRate = CBR_57600;
        break;
    case 115200:
        dcbSerialParams.BaudRate = CBR_115200;
        break;
    case 128000:
        dcbSerialParams.BaudRate = CBR_128000;
        break;
    case 256000:
        dcbSerialParams.BaudRate = CBR_256000;
        break;
    default:
        return -4;
    }
    // select data size
    BYTE bytesize = 0;
    switch (Databits)
    {
    case SERIAL_DATABITS_5:
        bytesize = 5;
        break;
    case SERIAL_DATABITS_6:
        bytesize = 6;
        break;
    case SERIAL_DATABITS_7:
        bytesize = 7;
        break;
    case SERIAL_DATABITS_8:
        bytesize = 8;
        break;
    case SERIAL_DATABITS_16:
        bytesize = 16;
        break;
    default:
        return -7;
    }
    BYTE stopBits = 0;
    switch (Stopbits)
    {
    case SERIAL_STOPBITS_1:
        stopBits = ONESTOPBIT;
        break;
    case SERIAL_STOPBITS_1_5:
        stopBits = ONE5STOPBITS;
        break;
    case SERIAL_STOPBITS_2:
        stopBits = TWOSTOPBITS;
        break;
    default:
        return -8;
    }
    BYTE parity = 0;
    switch (Parity)
    {
    case SERIAL_PARITY_NONE:
        parity = NOPARITY;
        break;
    case SERIAL_PARITY_EVEN:
        parity = EVENPARITY;
        break;
    case SERIAL_PARITY_ODD:
        parity = ODDPARITY;
        break;
    case SERIAL_PARITY_MARK:
        parity = MARKPARITY;
        break;
    case SERIAL_PARITY_SPACE:
        parity = SPACEPARITY;
        break;
    default:
        return -9;
    }
    // configure byte size
    dcbSerialParams.ByteSize = bytesize;
    // configure stop bits
    dcbSerialParams.StopBits = stopBits;
    // configure parity
    dcbSerialParams.Parity = parity;

    // Write the parameters
    if (!SetCommState(hSerial, &dcbSerialParams))
        return -5;

    // Set TimeOut

    // Set the Timeout parameters
    timeouts.ReadIntervalTimeout = 0;
    // No TimeOut
    timeouts.ReadTotalTimeoutConstant = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = MAXDWORD;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    // Write the parameters
    if (!SetCommTimeouts(hSerial, &timeouts))
        return -6;

    // Opening successfull
    return 1;
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Structure with the device's options

    SerialDataBits Databits = SERIAL_DATABITS_8;
    SerialParity Parity = SERIAL_PARITY_NONE;
    SerialStopBits Stopbits = SERIAL_STOPBITS_1;

    struct termios options;

    // Open device
    serial_fd = open(Device, O_RDWR | O_NOCTTY); //| O_NDELAY);
    // If the device is not open, return -1
    if (serial_fd == -1)
        return -2;
    // Open the device in nonblocking mode
    // fcntl(serial_fd, F_SETFL, FNDELAY);

    // Get the current options of the port
    tcgetattr(serial_fd, &options);
    // Clear all the options
    bzero(&options, sizeof(options));

    // Prepare speed (Bauds)
    speed_t Speed;
    switch (Bauds)
    {
    case 110:
        Speed = B110;
        break;
    case 300:
        Speed = B300;
        break;
    case 600:
        Speed = B600;
        break;
    case 1200:
        Speed = B1200;
        break;
    case 2400:
        Speed = B2400;
        break;
    case 4800:
        Speed = B4800;
        break;
    case 9600:
        Speed = B9600;
        break;
    case 19200:
        Speed = B19200;
        break;
    case 38400:
        Speed = B38400;
        break;
    case 57600:
        Speed = B57600;
        break;
    case 115200:
        Speed = B115200;
        break;
    case 230400:
        Speed = B230400;
        break;
    case 460800:
        Speed = B460800;
        break;
    case 921600:
        Speed = B921600;
        break;
    case 1000000:
        Speed = B1000000;
        break;
    case 1152000:
        Speed = B1152000;
        break;
    case 1500000:
        Speed = B1500000;
        break;
    case 2000000:
        Speed = B2000000;
        break;
    case 2500000:
        Speed = B2500000;
        break;
    case 3000000:
        Speed = B3000000;
        break;
    case 3500000:
        Speed = B3500000;
        break;
    case 4000000:
        Speed = B4000000;
        break;
    default:
        return -4;
    }
    int databits_flag = 0;
    switch (Databits)
    {
    case SERIAL_DATABITS_5:
        databits_flag = CS5;
        break;
    case SERIAL_DATABITS_6:
        databits_flag = CS6;
        break;
    case SERIAL_DATABITS_7:
        databits_flag = CS7;
        break;
    case SERIAL_DATABITS_8:
        databits_flag = CS8;
        break;
    // 16 bits and everything else not supported
    default:
        return -7;
    }
    int stopbits_flag = 0;
    switch (Stopbits)
    {
    case SERIAL_STOPBITS_1:
        stopbits_flag = 0;
        break;
    case SERIAL_STOPBITS_2:
        stopbits_flag = CSTOPB;
        break;
    // 1.5 stopbits and everything else not supported
    default:
        return -8;
    }
    int parity_flag = 0;
    switch (Parity)
    {
    case SERIAL_PARITY_NONE:
        parity_flag = 0;
        break;
    case SERIAL_PARITY_EVEN:
        parity_flag = PARENB;
        break;
    case SERIAL_PARITY_ODD:
        parity_flag = (PARENB | PARODD);
        break;
    // mark and space parity not supported
    default:
        return -9;
    }

    // Set the baud rate
    cfsetispeed(&options, Speed);
    cfsetospeed(&options, Speed);
    // Configure the device : data bits, stop bits, parity, no control flow
    // Ignore modem control lines (CLOCAL) and Enable receiver (CREAD)
    options.c_cflag |= (CLOCAL | CREAD | databits_flag | parity_flag | stopbits_flag);
    options.c_iflag |= (IGNPAR | IGNBRK);
    // Timer unused
    options.c_cc[VTIME] = 1;
    // At least on character before satisfy reading
    options.c_cc[VMIN] = 0;
    // Activate the settings
    tcsetattr(serial_fd, TCSANOW, &options);
    // Success
    return (1);
#endif
}

// Creat Serial port Block and Inisialize it's Parameter
int Open_serial_port(const char *s, const unsigned int baudrate)
{
    int serial_port = open(s, O_RDWR | O_NOCTTY); //| O_NDELAY

    struct termios tty;

    // Read in existing settings, and handle any error
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr.\n", errno);
        return -1;
    }

    tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
    tty.c_cflag |= CS8;            // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;                                                        // Disable echo
    tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
    tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
    tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 0; // Wait for up to 0.1s (1 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be CUSTOM or Standard Mbps
    cfsetspeed(&tty, baudrate);
    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr.\n", errno);
        return -1;
    }
    serial_fd = serial_port;
    return serial_port;
}

/*!
     \brief Close the connection with the current device
*/
void closeDevice()
{
#if defined(_WIN32) || defined(_WIN64)
    CloseHandle(hSerial);
#endif
#if defined(__linux__) || defined(__APPLE__)
    close(serial_fd);
#endif
}

//___________________________________________
// ::: Read/Write operation on characters :::

/*!
     \brief Write a char on the current serial port
     \param Byte : char to send on the port (must be terminated by '\0')
     \return 1 success
     \return -1 error while writting data
  */
char writeChar(const char Byte)
{
#if defined(_WIN32) || defined(_WIN64)
    // Number of bytes written
    DWORD dwBytesWritten;
    // Write the char to the serial device
    // Return -1 if an error occured
    if (!WriteFile(hSerial, &Byte, 1, &dwBytesWritten, NULL))
        return -1;
    // Write operation successfull
    return 1;
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Write the char
    if (write(serial_fd, &Byte, 1) != 1)
        return -1;

    // Write operation successfull
    return 1;
#endif
}

//________________________________________
// ::: Read/Write operation on strings :::

/*!
     \brief     Write a string on the current serial port
     \param     receivedString : string to send on the port (must be terminated by '\0')
     \return     1 success
     \return    -1 error while writting data
  */
char writeString(const char *receivedString)
{
#if defined(_WIN32) || defined(_WIN64)
    // Number of bytes written
    DWORD dwBytesWritten;
    // Write the string
    if (!WriteFile(hSerial, receivedString, strlen(receivedString), &dwBytesWritten, NULL))
        // Error while writing, return -1
        return -1;
    // Write operation successfull
    return 1;
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Lenght of the string
    int Lenght = strlen(receivedString);
    // Write the string
    if (write(serial_fd, receivedString, Lenght) != Lenght)
        return -1;
    // Write operation successfull
    return 1;
#endif
}

// _____________________________________
// ::: Read/Write operation on bytes :::

/*!
     \brief Write an array of data on the current serial port
     \param Buffer : array of bytes to send on the port
     \param NbBytes : number of byte to send
     \return 1 success
     \return -1 error while writting data
  */
char writeBytes(const void *Buffer, const unsigned int NbBytes)
{
#if defined(_WIN32) || defined(_WIN64)
    // Number of bytes written
    DWORD dwBytesWritten;
    // Write data
    if (!WriteFile(hSerial, Buffer, NbBytes, &dwBytesWritten, NULL))
        // Error while writing, return -1
        return -1;
    // Write operation successfull
    return 1;
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Write data
    // int used = 0;
    // while (used < pkt_size)
    // {
    // 	i = write(serial_port, (uint8_t *)data, 1);
    // 	if (i > 0)
    // 	{
    // 		data++;
    // 		used++;
    // 		update_crc_ccitt(crc, *data);
    // 	}
    // }
    int ret = write(serial_fd, Buffer, NbBytes);
    if (ret != (ssize_t)NbBytes)
    {
        printf("ret %d, NbBytes %d, The error is : %s", ret, NbBytes, strerror(errno)); // TODO add Tag file name , to cach debug messages
        return -1;
    }
    // tcdrain(serial_fd); /* this is very important instruction to insure that the system finish transmit all the data*/
    //  Write operation successfull
    return 1;
#endif
}

/*!
     \brief Wait for a byte from the serial device and return the data read
     \param pByte : data read on the serial device
     \param timeOut_ms : delay of timeout before giving up the reading
            If set to zero, timeout is disable (Optional)
     \return 1 success
     \return 0 Timeout reached
     \return -1 error while setting the Timeout
     \return -2 error while reading the byte
  */
char readChar(char *pByte, unsigned int timeOut_ms)
{
#if defined(_WIN32) || defined(_WIN64)
    // Number of bytes read
    DWORD dwBytesRead = 0;

    // Set the TimeOut
    timeouts.ReadTotalTimeoutConstant = timeOut_ms;

    // Write the parameters, return -1 if an error occured
    if (!SetCommTimeouts(hSerial, &timeouts))
        return -1;

    // Read the byte, return -2 if an error occured
    if (!ReadFile(hSerial, pByte, 1, &dwBytesRead, NULL))
        return -2;

    // Return 0 if the timeout is reached
    if (dwBytesRead == 0)
        return 0;

    // The byte is read
    return 1;
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Timer used for timeout
    initTimer();
    // While Timeout is not reached
    while (elapsedTime_ms() < timeOut_ms || timeOut_ms == 0)
    {
        // Try to read a byte on the device
        switch (read(serial_fd, pByte, 1))
        {
        case 1:
            return 1; // Read successfull
        case -1:
            return -2; // Error while reading
        }
    }
    return 0;
#endif
}

/*!
     \brief Read a string from the serial device (without TimeOut)
     \param receivedString : string read on the serial device
     \param FinalChar : final char of the string
     \param MaxNbBytes : maximum allowed number of bytes read
     \return >0 success, return the number of bytes read
     \return -1 error while setting the Timeout
     \return -2 error while reading the byte
     \return -3 MaxNbBytes is reached
  */
int readStringNoTimeOut(char *receivedString, char finalChar, unsigned int maxNbBytes)
{
    // Number of characters read
    unsigned int NbBytes = 0;
    // Returned value from Read
    char charRead;

    // While the buffer is not full
    while (NbBytes < maxNbBytes)
    {
        // Read a character with the restant time
        charRead = readChar(&receivedString[NbBytes], 0);

        // Check a character has been read
        if (charRead == 1)
        {
            // Check if this is the final char
            if (receivedString[NbBytes] == finalChar)
            {
                // This is the final char, add zero (end of string)
                receivedString[++NbBytes] = 0;
                // Return the number of bytes read
                return NbBytes;
            }

            // The character is not the final char, increase the number of bytes read
            NbBytes++;
        }

        // An error occured while reading, return the error number
        if (charRead < 0)
            return charRead;
    }
    // Buffer is full : return -3
    return -3;
}

/*!
     \brief Read a string from the serial device (with timeout)
     \param receivedString : string read on the serial device
     \param finalChar : final char of the string
     \param maxNbBytes : maximum allowed number of bytes read
     \param timeOut_ms : delay of timeout before giving up the reading (optional)
     \return  >0 success, return the number of bytes read
     \return  0 timeout is reached
     \return -1 error while setting the Timeout
     \return -2 error while reading the byte
     \return -3 MaxNbBytes is reached
  */
int readString(char *receivedString, char finalChar, unsigned int maxNbBytes, unsigned int timeOut_ms)
{
    // Check if timeout is requested
    if (timeOut_ms == 0)
        return readStringNoTimeOut(receivedString, finalChar, maxNbBytes);

    // Number of bytes read
    unsigned int nbBytes = 0;
    // Character read on serial device
    char charRead;
    // Timer used for timeout

    long int timeOutParam;

    // Initialize the timer (for timeout)
    initTimer();

    // While the buffer is not full
    while (nbBytes < maxNbBytes)
    {
        // Compute the TimeOut for the next call of ReadChar
        timeOutParam = timeOut_ms - elapsedTime_ms();

        // If there is time remaining
        if (timeOutParam > 0)
        {
            // Wait for a byte on the serial link with the remaining time as timeout
            charRead = readChar(&receivedString[nbBytes], timeOutParam);

            // If a byte has been received
            if (charRead == 1)
            {
                // Check if the character received is the final one
                if (receivedString[nbBytes] == finalChar)
                {
                    // Final character: add the end character 0
                    receivedString[++nbBytes] = 0;
                    // Return the number of bytes read
                    return nbBytes;
                }
                // This is not the final character, just increase the number of bytes read
                nbBytes++;
            }
            // Check if an error occured during reading char
            // If an error occurend, return the error number
            if (charRead < 0)
                return charRead;
        }
        // Check if timeout is reached
        if (elapsedTime_ms() > timeOut_ms)
        {
            // Add the end caracter
            receivedString[nbBytes] = 0;
            // Return 0 (timeout reached)
            return 0;
        }
    }

    // Buffer is full : return -3
    return -3;
}

/*!
     \brief Read an array of bytes from the serial device (with timeout)
     \param buffer : array of bytes read from the serial device
     \param maxNbBytes : maximum allowed number of bytes read
     \param timeOut_ms : delay of timeout before giving up the reading
     \param sleepDuration_us : delay of CPU relaxing in microseconds (Linux only)
            In the reading loop, a sleep can be performed after each reading
            This allows CPU to perform other tasks
     \return >=0 return the number of bytes read before timeout or
                requested data is completed
     \return -1 error while setting the Timeout
     \return -2 error while reading the byte
  */
int readBytes(void *buffer, unsigned int maxNbBytes, unsigned int timeOut_ms, unsigned int sleepDuration_us)
{
#if defined(_WIN32) || defined(_WIN64)
    // Avoid warning while compiling
    UNUSED(sleepDuration_us);

    // Number of bytes read
    DWORD dwBytesRead = 0;

    // Set the TimeOut
    timeouts.ReadTotalTimeoutConstant = (DWORD)timeOut_ms;

    // Write the parameters and return -1 if an error occrured
    if (!SetCommTimeouts(hSerial, &timeouts))
        return -1;

    // Read the bytes from the serial device, return -2 if an error occured
    if (!ReadFile(hSerial, buffer, (DWORD)maxNbBytes, &dwBytesRead, NULL))
        return -2;

    // Return the byte read
    return dwBytesRead;
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Timer used for timeout

    // Initialise the timer
    initTimer();
    unsigned int NbByteRead = 0;
    // While Timeout is not reached
    while (elapsedTime_ms() < timeOut_ms || timeOut_ms == 0)
    {
        // Compute the position of the current byte
        unsigned char *Ptr = (unsigned char *)buffer + NbByteRead;
        // Try to read a byte on the device
        int Ret = read(serial_fd, (void *)Ptr, maxNbBytes - NbByteRead);
        // Error while reading
        if (Ret == -1)
            return -2;

        // One or several byte(s) has been read on the device
        if (Ret > 0)
        {
            // Increase the number of read bytes
            NbByteRead += Ret;
            // Success : bytes has been read
            if (NbByteRead >= maxNbBytes)
                return NbByteRead;
        }
        // Suspend the loop to avoid charging the CPU
        usleep(sleepDuration_us);
    }
    // Timeout reached, return the number of bytes read
    return NbByteRead;
#endif
}

// _________________________
// ::: Special operation :::

/*!
    \brief Empty receiver buffer
    \return If the function succeeds, the return value is nonzero.
            If the function fails, the return value is zero.
*/
char flushReceiver()
{
#if defined(_WIN32) || defined(_WIN64)
    // Purge receiver
    return PurgeComm(hSerial, PURGE_RXCLEAR);
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Purge receiver
    tcflush(serial_fd, TCIFLUSH);
    return true;
#endif
}

/*!
    \brief  Return the number of bytes in the received buffer (UNIX only)
    \return The number of bytes received by the serial provider but not yet read.
*/
int available()
{
#if defined(_WIN32) || defined(_WIN64)
    // Device errors
    DWORD commErrors;
    // Device status
    COMSTAT commStatus;
    // Read status
    ClearCommError(hSerial, &commErrors, &commStatus);
    // Return the number of pending bytes
    return commStatus.cbInQue;
#endif
#if defined(__linux__) || defined(__APPLE__)
    int nBytes = 0;
    // Return number of pending bytes in the receiver
    ioctl(serial_fd, FIONREAD, &nBytes);
    return nBytes;
#endif
}

// __________________
// ::: I/O Access :::

/*!
    \brief      Set or unset the bit DTR (pin 4)
                DTR stands for Data Terminal Ready
                Convenience method :This method calls setDTR and clearDTR
    \param      status = true set DTR
                status = false unset DTR
    \return     If the function fails, the return value is false
                If the function succeeds, the return value is true.
*/
bool DTR(bool status)
{
    if (status)
        // Set DTR
        return setDTR();
    else
        // Unset DTR
        return clearDTR();
}

/*!
    \brief      Set the bit DTR (pin 4)
                DTR stands for Data Terminal Ready
    \return     If the function fails, the return value is false
                If the function succeeds, the return value is true.
*/
bool setDTR()
{
#if defined(_WIN32) || defined(_WIN64)
    // Set DTR
    currentStateDTR = true;
    return EscapeCommFunction(hSerial, SETDTR);
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Set DTR
    int status_DTR = 0;
    ioctl(serial_fd, TIOCMGET, &status_DTR);
    status_DTR |= TIOCM_DTR;
    ioctl(serial_fd, TIOCMSET, &status_DTR);
    return true;
#endif
}

/*!
    \brief      Clear the bit DTR (pin 4)
                DTR stands for Data Terminal Ready
    \return     If the function fails, the return value is false
                If the function succeeds, the return value is true.
*/
bool clearDTR()
{
#if defined(_WIN32) || defined(_WIN64)
    // Clear DTR
    currentStateDTR = true;
    return EscapeCommFunction(hSerial, CLRDTR);
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Clear DTR
    int status_DTR = 0;
    ioctl(serial_fd, TIOCMGET, &status_DTR);
    status_DTR &= ~TIOCM_DTR;
    ioctl(serial_fd, TIOCMSET, &status_DTR);
    return true;
#endif
}

/*!
    \brief      Set or unset the bit RTS (pin 7)
                RTS stands for Data Termina Ready
                Convenience method :This method calls setDTR and clearDTR
    \param      status = true set DTR
                status = false unset DTR
    \return     false if the function fails
    \return     true if the function succeeds
*/
bool RTS(bool status)
{
    if (status)
        // Set RTS
        return setRTS();
    else
        // Unset RTS
        return clearRTS();
}

/*!
    \brief      Set the bit RTS (pin 7)
                RTS stands for Data Terminal Ready
    \return     If the function fails, the return value is false
                If the function succeeds, the return value is true.
*/
bool setRTS()
{
#if defined(_WIN32) || defined(_WIN64)
    // Set RTS
    currentStateRTS = false;
    return EscapeCommFunction(hSerial, SETRTS);
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Set RTS
    int status_RTS = 0;
    ioctl(serial_fd, TIOCMGET, &status_RTS);
    status_RTS |= TIOCM_RTS;
    ioctl(serial_fd, TIOCMSET, &status_RTS);
    return true;
#endif
}

/*!
    \brief      Clear the bit RTS (pin 7)
                RTS stands for Data Terminal Ready
    \return     If the function fails, the return value is false
                If the function succeeds, the return value is true.
*/
bool clearRTS()
{
#if defined(_WIN32) || defined(_WIN64)
    // Clear RTS
    currentStateRTS = false;
    return EscapeCommFunction(hSerial, CLRRTS);
#endif
#if defined(__linux__) || defined(__APPLE__)
    // Clear RTS
    int status_RTS = 0;
    ioctl(serial_fd, TIOCMGET, &status_RTS);
    status_RTS &= ~TIOCM_RTS;
    ioctl(serial_fd, TIOCMSET, &status_RTS);
    return true;
#endif
}

/*!
    \brief      Get the CTS's status (pin 8)
                CTS stands for Clear To Send
    \return     Return true if CTS is set otherwise false
  */
bool isCTS()
{
#if defined(_WIN32) || defined(_WIN64)
    DWORD modemStat;
    GetCommModemStatus(hSerial, &modemStat);
    return modemStat & MS_CTS_ON;
#endif
#if defined(__linux__) || defined(__APPLE__)
    int status = 0;
    // Get the current status of the CTS bit
    ioctl(serial_fd, TIOCMGET, &status);
    return status & TIOCM_CTS;
#endif
}

/*!
    \brief      Get the DSR's status (pin 6)
                DSR stands for Data Set Ready
    \return     Return true if DTR is set otherwise false
  */
bool isDSR()
{
#if defined(_WIN32) || defined(_WIN64)
    DWORD modemStat;
    GetCommModemStatus(hSerial, &modemStat);
    return modemStat & MS_DSR_ON;
#endif
#if defined(__linux__) || defined(__APPLE__)
    int status = 0;
    // Get the current status of the DSR bit
    ioctl(serial_fd, TIOCMGET, &status);
    return status & TIOCM_DSR;
#endif
}

/*!
    \brief      Get the DCD's status (pin 1)
                CDC stands for Data Carrier Detect
    \return     true if DCD is set
    \return     false otherwise
  */
bool isDCD()
{
#if defined(_WIN32) || defined(_WIN64)
    DWORD modemStat;
    GetCommModemStatus(hSerial, &modemStat);
    return modemStat & MS_RLSD_ON;
#endif
#if defined(__linux__) || defined(__APPLE__)
    int status = 0;
    // Get the current status of the DCD bit
    ioctl(serial_fd, TIOCMGET, &status);
    return status & TIOCM_CAR;
#endif
}

/*!
    \brief      Get the RING's status (pin 9)
                Ring Indicator
    \return     Return true if RING is set otherwise false
  */
bool isRI()
{
#if defined(_WIN32) || defined(_WIN64)
    DWORD modemStat;
    GetCommModemStatus(hSerial, &modemStat);
    return modemStat & MS_RING_ON;
#endif
#if defined(__linux__) || defined(__APPLE__)
    int status = 0;
    // Get the current status of the RING bit
    ioctl(serial_fd, TIOCMGET, &status);
    return status & TIOCM_RNG;
#endif
}

/*!
    \brief      Get the DTR's status (pin 4)
                DTR stands for Data Terminal Ready
                May behave abnormally on Windows
    \return     Return true if CTS is set otherwise false
  */
bool isDTR()
{
#if defined(_WIN32) || defined(_WIN64)
    return currentStateDTR;
#endif
#if defined(__linux__) || defined(__APPLE__)
    int status = 0;
    // Get the current status of the DTR bit
    ioctl(serial_fd, TIOCMGET, &status);
    return status & TIOCM_DTR;
#endif
}

/*!
    \brief      Get the RTS's status (pin 7)
                RTS stands for Request To Send
                May behave abnormally on Windows
    \return     Return true if RTS is set otherwise false
  */
bool isRTS()
{
#if defined(_WIN32) || defined(_WIN64)
    return currentStateRTS;
#endif
#if defined(__linux__) || defined(__APPLE__)
    int status = 0;
    // Get the current status of the CTS bit
    ioctl(serial_fd, TIOCMGET, &status);
    return status & TIOCM_RTS;
#endif
}

// ******************************************
//  Class timeOut
// ******************************************

/*!
    \brief      Initialise the timer. It writes the current time of the day in the structure PreviousTime.
*/
// Initialize the timer
void initTimer()
{
    gettimeofday(&previousTime, NULL);
}

/*!
    \brief      Returns the time elapsed since initialization.  It write the current time of the day in the structure CurrentTime.
                Then it returns the difference between CurrentTime and PreviousTime.
    \return     The number of microseconds elapsed since the functions InitTimer was called.
  */
// Return the elapsed time since initialization
unsigned long int elapsedTime_ms()
{
    // Current time
    struct timeval CurrentTime;
    // Number of seconds and microseconds since last call
    int sec, usec;

    // Get current time
    gettimeofday(&CurrentTime, NULL);

    // Compute the number of seconds and microseconds elapsed since last call
    sec = CurrentTime.tv_sec - previousTime.tv_sec;
    usec = CurrentTime.tv_usec - previousTime.tv_usec;

    // If the previous usec is higher than the current one
    if (usec < 0)
    {
        // Recompute the microseonds and substract one second
        usec = 1000000 - previousTime.tv_usec + CurrentTime.tv_usec;
        sec--;
    }

    // Return the elapsed time in milliseconds
    return sec * 1000 + usec / 1000;
}
