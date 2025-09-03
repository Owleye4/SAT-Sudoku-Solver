#include "percent_sudoku_solver.h"
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#endif

// 创建求解器配置
SolverConfig* create_solver_config(void) {
    SolverConfig* config = (SolverConfig*)safe_malloc(sizeof(SolverConfig));
    
    config->use_backtracking = 1;
    config->use_sat_solver = 1;
    config->verbose_output = 0;
    config->save_intermediate = 0;
    config->output_dir = NULL;
    
    return config;
}

// 释放求解器配置内存
void free_solver_config(SolverConfig* config) {
    if (config != NULL) {
        if (config->output_dir != NULL) {
            safe_free(config->output_dir);
        }
        safe_free(config);
    }
}

// 设置求解器选项
void set_solver_options(SolverConfig* config, int backtracking, int sat_solver, int verbose) {
    if (config == NULL) return;
    
    config->use_backtracking = backtracking;
    config->use_sat_solver = sat_solver;
    config->verbose_output = verbose;
}

// get_current_time_ms函数已在common模块中定义

// 初始化求解统计信息
void initialize_solving_stats(PercentSudokuStats* stats) {
    if (stats == NULL) return;
    
    stats->generation_time = 0.0;
    stats->cnf_time = 0.0;
    stats->solving_time = 0.0;
    stats->total_time = 0.0;
    stats->num_variables = 0;
    stats->num_clauses = 0;
    stats->num_givens = 0;
    stats->result = PERCENT_SUDOKU_ERROR;
}

// 更新求解统计信息
void update_solving_stats(PercentSudokuStats* stats, PercentSudokuResult result) {
    if (stats == NULL) return;
    
    stats->result = result;
    stats->total_time = stats->generation_time + stats->cnf_time + stats->solving_time;
}

// 使用SAT求解器求解百分号数独
PercentSudokuResult solve_percent_sudoku_with_sat(PercentSudokuGame* game, double timeout_seconds) {
    if (game == NULL) {
        return PERCENT_SUDOKU_ERROR;
    }
    
    (void)timeout_seconds; // Suppress unused parameter warning
    
    // 转换为CNF
    Formula* formula = percent_sudoku_to_cnf(&game->puzzle);
    if (formula == NULL) {
        return PERCENT_SUDOKU_ERROR;
    }
    
    // 创建变量赋值数组
    int* assignments = (int*)safe_malloc(formula->num_vars * sizeof(int));
    if (assignments == NULL) {
        free_formula(formula);
        return PERCENT_SUDOKU_ERROR;
    }
    
    // 初始化赋值数组为0
    for (int i = 0; i < formula->num_vars; i++) {
        assignments[i] = 0;
    }
    
    // 初始化所有变量为未赋值状态
    for (int i = 0; i < formula->num_vars; i++) {
        assignments[i] = UNASSIGNED;
    }
    
    // 使用DPLL求解
    int result = DPLL(formula, assignments);
    
    PercentSudokuResult percent_sudoku_result;
    if (result == SAT) {
        // 从CNF解还原百分号数独
        PercentSudokuGrid* solution = cnf_to_percent_sudoku(formula, assignments, &game->puzzle);
        if (solution != NULL) {
            copy_percent_sudoku_grid(solution, &game->solution);
            free_percent_sudoku_grid(solution);
            percent_sudoku_result = PERCENT_SUDOKU_SOLVED;
        } else {
            percent_sudoku_result = PERCENT_SUDOKU_ERROR;
        }
    } else if (result == UNSAT) {
        percent_sudoku_result = PERCENT_SUDOKU_UNSOLVABLE;
    } else if (result == TIMEOUT) {
        percent_sudoku_result = PERCENT_SUDOKU_TIMEOUT;
    } else {
        percent_sudoku_result = PERCENT_SUDOKU_ERROR;
    }
    
    // 清理内存
    safe_free(assignments);
    free_formula(formula);
    
    return percent_sudoku_result;
}

// 求解百分号数独谜题
PercentSudokuResult solve_percent_sudoku_puzzle(const PercentSudokuGrid* puzzle, PercentSudokuGrid* solution, double timeout_seconds) {
    if (puzzle == NULL || solution == NULL) {
        return PERCENT_SUDOKU_ERROR;
    }
    
    // 复制谜题到解答
    copy_percent_sudoku_grid(puzzle, solution);
    
    // 使用回溯算法求解
    double start_time = get_current_time_ms();
    int solved = solve_percent_sudoku(solution);
    double solving_time = get_current_time_ms() - start_time;
    
    // 检查是否超时
    if (solving_time > timeout_seconds * 1000.0) {
        return PERCENT_SUDOKU_TIMEOUT;
    }
    
    if (solved) {
        return PERCENT_SUDOKU_SOLVED;
    } else {
        return PERCENT_SUDOKU_UNSOLVABLE;
    }
}

// 生成并求解百分号数独
PercentSudokuResult generate_and_solve_percent_sudoku(int difficulty, double timeout_seconds, PercentSudokuStats* stats) {
    if (stats == NULL) {
        return PERCENT_SUDOKU_ERROR;
    }
    
    initialize_solving_stats(stats);
    
    // 生成百分号数独
    double gen_start = get_current_time_ms();
    PercentSudokuGame* game = generate_percent_sudoku_puzzle(difficulty);
    if (game == NULL) {
        return PERCENT_SUDOKU_ERROR;
    }
    stats->generation_time = get_current_time_ms() - gen_start;
    
    // 统计信息
    stats->num_givens = game->num_givens;
    
    // 转换为CNF并统计
    double cnf_start = get_current_time_ms();
    Formula* formula = percent_sudoku_to_cnf(&game->puzzle);
    if (formula == NULL) {
        free_percent_sudoku_game(game);
        return PERCENT_SUDOKU_ERROR;
    }
    stats->cnf_time = get_current_time_ms() - cnf_start;
    stats->num_variables = formula->num_vars;
    stats->num_clauses = formula->num_clauses;
    
    // 求解
    double solve_start = get_current_time_ms();
    PercentSudokuResult result = solve_percent_sudoku_with_sat(game, timeout_seconds);
    stats->solving_time = get_current_time_ms() - solve_start;
    
    update_solving_stats(stats, result);
    
    // 清理内存
    free_formula(formula);
    free_percent_sudoku_game(game);
    
    return result;
}

// 从文件求解百分号数独
PercentSudokuResult solve_from_file(const char* puzzle_file, double timeout_seconds, PercentSudokuStats* stats) {
    if (puzzle_file == NULL || stats == NULL) {
        return PERCENT_SUDOKU_ERROR;
    }
    
    initialize_solving_stats(stats);
    
    // 加载百分号数独
    PercentSudokuGame* game = load_percent_sudoku_from_file(puzzle_file);
    if (game == NULL) {
        return PERCENT_SUDOKU_ERROR;
    }
    
    stats->num_givens = game->num_givens;
    
    // 转换为CNF
    double cnf_start = get_current_time_ms();
    Formula* formula = percent_sudoku_to_cnf(&game->puzzle);
    if (formula == NULL) {
        free_percent_sudoku_game(game);
        return PERCENT_SUDOKU_ERROR;
    }
    stats->cnf_time = get_current_time_ms() - cnf_start;
    stats->num_variables = formula->num_vars;
    stats->num_clauses = formula->num_clauses;
    
    // 求解
    double solve_start = get_current_time_ms();
    PercentSudokuResult result = solve_percent_sudoku_with_sat(game, timeout_seconds);
    stats->solving_time = get_current_time_ms() - solve_start;
    
    update_solving_stats(stats, result);
    
    // 清理内存
    free_formula(formula);
    free_percent_sudoku_game(game);
    
    return result;
}

// 验证解答正确性
int verify_solution_correctness(const PercentSudokuGrid* puzzle, const PercentSudokuGrid* solution) {
    if (puzzle == NULL || solution == NULL) {
        return 0;
    }
    
    // 检查解答是否完整
    if (!is_complete_percent_sudoku(solution)) {
        printf("Error: Solution is incomplete\n");
        return 0;
    }
    
    // 检查解答是否有效
    if (!is_valid_percent_sudoku(solution)) {
        printf("Error: Solution is invalid\n");
        return 0;
    }
    
    // 检查解答是否与谜题一致
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (puzzle->grid[i][j] != PERCENT_SUDOKU_EMPTY && 
                puzzle->grid[i][j] != solution->grid[i][j]) {
                printf("Error: Solution doesn't match puzzle at position (%d,%d)\n", i+1, j+1);
                return 0;
            }
        }
    }
    
    return 1;
}

// 打印求解统计信息
void print_solving_stats(const PercentSudokuStats* stats) {
    if (stats == NULL) {
        printf("Statistics are empty\n");
        return;
    }
    
    printf("\n=== Percent Sudoku Solving Statistics ===\n");
    printf("Generation time: %.2f ms\n", stats->generation_time);
    printf("CNF conversion time: %.2f ms\n", stats->cnf_time);
    printf("Solving time: %.2f ms\n", stats->solving_time);
    printf("Total time: %.2f ms\n", stats->total_time);
    printf("Number of variables: %d\n", stats->num_variables);
    printf("Number of clauses: %d\n", stats->num_clauses);
    printf("Given numbers: %d\n", stats->num_givens);
    
    printf("Solving result: ");
    switch (stats->result) {
        case PERCENT_SUDOKU_SOLVED:
            printf("Successfully solved\n");
            break;
        case PERCENT_SUDOKU_UNSOLVABLE:
            printf("No solution\n");
            break;
        case PERCENT_SUDOKU_TIMEOUT:
            printf("Timeout\n");
            break;
        case PERCENT_SUDOKU_ERROR:
            printf("Error\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }
    printf("=====================================\n");
}

// 保存求解报告
void save_solving_report(const PercentSudokuStats* stats, const char* filename) {
    if (stats == NULL || filename == NULL) {
        return;
    }
    
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        print_error("save_solving_report", COMMON_ERROR_FILE_NOT_FOUND, "Cannot create report file");
        return;
    }
    
    fprintf(file, "Percent Sudoku Solving Report\n");
    fprintf(file, "=======================\n\n");
    fprintf(file, "Generation time: %.2f ms\n", stats->generation_time);
    fprintf(file, "CNF conversion time: %.2f ms\n", stats->cnf_time);
    fprintf(file, "Solving time: %.2f ms\n", stats->solving_time);
    fprintf(file, "Total time: %.2f ms\n", stats->total_time);
    fprintf(file, "Number of variables: %d\n", stats->num_variables);
    fprintf(file, "Number of clauses: %d\n", stats->num_clauses);
    fprintf(file, "Given numbers: %d\n", stats->num_givens);
    
    fprintf(file, "Solving result: ");
    switch (stats->result) {
        case PERCENT_SUDOKU_SOLVED:
            fprintf(file, "Successfully solved\n");
            break;
        case PERCENT_SUDOKU_UNSOLVABLE:
            fprintf(file, "No solution\n");
            break;
        case PERCENT_SUDOKU_TIMEOUT:
            fprintf(file, "Timeout\n");
            break;
        case PERCENT_SUDOKU_ERROR:
            fprintf(file, "Error\n");
            break;
        default:
            fprintf(file, "Unknown\n");
            break;
    }
    
    fclose(file);
    printf("Solving report saved to: %s\n", filename);
}

// 比较求解方法
void compare_solving_methods(const PercentSudokuGrid* puzzle, double timeout_seconds) {
    if (puzzle == NULL) {
        return;
    }
    
    printf("\n=== Solving Method Comparison ===\n");
    
    // Method 1: Backtracking algorithm
    printf("Method 1: Backtracking algorithm\n");
    PercentSudokuGrid* solution1 = create_percent_sudoku_grid();
    if (solution1 != NULL) {
        double start_time = get_current_time_ms();
        PercentSudokuResult result1 = solve_percent_sudoku_puzzle(puzzle, solution1, timeout_seconds);
        double time1 = get_current_time_ms() - start_time;
        
        printf("  Result: ");
        switch (result1) {
            case PERCENT_SUDOKU_SOLVED: printf("Success"); break;
            case PERCENT_SUDOKU_UNSOLVABLE: printf("No solution"); break;
            case PERCENT_SUDOKU_TIMEOUT: printf("Timeout"); break;
            default: printf("Error"); break;
        }
        printf("\n");
        printf("  Time: %.2f ms\n", time1);
        
        free_percent_sudoku_grid(solution1);
    }
    
    // Method 2: SAT solver
    printf("Method 2: SAT solver\n");
    PercentSudokuGame* game = create_percent_sudoku_game();
    if (game != NULL) {
        copy_percent_sudoku_grid(puzzle, &game->puzzle);
        
        double start_time = get_current_time_ms();
        PercentSudokuResult result2 = solve_percent_sudoku_with_sat(game, timeout_seconds);
        double time2 = get_current_time_ms() - start_time;
        
        printf("  Result: ");
        switch (result2) {
            case PERCENT_SUDOKU_SOLVED: printf("Success"); break;
            case PERCENT_SUDOKU_UNSOLVABLE: printf("No solution"); break;
            case PERCENT_SUDOKU_TIMEOUT: printf("Timeout"); break;
            default: printf("Error"); break;
        }
        printf("\n");
        printf("  Time: %.2f ms\n", time2);
        
        free_percent_sudoku_game(game);
    }
    
    printf("===============================\n");
}

// 测试百分号数独求解器
int test_percent_sudoku_solver(void) {
    printf("Starting Percent Sudoku solver tests...\n");
    
    int passed = 0;
    int total = 0;
    
    // Test 1: Generate simple Percent Sudoku
    total++;
    printf("Test 1: Generate simple Percent Sudoku...\n");
    PercentSudokuGame* game = generate_percent_sudoku_puzzle(1);
    if (game != NULL && is_valid_percent_sudoku(&game->puzzle)) {
        printf("  ✓ Passed\n");
        passed++;
        free_percent_sudoku_game(game);
    } else {
        printf("  ✗ Failed\n");
    }
    
    // Test 2: Solve Percent Sudoku
    total++;
    printf("Test 2: Solve Percent Sudoku...\n");
    if (game != NULL) {
        PercentSudokuResult result = solve_percent_sudoku_with_sat(game, 10.0);
        if (result == PERCENT_SUDOKU_SOLVED && is_valid_percent_sudoku(&game->solution)) {
            printf("  ✓ Passed\n");
            passed++;
        } else {
            printf("  ✗ Failed\n");
        }
        free_percent_sudoku_game(game);
    } else {
        printf("  ✗ Failed\n");
    }
    
    // Test 3: CNF conversion
    total++;
    printf("Test 3: CNF conversion...\n");
    PercentSudokuGame* test_game = generate_percent_sudoku_puzzle(1);
    if (test_game != NULL) {
        Formula* formula = percent_sudoku_to_cnf(&test_game->puzzle);
        if (formula != NULL && formula->num_vars == 729) {
            printf("  ✓ Passed (Variables: %d, Clauses: %d)\n", formula->num_vars, formula->num_clauses);
            passed++;
            free_formula(formula);
        } else {
            printf("  ✗ Failed\n");
        }
        free_percent_sudoku_game(test_game);
    } else {
        printf("  ✗ Failed\n");
    }
    
    printf("Test results: %d/%d passed\n", passed, total);
    return (passed == total) ? 1 : 0;
}

// 运行百分号数独测试
void run_percent_sudoku_tests(void) {
    printf("=== Percent Sudoku Solver Tests ===\n");
    
    if (test_percent_sudoku_solver()) {
        printf("All tests passed!\n");
    } else {
        printf("Some tests failed!\n");
    }
    
    printf("=============================\n");
}
