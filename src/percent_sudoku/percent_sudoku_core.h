#ifndef PERCENT_SUDOKU_CORE_H
#define PERCENT_SUDOKU_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../common/common.h"

// 百分号数独常量定义
#define PERCENT_SUDOKU_SIZE 9
#define PERCENT_SUDOKU_EMPTY 0
#define PERCENT_SUDOKU_MIN_NUM 1
#define PERCENT_SUDOKU_MAX_NUM 9

// 百分号数独网格结构
typedef struct {
    int grid[PERCENT_SUDOKU_SIZE][PERCENT_SUDOKU_SIZE];  // 9x9网格
    int is_given[PERCENT_SUDOKU_SIZE][PERCENT_SUDOKU_SIZE];  // 标记哪些格子是给定的
} PercentSudokuGrid;

// 百分号数独游戏结构
typedef struct {
    PercentSudokuGrid puzzle;     // 谜题（部分填充）
    PercentSudokuGrid solution;   // 完整解答
    int difficulty;         // 难度等级 (1-5)
    int num_givens;         // 给定数字的数量
} PercentSudokuGame;

// 百分号数独约束类型
typedef enum {
    CONSTRAINT_ROW,         // 行约束
    CONSTRAINT_COL,         // 列约束
    CONSTRAINT_BOX,         // 宫约束
    CONSTRAINT_DIAGONAL,    // 撇对角线约束
    CONSTRAINT_WINDOW       // 窗口约束
} ConstraintType;

// 百分号数独位置编码结构
typedef struct {
    int row;
    int col;
    int encoding;  // 位置编码 (11-99)
} Position;

// 函数声明

// 基本操作
PercentSudokuGrid* create_percent_sudoku_grid(void);
void free_percent_sudoku_grid(PercentSudokuGrid* grid);
PercentSudokuGame* create_percent_sudoku_game(void);
void free_percent_sudoku_game(PercentSudokuGame* game);
void copy_percent_sudoku_grid(const PercentSudokuGrid* src, PercentSudokuGrid* dest);

// 位置编码转换
int position_to_encoding(int row, int col);
void encoding_to_position(int encoding, int* row, int* col);
int is_valid_position(int row, int col);

// 百分号数独约束检查
int is_valid_placement(const PercentSudokuGrid* grid, int row, int col, int num);
int check_row_constraint(const PercentSudokuGrid* grid, int row, int col, int num);
int check_col_constraint(const PercentSudokuGrid* grid, int row, int col, int num);
int check_box_constraint(const PercentSudokuGrid* grid, int row, int col, int num);
int check_diagonal_constraint(const PercentSudokuGrid* grid, int row, int col, int num);
int check_window_constraint(const PercentSudokuGrid* grid, int row, int col, int num);

// 百分号数独求解（基础回溯算法）
int solve_percent_sudoku(PercentSudokuGrid* grid);
int solve_percent_sudoku_recursive(PercentSudokuGrid* grid, int row, int col);

// 百分号数独生成
PercentSudokuGame* generate_percent_sudoku_puzzle(int difficulty);
void generate_complete_percent_sudoku(PercentSudokuGrid* grid);
void remove_numbers_for_puzzle(PercentSudokuGrid* puzzle, PercentSudokuGrid* solution, int difficulty);

// 百分号数独验证
int is_complete_percent_sudoku(const PercentSudokuGrid* grid);
int is_valid_percent_sudoku(const PercentSudokuGrid* grid);
int has_unique_solution(const PercentSudokuGrid* grid);

// 文件I/O
void save_percent_sudoku_to_file(const PercentSudokuGame* game, const char* filename);
PercentSudokuGame* load_percent_sudoku_from_file(const char* filename);

// 控制台显示
void print_percent_sudoku_console(PercentSudokuGrid* grid, int show_constraints);
void print_percent_sudoku_game_console(PercentSudokuGame* game, int show_solution, int show_constraints);

// 工具函数
int get_random_number(int min, int max);
void shuffle_array(int* array, int size);
int count_solutions(const PercentSudokuGrid* grid);
void count_solutions_recursive(PercentSudokuGrid* grid, int row, int col, int* count);

// 百分号数独特殊位置定义
// 撇对角线位置编码: 19, 28, 37, 46, 55, 64, 73, 82, 91
extern const int DIAGONAL_POSITIONS[9];

// 上方窗口位置编码: 22, 23, 24, 32, 33, 34, 42, 43, 44
extern const int UPPER_WINDOW_POSITIONS[9];

// 下方窗口位置编码: 66, 67, 68, 76, 77, 78, 86, 87, 88
extern const int LOWER_WINDOW_POSITIONS[9];

#endif // PERCENT_SUDOKU_CORE_H
