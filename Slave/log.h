/**
 * @file log.h
 * @author Abdo Daood (abdo.daood94@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-12-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef LOG_HEADER_H_
#define LOG_HEADER_H_
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef enum
    {
        ERROR_LOG_LEVEL = 0,
        WARNING_LOG_LEVEL = 1,
        INFO_LOG_LEVEL = 2
    } LOG_LEVEL;

    void log_set_level(LOG_LEVEL _level);
    void log_error(const char *file, int line, const char *function, const char *msg);
    void log_warning(const char *file, int line, const char *function, const char *msg);
    void log_info(const char *msg);
#define LOG_ERROR(msg) log_error(__FILE__, __LINE__, __FUNCTION__, msg)
#define LOG_WARNING(msg) log_warning(__FILE__, __LINE__, __FUNCTION__, msg)
#define LOG_INFO(msg) log_info(msg);

#ifdef __cplusplus
}
#endif
#endif // LOG_HEADER_H_
