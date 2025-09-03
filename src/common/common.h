#ifndef COMMON_H
#define COMMON_H

// 通用常量和类型定义
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 通用常量定义
#define MAX_PATH_LENGTH 512
#define MAX_LINE_LENGTH 1024

// 通用错误代码
typedef enum {
    COMMON_SUCCESS = 0,
    COMMON_ERROR_MEMORY_ALLOCATION = -1,
    COMMON_ERROR_FILE_NOT_FOUND = -2,
    COMMON_ERROR_INVALID_PARAMETER = -3,
    COMMON_ERROR_TIMEOUT = -4,
    COMMON_ERROR_UNKNOWN = -99
} ErrorCode;

// 通用工具函数声明
void* safe_malloc(size_t size);
void* safe_realloc(void* ptr, size_t size);
void safe_free(void* ptr);
int is_valid_file_path(const char* path);
double get_current_time_ms(void);
void print_error(const char* function_name, ErrorCode error_code, const char* message);

#endif // COMMON_H
