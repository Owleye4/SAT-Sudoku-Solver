#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif
#include "src/core/core.h"
#include "src/parser/parser.h"
#include "src/solver/solver.h"
#include "src/percent_sudoku/percent_sudoku.h"
#include "src/display/gui_display.h"

// 全局变量
static volatile int timeout_flag = 0;
static double timeout_seconds = 300.0; // 默认5分钟超时

// 函数声明
void print_usage(const char* program_name);
void print_solution_summary(const Formula* formula, int* assignments, int result, double time_spent);
void save_solution_to_file(const char* filename, const Formula* formula, int* assignments, int result, double time_spent);
void print_formula_details(const Formula* formula);
void timeout_handler(int sig);
int check_timeout(double start_time);

// 百分号数独相关函数声明
void run_percent_sudoku_mode(int argc, char* argv[]);
void run_percent_sudoku_generate_mode(int difficulty, double timeout);
void run_percent_sudoku_solve_mode(const char* puzzle_file, double timeout);
void run_percent_sudoku_test_mode(void);
void run_percent_sudoku_demo_mode(void);
void run_percent_sudoku_gui_mode(int difficulty);

int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // 检查是否为百分号数独模式
    if (strcmp(argv[1], "--percent-sudoku") == 0) {
        run_percent_sudoku_mode(argc, argv);
        return 0;
    }

    // 原有的CNF求解模式
    if (argc > 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char* cnf_filename = argv[1];
    
    // 设置超时时间（如果提供）
    if (argc == 3) {
        timeout_seconds = atof(argv[2]);
        if (timeout_seconds <= 0) {
            fprintf(stderr, "Error: Invalid timeout value\n");
            return 1;
        }
    }
    
    // 设置超时处理（Windows兼容）
#ifdef _WIN32
    // Windows下使用线程超时处理
#else
    signal(SIGALRM, timeout_handler);
#endif
    
    printf("========================================\n");
    printf("        SAT Solver - Main Module\n");
    printf("========================================\n\n");
    
    // ⑴ 输入输出功能：读取CNF文件
    printf("Reading CNF file: %s\n", cnf_filename);
    Formula* formula = parse_cnf_file(cnf_filename);
    if (formula == NULL) {
        fprintf(stderr, "Error: Cannot parse CNF file %s\n", cnf_filename);
        return 1;
    }
    printf("✓ CNF file parsed successfully\n");
    
    // ⑵ 公式解析与验证：显示解析结果
    printf("\n=== Formula Parsing Results ===\n");
    printf("Number of variables: %d\n", formula->num_vars);
    printf("Number of clauses: %d\n", formula->num_clauses);
    
    printf("\n=== Clause Details ===\n");
    print_formula_details(formula);
    
    // 创建变量赋值数组
    int* assignments = (int*)calloc(formula->num_vars, sizeof(int));
    if (assignments == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free_formula(formula);
        return 1;
    }
    
    // 初始化所有变量为未赋值状态
    for (int i = 0; i < formula->num_vars; i++) {
        assignments[i] = UNASSIGNED;
    }
    
    printf("\n=== Starting DPLL Solving ===\n");
    printf("Timeout set to: %.1f seconds\n", timeout_seconds);
    
    // ⑷ 时间性能测量：记录DPLL执行时间
    clock_t begin = clock();
    double start_time = (double)begin / CLOCKS_PER_SEC;
    
    // ⑶ DPLL过程：调用DPLL算法求解
    int result = DPLL(formula, assignments);
    
    // 检查是否超时
    if (check_timeout(start_time)) {
        result = TIMEOUT;
        printf("Solving timed out after %.1f seconds\n", timeout_seconds);
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    double time_spent_ms = time_spent * 1000.0;
    
    printf("\n=== Solving Completed ===\n");
    
    // 输出求解结果
    print_solution_summary(formula, assignments, result, time_spent_ms);
    
    // 保存结果到文件
    save_solution_to_file(cnf_filename, formula, assignments, result, time_spent_ms);
    
    // 清理内存
    free(assignments);
    free_formula(formula);
    
    printf("\nProgram execution completed!\n");
    return 0;
}

// 运行百分号数独模式
void run_percent_sudoku_mode(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Error: Percent Sudoku mode requires a command\n");
        printf("Usage: %s --percent-sudoku <command> [options]\n", argv[0]);
        printf("Commands: generate, solve, test, demo\n");
        return;
    }
    
    const char* command = argv[2];
    
    if (strcmp(command, "generate") == 0) {
        int difficulty = 3;  // 默认难度
        double timeout = 60.0;  // 默认超时
        
        if (argc >= 4) {
            difficulty = atoi(argv[3]);
            if (difficulty < 1 || difficulty > 5) {
                printf("Error: Difficulty must be between 1-5\n");
                return;
            }
        }
        
        if (argc >= 5) {
            timeout = atof(argv[4]);
            if (timeout <= 0) {
                printf("Error: Timeout must be greater than 0\n");
                return;
            }
        }
        
        run_percent_sudoku_generate_mode(difficulty, timeout);
        
    } else if (strcmp(command, "solve") == 0) {
        if (argc < 4) {
            printf("Error: Solve mode requires a puzzle file\n");
            printf("Usage: %s --percent-sudoku solve <puzzle_file> [timeout]\n", argv[0]);
            return;
        }
        
        const char* puzzle_file = argv[3];
        double timeout = 60.0;  // 默认超时
        
        if (argc >= 5) {
            timeout = atof(argv[4]);
            if (timeout <= 0) {
                printf("Error: Timeout must be greater than 0\n");
                return;
            }
        }
        
        run_percent_sudoku_solve_mode(puzzle_file, timeout);
        
    } else if (strcmp(command, "test") == 0) {
        run_percent_sudoku_test_mode();
        
    } else if (strcmp(command, "demo") == 0) {
        run_percent_sudoku_demo_mode();
        
    } else if (strcmp(command, "gui") == 0) {
        int difficulty = 2; // 默认难度
        if (argc > 3) {
            difficulty = atoi(argv[3]);
            if (difficulty < 1 || difficulty > 5) {
                printf("Invalid difficulty. Using default difficulty 2.\n");
                difficulty = 2;
            }
        }
        run_percent_sudoku_gui_mode(difficulty);
        
    } else {
        printf("Error: Unknown command '%s'\n", command);
        printf("Available commands: generate, solve, gui, test, demo\n");
    }
}

// 运行百分号数独生成模式
void run_percent_sudoku_generate_mode(int difficulty, double timeout) {
    printf("========================================\n");
    printf("        Percent Sudoku Generator\n");
    printf("========================================\n\n");
    
    printf("Generation Parameters:\n");
    printf("  Difficulty Level: %d\n", difficulty);
    printf("  Timeout: %.1f seconds\n", timeout);
    printf("\n");
    
    PercentSudokuStats stats;
    PercentSudokuResult result = generate_and_solve_percent_sudoku(difficulty, timeout, &stats);
    
    if (result == PERCENT_SUDOKU_SOLVED) {
        printf("Percent Sudoku generation and solving successful!\n\n");
        
        // 显示统计信息
        print_solving_stats(&stats);
        
        // 生成并显示游戏
        PercentSudokuGame* game = generate_percent_sudoku_puzzle(difficulty);
        if (game != NULL) {
            // 控制台显示
            print_percent_sudoku_game_console(game, 1, 1);
            
            // 保存到文件
            char filename[256];
            sprintf(filename, "percent_sudoku_difficulty_%d.percent_sudoku", difficulty);
            save_percent_sudoku_to_file(game, filename);
            
            free_percent_sudoku_game(game);
        }
        
    } else {
        printf("Percent Sudoku generation failed\n");
        switch (result) {
            case PERCENT_SUDOKU_UNSOLVABLE:
                printf("Reason: Unsolvable\n");
                break;
            case PERCENT_SUDOKU_TIMEOUT:
                printf("Reason: Timeout\n");
                break;
            case PERCENT_SUDOKU_ERROR:
                printf("Reason: Error\n");
                break;
            default:
                printf("Reason: Unknown\n");
                break;
        }
    }
}

// 运行百分号数独求解模式
void run_percent_sudoku_solve_mode(const char* puzzle_file, double timeout) {
    printf("========================================\n");
    printf("        Percent Sudoku Solver\n");
    printf("========================================\n\n");
    
    printf("Solving Parameters:\n");
    printf("  Puzzle File: %s\n", puzzle_file);
    printf("  Timeout: %.1f seconds\n", timeout);
    printf("\n");
    
    PercentSudokuStats stats;
    PercentSudokuResult result = solve_from_file(puzzle_file, timeout, &stats);
    
    if (result == PERCENT_SUDOKU_SOLVED) {
        printf("Percent Sudoku solving successful!\n\n");
        
        // 显示统计信息
        print_solving_stats(&stats);
        
        // 加载并显示游戏
        PercentSudokuGame* game = load_percent_sudoku_from_file(puzzle_file);
        if (game != NULL) {
            // 控制台显示
            print_percent_sudoku_game_console(game, 1, 1);
            
            // 保存解答到文件
            char solution_file[256];
            strcpy(solution_file, puzzle_file);
            char* dot = strrchr(solution_file, '.');
            if (dot != NULL) {
                strcpy(dot, "_solution.percent_sudoku");
            } else {
                strcat(solution_file, "_solution.percent_sudoku");
            }
            save_percent_sudoku_to_file(game, solution_file);
            
            free_percent_sudoku_game(game);
        }
        
    } else {
        printf("Percent Sudoku solving failed\n");
        switch (result) {
            case PERCENT_SUDOKU_UNSOLVABLE:
                printf("Reason: Unsolvable\n");
                break;
            case PERCENT_SUDOKU_TIMEOUT:
                printf("Reason: Timeout\n");
                break;
            case PERCENT_SUDOKU_ERROR:
                printf("Reason: Error\n");
                break;
            default:
                printf("Reason: Unknown\n");
                break;
        }
    }
}

// 运行百分号数独测试模式
void run_percent_sudoku_test_mode(void) {
    printf("========================================\n");
    printf("        Percent Sudoku Test Mode\n");
    printf("========================================\n\n");
    
    run_percent_sudoku_tests();
}

// 运行百分号数独演示模式
void run_percent_sudoku_demo_mode(void) {
    printf("========================================\n");
    printf("        Percent Sudoku Demo Mode\n");
    printf("========================================\n\n");
    
    printf("Demo content:\n");
    printf("1. Generate Percent Sudoku puzzles of different difficulties\n");
    printf("2. Show constraint information\n");
    printf("3. Demonstrate solving process\n");
    printf("4. Compare solving methods\n\n");
    
    // Demo 1: Generate simple Percent Sudoku
    printf("=== Demo 1: Generate Simple Percent Sudoku ===\n");
    PercentSudokuGame* easy_game = generate_percent_sudoku_puzzle(1);
    if (easy_game != NULL) {
        // 控制台显示
        print_percent_sudoku_game_console(easy_game, 1, 1);
        free_percent_sudoku_game(easy_game);
    }
    
    printf("\n");
    
    // Demo 2: Generate difficult Percent Sudoku
    printf("=== Demo 2: Generate Difficult Percent Sudoku ===\n");
    PercentSudokuGame* hard_game = generate_percent_sudoku_puzzle(4);
    if (hard_game != NULL) {
        // 控制台显示
        print_percent_sudoku_game_console(hard_game, 1, 1);
        
        // Demo solving
        printf("\nStarting solving...\n");
        PercentSudokuStats stats;
        PercentSudokuResult result = solve_percent_sudoku_with_sat(hard_game, 30.0);
        
        if (result == PERCENT_SUDOKU_SOLVED) {
            printf("✓ Solving successful!\n");
            print_solving_stats(&stats);
            
            // Display solution
            printf("\nSolution:\n");
            print_percent_sudoku_console(&hard_game->solution, 0);
        } else {
            printf("✗ Solving failed\n");
        }
        
        free_percent_sudoku_game(hard_game);
    }
    
    printf("\n=== Demo Completed ===\n");
}

// 运行百分号数独GUI模式
void run_percent_sudoku_gui_mode(int difficulty) {
    printf("========================================\n");
    printf("        Percent Sudoku GUI Mode\n");
    printf("========================================\n\n");
    
    printf("Starting interactive Percent Sudoku game...\n");
    printf("Difficulty: %d\n", difficulty);
    printf("\nInstructions:\n");
    printf("- Click on cells to select them\n");
    printf("- Use number keys 1-9 to fill cells\n");
    printf("- Use Delete/Backspace to clear cells\n");
    printf("- Use arrow keys to navigate\n");
    printf("- Press Ctrl+S to solve, Ctrl+N for new puzzle\n");
    printf("- Press Ctrl+H for hint, Ctrl+C to check solution\n\n");
    
    // 生成游戏
    PercentSudokuGame* game = generate_percent_sudoku_puzzle(difficulty);
    if (game == NULL) {
        printf("Error: Failed to generate Percent Sudoku puzzle\n");
        return;
    }
    
    printf("Puzzle generated successfully!\n");
    printf("Given numbers: %d\n", game->num_givens);
    printf("Opening GUI window...\n\n");
    
    // 启动GUI
    display_percent_sudoku_gui(game, difficulty);
    
    // 清理
    free_percent_sudoku_game(game);
    
    printf("GUI mode completed.\n");
}

// 打印使用说明
void print_usage(const char* program_name) {
    printf("SAT Solver - Supports CNF solving and Percent Sudoku solving\n");
    printf("=====================================================\n\n");
    
    printf("Usage 1 - CNF solving mode:\n");
    printf("  %s <cnf_file_path> [timeout_seconds]\n", program_name);
    printf("\n  Examples:\n");
    printf("    %s tests/cases/small/small_sat_1.cnf\n", program_name);
    printf("    %s tests/cases/medium/medium_sat_1.cnf 60\n", program_name);
    printf("    %s tests/cases/large/large_sat_1.cnf 300\n", program_name);
    
    printf("\nUsage 2 - Percent Sudoku mode:\n");
    printf("  %s --percent-sudoku <command> [options]\n", program_name);
    printf("\n  Commands:\n");
    printf("    generate <difficulty> [timeout]  - Generate Percent Sudoku puzzle\n");
    printf("    solve <puzzle_file> [timeout]    - Solve Percent Sudoku file\n");
    printf("    gui [difficulty]                 - Run interactive GUI\n");
    printf("    test                             - Run tests\n");
    printf("    demo                             - Run demo\n");
    printf("\n  Examples:\n");
    printf("    %s --percent-sudoku generate 3 60\n", program_name);
    printf("    %s --percent-sudoku solve puzzle.percent_sudoku 30\n", program_name);
    printf("    %s --percent-sudoku gui 2\n", program_name);
    printf("    %s --percent-sudoku test\n", program_name);
    printf("    %s --percent-sudoku demo\n", program_name);
    
    printf("\nParameter Description:\n");
    printf("  cnf_file_path: CNF file path\n");
    printf("  timeout_seconds: Timeout time (seconds, default 300)\n");
    printf("  difficulty: Percent Sudoku difficulty (1-5, default 3)\n");
    printf("  puzzle_file: Percent Sudoku file path\n");
}

// 打印求解结果摘要（控制台输出）
void print_solution_summary(const Formula* formula, int* assignments, int result, double time_spent) {
    if (result == SAT) {
        printf("Solving result: SATISFIABLE\n");
    } else if (result == UNSAT) {
        printf("Solving result: UNSATISFIABLE\n");
    } else if (result == TIMEOUT) {
        printf("Solving result: TIMEOUT\n");
    }
    printf("Execution time: %.2f ms (%.6f seconds)\n", time_spent, time_spent / 1000.0);
    
    if (result == SAT) {
        printf("\nSatisfying assignment:\n");
        print_assignments(formula, assignments);
        
        // 验证解的正确性
        printf("\nVerifying solution correctness...\n");
        if (is_formula_satisfied(formula, assignments)) {
            printf("Verification passed: All clauses are satisfied\n");
        } else {
            printf("Verification failed: Some clauses are not satisfied\n");
        }
        
        // 显示DIMACS格式的解
        printf("\nDIMACS format solution:\n");
        for (int i = 0; i < formula->num_vars; i++) {
            if (assignments[i] == TRUE) {
                printf("%d ", i + 1);
            } else if (assignments[i] == FALSE) {
                printf("%d ", -(i + 1));
            }
        }
        printf("0\n");
    } else {
        printf("\nThe formula is unsatisfiable\n");
    }
}

// 保存结果到文件
void save_solution_to_file(const char* filename, const Formula* formula, int* assignments, int result, double time_spent) {
    // 构造.res文件名
    char res_filename[512];
    strcpy(res_filename, filename);
    
    // 找到最后一个点号，替换扩展名为.res
    char* last_dot = strrchr(res_filename, '.');
    if (last_dot != NULL) {
        strcpy(last_dot, ".res");
    } else {
        strcat(res_filename, ".res");
    }
    
    FILE* file = fopen(res_filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Warning: Cannot create result file %s\n", res_filename);
        return;
    }
    
    // 按照新规范输出
    // s求解结果//1表示满足，0表示不满足，-1表示在限定时间内未完成求解
    if (result == SAT) {
        fprintf(file, "s 1\n");
    } else if (result == UNSAT) {
        fprintf(file, "s 0\n");
    } else if (result == TIMEOUT) {
        fprintf(file, "s -1\n");
    } else {
        fprintf(file, "s -1\n");  // 其他未知情况也标记为超时
    }
    
    // v -1 2 -3 … //满足时，每个变元的赋值序列
    if (result == SAT) {
        fprintf(file, "v");
        for (int i = 0; i < formula->num_vars; i++) {
            if (assignments[i] == TRUE) {
                fprintf(file, " %d", i + 1);  // 正数表示取真
            } else if (assignments[i] == FALSE) {
                fprintf(file, " %d", -(i + 1));  // 负数表示取假
            } else {
                // 根据规范，未赋值的变量应该默认为假（负数）
                fprintf(file, " %d", -(i + 1));  // 负数表示取假
            }
        }
        fprintf(file, "\n");
    }
    
    // t 17     //以毫秒为单位的DPLL执行时间
    fprintf(file, "t %.0f\n", time_spent);
    
    fclose(file);
    printf("Results saved to: %s\n", res_filename);
}

// 打印公式详细内容（用于验证解析正确性）
void print_formula_details(const Formula* formula) {
    for (int i = 0; i < formula->num_clauses; i++) {
        const Clause* clause = &formula->clauses[i];
        printf("Clause %d: ", i + 1);
        
        for (int j = 0; j < clause->length; j++) {
            int literal = clause->literals[j];
            if (j > 0) printf(" OR ");
            
            if (literal > 0) {
                printf("x%d", literal);
            } else if (literal < 0) {
                printf("NOT x%d", -literal);
            } else {
                printf("0"); // 子句结束符
            }
        }
        printf("\n");
    }
}

// 超时信号处理函数（Unix/Linux）
#ifndef _WIN32
void timeout_handler(int sig) {
    timeout_flag = 1;
    printf("\nTimeout signal received! Stopping solver...\n");
}
#endif

// 检查是否超时（辅助函数）
int check_timeout(double start_time) {
    if (timeout_flag) {
        return 1;
    }
    
    double current_time = (double)clock() / CLOCKS_PER_SEC;
    if (current_time - start_time > timeout_seconds) {
        timeout_flag = 1;
        return 1;
    }
    
    return 0;
}
