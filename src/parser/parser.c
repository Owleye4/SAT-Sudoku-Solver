#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 错误处理函数
void parser_error(const char* message) {
    print_error("parser", COMMON_ERROR_UNKNOWN, message);
    exit(1);
}

// 解析单行子句
Clause* parse_clause_line(const char* line) {
    Clause* clause = create_clause(10); // 初始容量为10
    
    // 跳过行首空白字符
    while (isspace(*line)) line++;
    
    // 使用strtok分割字符串
    char* line_copy = strdup(line);
    char* token = strtok(line_copy, " \t\n");
    
    while (token != NULL) {
        int literal = atoi(token);
        
        // 如果读到0，表示子句结束
        if (literal == 0) {
            break;
        }
        
        // 添加文字到子句
        add_literal(clause, literal);
        
        token = strtok(NULL, " \t\n");
    }
    
    safe_free(line_copy);
    return clause;
}

// 解析CNF文件的主要函数
Formula* parse_cnf_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        print_error("parse_cnf_file", COMMON_ERROR_FILE_NOT_FOUND, "Cannot open file");
        return NULL;
    }
    
    char line[1024];
    int num_vars = 0;
    int num_clauses = 0;
    Formula* formula = NULL;
    
    // 逐行读取文件
    while (fgets(line, sizeof(line), file) != NULL) {
        // 去除行末换行符
        line[strcspn(line, "\n")] = 0;
        
        // 跳过空行
        if (strlen(line) == 0) {
            continue;
        }
        
        // 如果行以 'c' 开头，忽略（注释行）
        if (line[0] == 'c') {
            continue;
        }
        
        // 如果行以 'p' 开头，解析问题行
        if (line[0] == 'p') {
            char format[10];
            if (sscanf(line, "p %s %d %d", format, &num_vars, &num_clauses) != 3) {
                parser_error("Cannot parse problem line");
            }
            
            // 检查格式是否为cnf
            if (strcmp(format, "cnf") != 0) {
                parser_error("Unsupported format, only CNF format is supported");
            }
            
            // 创建公式结构
            formula = create_formula(num_vars, num_clauses);
            continue;
        }
        
        // 其他行就是子句
        if (formula == NULL) {
            parser_error("Problem line not found before parsing clauses");
        }
        
        // 解析子句
        Clause* clause = parse_clause_line(line);
        
        // 将子句添加到公式中
        if (formula->num_clauses < num_clauses) {
            // 深拷贝子句数据
            Clause* target_clause = &formula->clauses[formula->num_clauses];
            target_clause->length = clause->length;
            target_clause->capacity = clause->length; // 使用实际长度作为容量
            
            // 分配新的内存并复制文字
            target_clause->literals = (int*)safe_malloc(clause->length * sizeof(int));
            
            // 复制文字数据
            for (int j = 0; j < clause->length; j++) {
                target_clause->literals[j] = clause->literals[j];
            }
            
            formula->num_clauses++;
        } else {
            // 如果子句数量超出预期，动态扩容
            int new_num_clauses = formula->num_clauses + 1;
            formula->clauses = (Clause*)safe_realloc(formula->clauses, new_num_clauses * sizeof(Clause));
            
            // 深拷贝子句数据
            Clause* target_clause = &formula->clauses[formula->num_clauses];
            target_clause->length = clause->length;
            target_clause->capacity = clause->length;
            
            // 分配新的内存并复制文字
            target_clause->literals = (int*)safe_malloc(clause->length * sizeof(int));
            
            // 复制文字数据
            for (int j = 0; j < clause->length; j++) {
                target_clause->literals[j] = clause->literals[j];
            }
            
            formula->num_clauses++;
        }
        
        // 释放临时子句
        free_clause(clause);
    }
    
    fclose(file);
    
    if (formula == NULL) {
        parser_error("Problem line not found in file");
    }
    
    return formula;
}
