#ifndef PARSER_H
#define PARSER_H

#include "../common/common.h"
#include "../core/core.h"

// 解析CNF文件的主要函数
Formula* parse_cnf_file(const char* filename);

// 解析单行子句的辅助函数
Clause* parse_clause_line(const char* line);

// 错误处理函数
void parser_error(const char* message);

#endif
