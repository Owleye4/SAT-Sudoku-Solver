#include "percent_sudoku_core.h"

// 百分号数独特殊位置定义
const int DIAGONAL_POSITIONS[9] = {19, 28, 37, 46, 55, 64, 73, 82, 91};
const int UPPER_WINDOW_POSITIONS[9] = {22, 23, 24, 32, 33, 34, 42, 43, 44};
const int LOWER_WINDOW_POSITIONS[9] = {66, 67, 68, 76, 77, 78, 86, 87, 88};

// 创建百分号数独网格
PercentSudokuGrid* create_percent_sudoku_grid(void) {
    PercentSudokuGrid* grid = (PercentSudokuGrid*)safe_malloc(sizeof(PercentSudokuGrid));
    
    // 初始化网格为空
    for (int i = 0; i < PERCENT_SUDOKU_SIZE; i++) {
        for (int j = 0; j < PERCENT_SUDOKU_SIZE; j++) {
            grid->grid[i][j] = PERCENT_SUDOKU_EMPTY;
            grid->is_given[i][j] = 0;
        }
    }
    
    return grid;
}

// 释放百分号数独网格内存
void free_percent_sudoku_grid(PercentSudokuGrid* grid) {
    if (grid != NULL) {
        safe_free(grid);
    }
}

// 创建百分号数独游戏
PercentSudokuGame* create_percent_sudoku_game(void) {
    PercentSudokuGame* game = (PercentSudokuGame*)safe_malloc(sizeof(PercentSudokuGame));
    
    game->puzzle = *create_percent_sudoku_grid();
    game->solution = *create_percent_sudoku_grid();
    game->difficulty = 1;
    game->num_givens = 0;
    
    return game;
}

// 释放百分号数独游戏内存
void free_percent_sudoku_game(PercentSudokuGame* game) {
    if (game != NULL) {
        safe_free(game);
    }
}

// 复制百分号数独网格
void copy_percent_sudoku_grid(const PercentSudokuGrid* src, PercentSudokuGrid* dest) {
    if (src == NULL || dest == NULL) {
        print_error("copy_percent_sudoku_grid", COMMON_ERROR_INVALID_PARAMETER, "NULL pointer in copy_percent_sudoku_grid");
        return;
    }
    
    for (int i = 0; i < PERCENT_SUDOKU_SIZE; i++) {
        for (int j = 0; j < PERCENT_SUDOKU_SIZE; j++) {
            dest->grid[i][j] = src->grid[i][j];
            dest->is_given[i][j] = src->is_given[i][j];
        }
    }
}

// 位置编码转换：行列坐标转换为编码
int position_to_encoding(int row, int col) {
    if (row < 1 || row > 9 || col < 1 || col > 9) {
        return -1;  // 无效位置
    }
    return row * 10 + col;
}

// 编码转换：编码转换为行列坐标
void encoding_to_position(int encoding, int* row, int* col) {
    if (encoding < 11 || encoding > 99) {
        *row = -1;
        *col = -1;
        return;
    }
    *row = encoding / 10;
    *col = encoding % 10;
}

// 检查位置是否有效
int is_valid_position(int row, int col) {
    return (row >= 0 && row < PERCENT_SUDOKU_SIZE && col >= 0 && col < PERCENT_SUDOKU_SIZE);
}

// 检查行约束
int check_row_constraint(const PercentSudokuGrid* grid, int row, int col, int num) {
    for (int j = 0; j < PERCENT_SUDOKU_SIZE; j++) {
        if (j != col && grid->grid[row][j] == num) {
            return 0;  // 违反行约束
        }
    }
    return 1;  // 满足行约束
}

// 检查列约束
int check_col_constraint(const PercentSudokuGrid* grid, int row, int col, int num) {
    for (int i = 0; i < PERCENT_SUDOKU_SIZE; i++) {
        if (i != row && grid->grid[i][col] == num) {
            return 0;  // 违反列约束
        }
    }
    return 1;  // 满足列约束
}

// 检查宫约束
int check_box_constraint(const PercentSudokuGrid* grid, int row, int col, int num) {
    int box_row = (row / 3) * 3;
    int box_col = (col / 3) * 3;
    
    for (int i = box_row; i < box_row + 3; i++) {
        for (int j = box_col; j < box_col + 3; j++) {
            if ((i != row || j != col) && grid->grid[i][j] == num) {
                return 0;  // 违反宫约束
            }
        }
    }
    return 1;  // 满足宫约束
}

// 检查撇对角线约束
int check_diagonal_constraint(const PercentSudokuGrid* grid, int row, int col, int num) {
    // 检查是否在撇对角线上 (row + col == 10)
    if (row + col != 8) {  // 数组索引从0开始，所以是8而不是10
        return 1;  // 不在对角线上，约束满足
    }
    
    // 在撇对角线上，检查其他位置
    for (int i = 0; i < PERCENT_SUDOKU_SIZE; i++) {
        int j = 8 - i;  // 撇对角线：i + j = 8
        if ((i != row || j != col) && grid->grid[i][j] == num) {
            return 0;  // 违反撇对角线约束
        }
    }
    return 1;  // 满足撇对角线约束
}

// 检查窗口约束
int check_window_constraint(const PercentSudokuGrid* grid, int row, int col, int num) {
    // 检查上方窗口 (2,2)到(4,4)
    if (row >= 1 && row <= 3 && col >= 1 && col <= 3) {
        for (int i = 1; i <= 3; i++) {
            for (int j = 1; j <= 3; j++) {
                if ((i != row || j != col) && grid->grid[i][j] == num) {
                    return 0;  // 违反上方窗口约束
                }
            }
        }
    }
    
    // 检查下方窗口 (6,6)到(8,8)
    if (row >= 5 && row <= 7 && col >= 5 && col <= 7) {
        for (int i = 5; i <= 7; i++) {
            for (int j = 5; j <= 7; j++) {
                if ((i != row || j != col) && grid->grid[i][j] == num) {
                    return 0;  // 违反下方窗口约束
                }
            }
        }
    }
    
    return 1;  // 满足窗口约束
}

// 检查数字放置是否有效（所有约束）
int is_valid_placement(const PercentSudokuGrid* grid, int row, int col, int num) {
    if (!is_valid_position(row, col) || num < PERCENT_SUDOKU_MIN_NUM || num > PERCENT_SUDOKU_MAX_NUM) {
        return 0;
    }
    
    return check_row_constraint(grid, row, col, num) &&
           check_col_constraint(grid, row, col, num) &&
           check_box_constraint(grid, row, col, num) &&
           check_diagonal_constraint(grid, row, col, num) &&
           check_window_constraint(grid, row, col, num);
}

// 递归求解百分号数独
int solve_percent_sudoku_recursive(PercentSudokuGrid* grid, int row, int col) {
    // 如果到达最后一行的下一行，说明求解完成
    if (row == PERCENT_SUDOKU_SIZE) {
        return 1;
    }
    
    // 如果到达当前行的最后一列，移动到下一行
    if (col == PERCENT_SUDOKU_SIZE) {
        return solve_percent_sudoku_recursive(grid, row + 1, 0);
    }
    
    // 如果当前位置已有数字，移动到下一列
    if (grid->grid[row][col] != PERCENT_SUDOKU_EMPTY) {
        return solve_percent_sudoku_recursive(grid, row, col + 1);
    }
    
    // 尝试放置数字1-9
    for (int num = PERCENT_SUDOKU_MIN_NUM; num <= PERCENT_SUDOKU_MAX_NUM; num++) {
        if (is_valid_placement(grid, row, col, num)) {
            grid->grid[row][col] = num;
            
            if (solve_percent_sudoku_recursive(grid, row, col + 1)) {
                return 1;  // 找到解
            }
            
            grid->grid[row][col] = PERCENT_SUDOKU_EMPTY;  // 回溯
        }
    }
    
    return 0;  // 无解
}

// 求解百分号数独
int solve_percent_sudoku(PercentSudokuGrid* grid) {
    if (grid == NULL) {
        return 0;
    }
    return solve_percent_sudoku_recursive(grid, 0, 0);
}

// 生成完整的百分号数独
void generate_complete_percent_sudoku(PercentSudokuGrid* grid) {
    if (grid == NULL) {
        return;
    }
    
    // 清空网格
    for (int i = 0; i < PERCENT_SUDOKU_SIZE; i++) {
        for (int j = 0; j < PERCENT_SUDOKU_SIZE; j++) {
            grid->grid[i][j] = PERCENT_SUDOKU_EMPTY;
        }
    }
    
    // 使用回溯算法生成完整数独
    solve_percent_sudoku(grid);
}

// 生成百分号数独谜题
PercentSudokuGame* generate_percent_sudoku_puzzle(int difficulty) {
    PercentSudokuGame* game = create_percent_sudoku_game();
    if (game == NULL) {
        print_error("generate_percent_sudoku_puzzle", COMMON_ERROR_INVALID_PARAMETER, "Game is NULL");
        return NULL;
    }
    
    game->difficulty = difficulty;
    
    // 生成完整解答
    generate_complete_percent_sudoku(&game->solution);
    
    // 复制解答到谜题
    copy_percent_sudoku_grid(&game->solution, &game->puzzle);
    
    // 根据难度移除数字
    remove_numbers_for_puzzle(&game->puzzle, &game->solution, difficulty);
    
    // 计算给定数字数量
    game->num_givens = 0;
    for (int i = 0; i < PERCENT_SUDOKU_SIZE; i++) {
        for (int j = 0; j < PERCENT_SUDOKU_SIZE; j++) {
            if (game->puzzle.grid[i][j] != PERCENT_SUDOKU_EMPTY) {
                game->puzzle.is_given[i][j] = 1;
                game->num_givens++;
            }
        }
    }
    
    return game;
}

// 根据难度移除数字创建谜题
void remove_numbers_for_puzzle(PercentSudokuGrid* puzzle, PercentSudokuGrid* solution, int difficulty) {
    (void)solution; // Suppress unused parameter warning
    int num_to_remove;
    
    // 根据难度确定要移除的数字数量
    switch (difficulty) {
        case 1: num_to_remove = 30; break;  // 简单
        case 2: num_to_remove = 40; break;  // 中等
        case 3: num_to_remove = 50; break;  // 困难
        case 4: num_to_remove = 55; break;  // 专家
        case 5: num_to_remove = 60; break;  // 大师
        default: num_to_remove = 40; break;
    }
    
    // 创建位置数组
    int positions[81];
    for (int i = 0; i < 81; i++) {
        positions[i] = i;
    }
    
    // 随机打乱位置
    srand(time(NULL));
    for (int i = 80; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = positions[i];
        positions[i] = positions[j];
        positions[j] = temp;
    }
    
    // 移除数字
    int removed = 0;
    for (int i = 0; i < 81 && removed < num_to_remove; i++) {
        int pos = positions[i];
        int row = pos / 9;
        int col = pos % 9;
        
        int original_num = puzzle->grid[row][col];
        puzzle->grid[row][col] = PERCENT_SUDOKU_EMPTY;
        
        // 检查是否仍有唯一解
        PercentSudokuGrid temp_grid;
        copy_percent_sudoku_grid(puzzle, &temp_grid);
        
        if (count_solutions(&temp_grid) == 1) {
            removed++;
        } else {
            // 恢复数字
            puzzle->grid[row][col] = original_num;
        }
    }
}

// 检查百分号数独是否完整
int is_complete_percent_sudoku(const PercentSudokuGrid* grid) {
    if (grid == NULL) {
        return 0;
    }
    
    for (int i = 0; i < PERCENT_SUDOKU_SIZE; i++) {
        for (int j = 0; j < PERCENT_SUDOKU_SIZE; j++) {
            if (grid->grid[i][j] == PERCENT_SUDOKU_EMPTY) {
                return 0;
            }
        }
    }
    return 1;
}

// 检查百分号数独是否有效
int is_valid_percent_sudoku(const PercentSudokuGrid* grid) {
    if (grid == NULL) {
        return 0;
    }
    
    for (int i = 0; i < PERCENT_SUDOKU_SIZE; i++) {
        for (int j = 0; j < PERCENT_SUDOKU_SIZE; j++) {
            if (grid->grid[i][j] != PERCENT_SUDOKU_EMPTY) {
                if (!is_valid_placement(grid, i, j, grid->grid[i][j])) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

// 计算解的个数
int count_solutions(const PercentSudokuGrid* grid) {
    if (grid == NULL) {
        return 0;
    }
    
    PercentSudokuGrid temp_grid;
    copy_percent_sudoku_grid(grid, &temp_grid);
    
    int solutions = 0;
    count_solutions_recursive(&temp_grid, 0, 0, &solutions);
    
    return solutions;
}

// 递归计算解的个数
void count_solutions_recursive(PercentSudokuGrid* grid, int row, int col, int* count) {
    if (*count >= 2) return;  // 只需要知道是否有唯一解
    
    if (row == PERCENT_SUDOKU_SIZE) {
        (*count)++;
        return;
    }
    
    if (col == PERCENT_SUDOKU_SIZE) {
        count_solutions_recursive(grid, row + 1, 0, count);
        return;
    }
    
    if (grid->grid[row][col] != PERCENT_SUDOKU_EMPTY) {
        count_solutions_recursive(grid, row, col + 1, count);
        return;
    }
    
    for (int num = PERCENT_SUDOKU_MIN_NUM; num <= PERCENT_SUDOKU_MAX_NUM; num++) {
        if (is_valid_placement(grid, row, col, num)) {
            grid->grid[row][col] = num;
            count_solutions_recursive(grid, row, col + 1, count);
            grid->grid[row][col] = PERCENT_SUDOKU_EMPTY;
            
            if (*count >= 2) return;
        }
    }
}

// 检查是否有唯一解
int has_unique_solution(const PercentSudokuGrid* grid) {
    return count_solutions(grid) == 1;
}

// 获取随机数
int get_random_number(int min, int max) {
    return min + rand() % (max - min + 1);
}

// 打乱数组
void shuffle_array(int* array, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// 注意：显示相关函数已移至 display 模块

// 保存百分号数独到文件
void save_percent_sudoku_to_file(const PercentSudokuGame* game, const char* filename) {
    if (game == NULL || filename == NULL) {
        return;
    }
    
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        print_error("save_percent_sudoku_grid", COMMON_ERROR_FILE_NOT_FOUND, "Cannot open file for writing");
        return;
    }
    
    fprintf(file, "# Percent Sudoku Game File\n");
    fprintf(file, "difficulty %d\n", game->difficulty);
    fprintf(file, "num_givens %d\n", game->num_givens);
    fprintf(file, "\n# Puzzle\n");
    
    for (int i = 0; i < PERCENT_SUDOKU_SIZE; i++) {
        for (int j = 0; j < PERCENT_SUDOKU_SIZE; j++) {
            fprintf(file, "%d ", game->puzzle.grid[i][j]);
        }
        fprintf(file, "\n");
    }
    
    fprintf(file, "\n# Solution\n");
    for (int i = 0; i < PERCENT_SUDOKU_SIZE; i++) {
        for (int j = 0; j < PERCENT_SUDOKU_SIZE; j++) {
            fprintf(file, "%d ", game->solution.grid[i][j]);
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    printf("Percent Sudoku saved to: %s\n", filename);
}

// 从文件加载百分号数独
PercentSudokuGame* load_percent_sudoku_from_file(const char* filename) {
    if (filename == NULL) {
        print_error("load_percent_sudoku_grid", COMMON_ERROR_INVALID_PARAMETER, "Filename is NULL");
        return NULL;
    }
    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        print_error("load_percent_sudoku_grid", COMMON_ERROR_FILE_NOT_FOUND, "Cannot open file for reading");
        return NULL;
    }
    
    PercentSudokuGame* game = create_percent_sudoku_game();
    if (game == NULL) {
        fclose(file);
        return NULL;
    }
    
    char line[256];
    int section = 0;  // 0: header, 1: puzzle, 2: solution
    int row = 0;
    
    while (fgets(line, sizeof(line), file)) {
        // 跳过注释行
        if (line[0] == '#') {
            continue;
        }
        
        if (strncmp(line, "difficulty", 10) == 0) {
            sscanf(line, "difficulty %d", &game->difficulty);
        } else if (strncmp(line, "num_givens", 10) == 0) {
            sscanf(line, "num_givens %d", &game->num_givens);
        } else if (strlen(line) > 1) {
            // Read grid data
            if (section == 0) {
                section = 1;  // Start reading puzzle
            }
            
            if (section == 1 && row < PERCENT_SUDOKU_SIZE) {
                sscanf(line, "%d %d %d %d %d %d %d %d %d",
                       &game->puzzle.grid[row][0], &game->puzzle.grid[row][1], &game->puzzle.grid[row][2],
                       &game->puzzle.grid[row][3], &game->puzzle.grid[row][4], &game->puzzle.grid[row][5],
                       &game->puzzle.grid[row][6], &game->puzzle.grid[row][7], &game->puzzle.grid[row][8]);
                row++;
                if (row == PERCENT_SUDOKU_SIZE) {
                    section = 2;  // Start reading solution
                    row = 0;
                }
            } else if (section == 2 && row < PERCENT_SUDOKU_SIZE) {
                sscanf(line, "%d %d %d %d %d %d %d %d %d",
                       &game->solution.grid[row][0], &game->solution.grid[row][1], &game->solution.grid[row][2],
                       &game->solution.grid[row][3], &game->solution.grid[row][4], &game->solution.grid[row][5],
                       &game->solution.grid[row][6], &game->solution.grid[row][7], &game->solution.grid[row][8]);
                row++;
            }
        }
    }
    
    fclose(file);
    return game;
}

// 控制台显示函数实现
void print_percent_sudoku_console(PercentSudokuGrid* grid, int show_constraints) {
    if (grid == NULL) {
        printf("Error: Grid is NULL\n");
        return;
    }
    
    printf("+-----+-----+-----+\n");
    
    for (int i = 0; i < 9; i++) {
        if (i > 0 && i % 3 == 0) {
            printf("+-----+-----+-----+\n");
        }
        
        printf("|");
        for (int j = 0; j < 9; j++) {
            if (j > 0 && j % 3 == 0) {
                printf("|");
            }
            
            int value = grid->grid[i][j];
            if (value == PERCENT_SUDOKU_EMPTY) {
                printf(" . ");
            } else {
                printf(" %d ", value);
            }
        }
        printf("|\n");
    }
    
    printf("+-----+-----+-----+\n");
    
    // 显示约束信息
    if (show_constraints) {
        printf("\nConstraint Information:\n");
        printf("- Anti-diagonal constraint: positions (1,9), (2,8), (3,7), (4,6), (5,5), (6,4), (7,3), (8,2), (9,1)\n");
        printf("- Upper window constraint: positions (2,2)-(4,4)\n");
        printf("- Lower window constraint: positions (6,6)-(8,8)\n");
        printf("- Standard Sudoku constraints: rows, columns, 3x3 boxes\n");
    }
}

void print_percent_sudoku_game_console(PercentSudokuGame* game, int show_solution, int show_constraints) {
    if (game == NULL) {
        printf("Error: Game is NULL\n");
        return;
    }
    
    printf("========================================\n");
    printf("        Percent Sudoku Game\n");
    printf("========================================\n\n");
    
    printf("Difficulty Level: %d\n", game->difficulty);
    printf("Given Numbers: %d\n", game->num_givens);
    printf("\n");
    
    // 显示谜题
    printf("Puzzle:\n");
    print_percent_sudoku_console(&game->puzzle, show_constraints);
    
    // 显示解答（如果需要）
    if (show_solution) {
        printf("\nSolution:\n");
        print_percent_sudoku_console(&game->solution, 0);
    }
    
    printf("\n");
}