#ifndef CORE_H
#define CORE_H

#include "../common/common.h"

// SAT求解器核心数据结构定义

// 子句结构
typedef struct {
    int* literals;      // 文字数组
    int length;         // 子句长度
    int capacity;       // 容量
} Clause;

// 公式结构
typedef struct {
    Clause* clauses;    // 子句数组
    int num_clauses;    // 子句数量
    int num_vars;       // 变量数量
    int* var_states;    // 变量状态数组
} Formula;

// 函数声明

// 子句操作
Clause* create_clause(int initial_capacity);
void add_literal(Clause* clause, int literal);
void free_clause(Clause* clause);

// 公式操作
Formula* create_formula(int num_vars, int num_clauses);
void free_formula(Formula* formula);
void print_formula(const Formula* formula);

#endif // CORE_H
