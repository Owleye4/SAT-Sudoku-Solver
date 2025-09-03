#ifndef PERCENT_SUDOKU_H
#define PERCENT_SUDOKU_H

// 百分号数独模块统一接口
// 这个文件作为百分号数独模块的统一入口点

// 包含核心模块
#include "percent_sudoku_core.h"

// 包含CNF转换模块
#include "percent_sudoku_cnf.h"

// 包含求解器模块
#include "percent_sudoku_solver.h"

// 这个文件现在只作为统一接口，具体的功能实现分别在各子模块中
// 保持了向后兼容性，同时提供了清晰的模块结构

#endif // PERCENT_SUDOKU_H