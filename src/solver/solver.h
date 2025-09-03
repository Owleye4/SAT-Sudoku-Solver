#ifndef SOLVER_H
#define SOLVER_H

#include "../common/common.h"
#include "../core/core.h"

// 返回常量
#define SAT 1
#define UNSAT 0
#define TIMEOUT -1

// 变量赋值状态
#define UNASSIGNED -1
#define FALSE 0
#define TRUE 1

// 主要DPLL函数
int DPLL(Formula* formula, int* assignments);

// 单子句传播函数
int unit_propagation(Formula* formula, int* assignments);

// 检查子句是否满足
int is_clause_satisfied(const Clause* clause, const int* assignments);

// 检查子句是否冲突（所有文字都为假）
int is_clause_conflict(const Clause* clause, const int* assignments);

// 检查子句是否为单子句
int is_unit_clause(const Clause* clause, const int* assignments, int* unit_literal);

// 检查公式是否完全满足
int is_formula_satisfied(const Formula* formula, const int* assignments);

// 选择分支变量
int select_branching_variable(const Formula* formula, const int* assignments);

// 打印当前赋值状态
void print_assignments(const Formula* formula, const int* assignments);

// 创建赋值数组的副本
int* copy_assignments(const Formula* formula, const int* assignments);

#endif
