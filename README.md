# UART File Transmission Project

## Overview
This project enables the transfer of files between two devices, referred to as the Master and the Slave, via a UART serial port, specifically tailored for Linux systems.

## How to Use

### Master Program
1. **Compilation**: Run the Makefile located in the Master program's directory to compile the code. The resulting executable will be in the `Master/bin` folder.
2. **Execution**: Start the program using the syntax: `./master <filename> <UART_port> <UART_baudrate>`.
   - **Example**: `./master temp.bin /dev/ttyUSB0 2000000`
     - `temp.bin` is your file for transfer.
     - `/dev/ttyUSB0` specifies the Master's serial port.
     - `2000000` sets the baud rate for the serial port.

### Slave Application
1. **Compilation**: Similar to the Master, compile by executing the Makefile in the Slave's directory. The output will be in the `bin` folder.
2. **Execution**: Run the application with: `$ ./slave <UART_port> <UART_baudrate>`.
   - **Example**: `./slave /dev/ttyUSB1 2000000`
     - `/dev/ttyUSB1` denotes the Slave's serial port.
     - `2000000` is the baud rate for the Slave's serial port.

## Key Points
- **Consistent Baud Rate**: It's crucial to use the same baud rate for both Master and Slave applications.
- **File Verification**: CRC32 is used to ensure the integrity of the file transmission.
- **Chunked File Transfer**: Files are transmitted in chunks, defaulting to 1024 bytes. To change this, edit the `CHUNK_MAX_PLD_LENGTH_XXXX` definition in `Master/main.h`.

This framework promises an efficient and robust method for file transfer between two devices in a Linux environment. 

**Enjoy seamless and reliable UART file transmission with our easy-to-use setup!**