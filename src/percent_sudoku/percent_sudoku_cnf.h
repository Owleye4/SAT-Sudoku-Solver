#ifndef PERCENT_SUDOKU_CNF_H
#define PERCENT_SUDOKU_CNF_H

#include "../common/common.h"
#include "../core/core.h"
#include "percent_sudoku_core.h"

// 百分号数独到CNF转换的变量编码
// 变量编码规则：对于位置(i,j)的数字k，变量编号为 (i*9 + j)*9 + k
// 其中 i,j 从0开始，k从1开始

// 函数声明

// 基本CNF转换
Formula* percent_sudoku_to_cnf(const PercentSudokuGrid* puzzle);
void add_basic_constraints(Formula* formula);
void add_diagonal_constraints(Formula* formula);
void add_window_constraints(Formula* formula);
void add_given_constraints(Formula* formula, const PercentSudokuGrid* puzzle);

// 约束生成函数
void add_cell_constraints(Formula* formula);
void add_row_constraints(Formula* formula);
void add_column_constraints(Formula* formula);
void add_box_constraints(Formula* formula);

// 从CNF解还原百分号数独
PercentSudokuGrid* cnf_to_percent_sudoku(const Formula* formula, int* assignments, const PercentSudokuGrid* puzzle);

// 变量编码辅助函数
int get_variable_index(int row, int col, int num);
void get_position_from_variable(int var_index, int* row, int* col, int* num);

// 约束验证
int verify_percent_sudoku_cnf(const Formula* formula, const PercentSudokuGrid* puzzle);
void print_cnf_statistics(const Formula* formula);

#endif // PERCENT_SUDOKU_CNF_H
