#include "common.h"

// 安全内存分配函数
void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Error: Memory allocation failed (requested %zu bytes)\n", size);
        exit(COMMON_ERROR_MEMORY_ALLOCATION);
    }
    return ptr;
}

void* safe_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL && size > 0) {
        fprintf(stderr, "Error: Memory reallocation failed (requested %zu bytes)\n", size);
        exit(COMMON_ERROR_MEMORY_ALLOCATION);
    }
    return new_ptr;
}

void safe_free(void* ptr) {
    if (ptr != NULL) {
        free(ptr);
    }
}

// 文件路径验证
int is_valid_file_path(const char* path) {
    if (path == NULL || strlen(path) == 0 || strlen(path) >= MAX_PATH_LENGTH) {
        return 0;
    }
    return 1;
}

// 获取当前时间（毫秒）
double get_current_time_ms(void) {
    return (double)clock() / CLOCKS_PER_SEC * 1000.0;
}

// 错误打印函数
void print_error(const char* function_name, ErrorCode error_code, const char* message) {
    fprintf(stderr, "Error in %s: [%d] %s\n", function_name, error_code, message);
}
