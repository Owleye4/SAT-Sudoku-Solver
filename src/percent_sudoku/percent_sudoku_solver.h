#ifndef PERCENT_SUDOKU_SOLVER_H
#define PERCENT_SUDOKU_SOLVER_H

#include "../common/common.h"
#include "percent_sudoku_core.h"
#include "percent_sudoku_cnf.h"
#include "../core/core.h"
#include "../solver/solver.h"

// 求解结果枚举
typedef enum {
    PERCENT_SUDOKU_SOLVED,        // 求解成功
    PERCENT_SUDOKU_UNSOLVABLE,    // 无解
    PERCENT_SUDOKU_TIMEOUT,       // 超时
    PERCENT_SUDOKU_ERROR          // 错误
} PercentSudokuResult;

// 求解统计信息
typedef struct {
    double generation_time;    // 生成时间（毫秒）
    double cnf_time;          // CNF转换时间（毫秒）
    double solving_time;      // 求解时间（毫秒）
    double total_time;        // 总时间（毫秒）
    int num_variables;        // 变量数量
    int num_clauses;          // 子句数量
    int num_givens;           // 给定数字数量
    PercentSudokuResult result;     // 求解结果
} PercentSudokuStats;

// 函数声明

// 主要求解函数
PercentSudokuResult solve_percent_sudoku_with_sat(PercentSudokuGame* game, double timeout_seconds);
PercentSudokuResult solve_percent_sudoku_puzzle(const PercentSudokuGrid* puzzle, PercentSudokuGrid* solution, double timeout_seconds);

// 求解流程控制
PercentSudokuResult generate_and_solve_percent_sudoku(int difficulty, double timeout_seconds, PercentSudokuStats* stats);
PercentSudokuResult solve_from_file(const char* puzzle_file, double timeout_seconds, PercentSudokuStats* stats);

// 批量求解
int solve_multiple_puzzles(const char* puzzle_dir, double timeout_seconds, const char* result_file);
void benchmark_percent_sudoku_solver(int num_puzzles, int difficulty, double timeout_seconds);

// 统计和报告
void print_solving_stats(const PercentSudokuStats* stats);
void save_solving_report(const PercentSudokuStats* stats, const char* filename);
void compare_solving_methods(const PercentSudokuGrid* puzzle, double timeout_seconds);

// 验证和测试
int verify_solution_correctness(const PercentSudokuGrid* puzzle, const PercentSudokuGrid* solution);
int test_percent_sudoku_solver(void);
void run_percent_sudoku_tests(void);

// 工具函数
void initialize_solving_stats(PercentSudokuStats* stats);
void update_solving_stats(PercentSudokuStats* stats, PercentSudokuResult result);

// 求解器配置
typedef struct {
    int use_backtracking;     // 是否使用回溯算法
    int use_sat_solver;       // 是否使用SAT求解器
    int verbose_output;       // 是否详细输出
    int save_intermediate;    // 是否保存中间结果
    char* output_dir;         // 输出目录
} SolverConfig;

SolverConfig* create_solver_config(void);
void free_solver_config(SolverConfig* config);
void set_solver_options(SolverConfig* config, int backtracking, int sat_solver, int verbose);

#endif // PERCENT_SUDOKU_SOLVER_H
