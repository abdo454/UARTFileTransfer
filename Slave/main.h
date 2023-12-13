/**
 * @file main.h
 * @author abdo daood (abdo.daood94@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-12-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef MAIN_HEADER_H_
#define MAIN_HEADER_H_
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// #define RS_485_ENABLE

#define MY_ID 0x01         // TOSET
#define BL_MAJOR_VERSION 1 // Bootloader major version
#define BL_MINOR_VERSION 0 // Bootloader minor version

#define BINARY_FILE_PATH "./app_xx.bin"

#ifdef __cplusplus
}
#endif
#endif // MAIN_HEADER_H_
