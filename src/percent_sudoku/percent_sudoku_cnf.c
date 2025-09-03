#include "percent_sudoku_cnf.h"
#include "../solver/solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Get variable index: the variable number corresponding to digit k at position (i, j)
int get_variable_index(int row, int col, int num) {
    if (row < 0 || row >= 9 || col < 0 || col >= 9 || num < 1 || num > 9) {
        return -1;  // Invalid parameter
    }
    return (row * 9 + col) * 9 + num;
}

// Get position and digit from variable index
void get_position_from_variable(int var_index, int* row, int* col, int* num) {
    if (var_index < 1 || var_index > 729) {  // 9*9*9 = 729 variables
        *row = -1;
        *col = -1;
        *num = -1;
        return;
    }
    
    var_index--;  // Convert to 0-based index
    *num = (var_index % 9) + 1;
    var_index /= 9;
    *col = var_index % 9;
    *row = var_index / 9;
}

// Add cell constraints: each cell must contain a digit from 1 to 9
void add_cell_constraints(Formula* formula) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            Clause* clause = create_clause(9);
            
            // Each cell must contain at least one digit (1-9)
            for (int k = 1; k <= 9; k++) {
                int var_index = get_variable_index(i, j, k);
                add_literal(clause, var_index);
            }
            
            // Add clause to formula
            formula->clauses[formula->num_clauses] = *clause;
            formula->num_clauses++;
            safe_free(clause);
        }
    }
}

// Add row constraints: each digit appears at most once in each row
void add_row_constraints(Formula* formula) {
    for (int i = 0; i < 9; i++) {
        for (int k = 1; k <= 9; k++) {
            // Each digit appears at most once in each row
            for (int j1 = 0; j1 < 9; j1++) {
                for (int j2 = j1 + 1; j2 < 9; j2++) {
                    Clause* clause = create_clause(2);
                    int var1 = get_variable_index(i, j1, k);
                    int var2 = get_variable_index(i, j2, k);
                    add_literal(clause, -var1);  // Negate first variable
                    add_literal(clause, -var2);  // Negate second variable
                    
                    formula->clauses[formula->num_clauses] = *clause;
                    formula->num_clauses++;
                    safe_free(clause);
                }
            }
        }
    }
}

// Add column constraints: each digit appears at most once in each column
void add_column_constraints(Formula* formula) {
    for (int j = 0; j < 9; j++) {
        for (int k = 1; k <= 9; k++) {
            // Each digit appears at most once in each column
            for (int i1 = 0; i1 < 9; i1++) {
                for (int i2 = i1 + 1; i2 < 9; i2++) {
                    Clause* clause = create_clause(2);
                    int var1 = get_variable_index(i1, j, k);
                    int var2 = get_variable_index(i2, j, k);
                    add_literal(clause, -var1);  // Negate first variable
                    add_literal(clause, -var2);  // Negate second variable
                    
                    formula->clauses[formula->num_clauses] = *clause;
                    formula->num_clauses++;
                    safe_free(clause);
                }
            }
        }
    }
}

// Add box constraints: each digit appears at most once in each 3x3 box
void add_box_constraints(Formula* formula) {
    for (int box_row = 0; box_row < 3; box_row++) {
        for (int box_col = 0; box_col < 3; box_col++) {
            for (int k = 1; k <= 9; k++) {
                // Each digit appears at most once in each box
                for (int i1 = 0; i1 < 3; i1++) {
                    for (int j1 = 0; j1 < 3; j1++) {
                        for (int i2 = 0; i2 < 3; i2++) {
                            for (int j2 = 0; j2 < 3; j2++) {
                                if (i1 != i2 || j1 != j2) {
                                    int row1 = box_row * 3 + i1;
                                    int col1 = box_col * 3 + j1;
                                    int row2 = box_row * 3 + i2;
                                    int col2 = box_col * 3 + j2;
                                    
                                    // Avoid duplicate constraints
                                    if ((row1 * 9 + col1) < (row2 * 9 + col2)) {
                                        Clause* clause = create_clause(2);
                                        int var1 = get_variable_index(row1, col1, k);
                                        int var2 = get_variable_index(row2, col2, k);
                                        add_literal(clause, -var1);
                                        add_literal(clause, -var2);
                                        
                                        formula->clauses[formula->num_clauses] = *clause;
                                        formula->num_clauses++;
                                        safe_free(clause);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Add anti-diagonal constraints
void add_diagonal_constraints(Formula* formula) {
    // Anti-diagonal: i + j = 8 (array index starts from 0)
    for (int k = 1; k <= 9; k++) {
        // Each digit appears at most once on the anti-diagonal
        for (int i1 = 0; i1 < 9; i1++) {
            int j1 = 8 - i1;  // Anti-diagonal: i + j = 8
            for (int i2 = i1 + 1; i2 < 9; i2++) {
                int j2 = 8 - i2;
                
                Clause* clause = create_clause(2);
                int var1 = get_variable_index(i1, j1, k);
                int var2 = get_variable_index(i2, j2, k);
                add_literal(clause, -var1);
                add_literal(clause, -var2);
                
                formula->clauses[formula->num_clauses] = *clause;
                formula->num_clauses++;
                safe_free(clause);
            }
        }
    }
}

// Add window constraints
void add_window_constraints(Formula* formula) {
    // Upper window: (1,1) to (3,3)
    for (int k = 1; k <= 9; k++) {
        for (int i1 = 1; i1 <= 3; i1++) {
            for (int j1 = 1; j1 <= 3; j1++) {
                for (int i2 = 1; i2 <= 3; i2++) {
                    for (int j2 = 1; j2 <= 3; j2++) {
                        if (i1 != i2 || j1 != j2) {
                            // Avoid duplicate constraints
                            if ((i1 * 9 + j1) < (i2 * 9 + j2)) {
                                Clause* clause = create_clause(2);
                                int var1 = get_variable_index(i1, j1, k);
                                int var2 = get_variable_index(i2, j2, k);
                                add_literal(clause, -var1);
                                add_literal(clause, -var2);
                                
                                formula->clauses[formula->num_clauses] = *clause;
                                formula->num_clauses++;
                                safe_free(clause);
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Lower window: (5,5) to (7,7)
    for (int k = 1; k <= 9; k++) {
        for (int i1 = 5; i1 <= 7; i1++) {
            for (int j1 = 5; j1 <= 7; j1++) {
                for (int i2 = 5; i2 <= 7; i2++) {
                    for (int j2 = 5; j2 <= 7; j2++) {
                        if (i1 != i2 || j1 != j2) {
                            // Avoid duplicate constraints
                            if ((i1 * 9 + j1) < (i2 * 9 + j2)) {
                                Clause* clause = create_clause(2);
                                int var1 = get_variable_index(i1, j1, k);
                                int var2 = get_variable_index(i2, j2, k);
                                add_literal(clause, -var1);
                                add_literal(clause, -var2);
                                
                                formula->clauses[formula->num_clauses] = *clause;
                                formula->num_clauses++;
                                safe_free(clause);
                            }
                        }
                    }
                }
            }
        }
    }
}

// Add given digit constraints
void add_given_constraints(Formula* formula, const PercentSudokuGrid* puzzle) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (puzzle->grid[i][j] != PERCENT_SUDOKU_EMPTY) {
                int num = puzzle->grid[i][j];
                Clause* clause = create_clause(1);
                int var_index = get_variable_index(i, j, num);
                add_literal(clause, var_index);  // Force this variable to be true
                
                formula->clauses[formula->num_clauses] = *clause;
                formula->num_clauses++;
                safe_free(clause);
            }
        }
    }
}

// Add basic constraints (row, column, box)
void add_basic_constraints(Formula* formula) {
    add_cell_constraints(formula);
    add_row_constraints(formula);
    add_column_constraints(formula);
    add_box_constraints(formula);
}

// Convert Percent Sudoku puzzle to CNF formula
Formula* percent_sudoku_to_cnf(const PercentSudokuGrid* puzzle) {
    if (puzzle == NULL) {
        print_error("percent_sudoku_to_cnf", COMMON_ERROR_INVALID_PARAMETER, "Puzzle is NULL");
        return NULL;
    }
    
    // Calculate number of clauses
    int num_clauses = 0;
    
    // Cell constraints: 81 clauses (each cell at least one digit)
    num_clauses += 81;
    
    // Row constraints: 9 rows × 9 digits × C(9,2) = 9 × 9 × 36 = 2916 clauses
    num_clauses += 9 * 9 * 36;
    
    // Column constraints: 9 columns × 9 digits × C(9,2) = 9 × 9 × 36 = 2916 clauses
    num_clauses += 9 * 9 * 36;
    
    // Box constraints: 9 boxes × 9 digits × C(9,2) = 9 × 9 × 36 = 2916 clauses
    num_clauses += 9 * 9 * 36;
    
    // Anti-diagonal constraints: 9 digits × C(9,2) = 9 × 36 = 324 clauses
    num_clauses += 9 * 36;
    
    // Upper window constraints: 9 digits × C(9,2) = 9 × 36 = 324 clauses
    num_clauses += 9 * 36;
    
    // Lower window constraints: 9 digits × C(9,2) = 9 × 36 = 324 clauses
    num_clauses += 9 * 36;
    
    // Given digit constraints: up to 81 clauses
    int num_givens = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (puzzle->grid[i][j] != PERCENT_SUDOKU_EMPTY) {
                num_givens++;
            }
        }
    }
    num_clauses += num_givens;
    
    // Create formula: 729 variables (9×9×9), calculated number of clauses
    Formula* formula = create_formula(729, num_clauses);
    if (formula == NULL) {
        print_error("verify_percent_sudoku_cnf", COMMON_ERROR_INVALID_PARAMETER, "Formula is NULL");
        return 0;
    }
    
    // Add all constraints
    add_basic_constraints(formula);
    add_diagonal_constraints(formula);
    add_window_constraints(formula);
    add_given_constraints(formula, puzzle);
    
    return formula;
}

// Restore Percent Sudoku from CNF solution
PercentSudokuGrid* cnf_to_percent_sudoku(const Formula* formula, int* assignments, const PercentSudokuGrid* puzzle) {
    if (formula == NULL || assignments == NULL || puzzle == NULL) {
        print_error("cnf_to_percent_sudoku", COMMON_ERROR_INVALID_PARAMETER, "One or more parameters are NULL");
        return NULL;
    }
    
    PercentSudokuGrid* solution = create_percent_sudoku_grid();
    if (solution == NULL) {
        print_error("cnf_to_percent_sudoku", COMMON_ERROR_MEMORY_ALLOCATION, "Failed to create solution grid");
        return NULL;
    }
    
    // Copy puzzle to solution
    copy_percent_sudoku_grid(puzzle, solution);
    
    // Fill solution according to variable assignments
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (solution->grid[i][j] == PERCENT_SUDOKU_EMPTY) {
                for (int k = 1; k <= 9; k++) {
                    int var_index = get_variable_index(i, j, k);
                    if (var_index > 0 && var_index <= formula->num_vars) {
                        if (assignments[var_index - 1] == TRUE) {  // Variable index starts from 1, array from 0
                            solution->grid[i][j] = k;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    return solution;
}

// Verify Percent Sudoku CNF formula
int verify_percent_sudoku_cnf(const Formula* formula, const PercentSudokuGrid* puzzle) {
    if (formula == NULL || puzzle == NULL) {
        return 0;
    }
    
    // Check variable count
    if (formula->num_vars != 729) {
        printf("Error: Expected 729 variables, got %d\n", formula->num_vars);
        return 0;
    }
    
    // Check if clause count is reasonable
    if (formula->num_clauses < 1000 || formula->num_clauses > 10000) {
        printf("Warning: Unusual number of clauses: %d\n", formula->num_clauses);
    }
    
    // Check given digit constraints
    int num_givens = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (puzzle->grid[i][j] != PERCENT_SUDOKU_EMPTY) {
                num_givens++;
            }
        }
    }
    
    printf("CNF verification result:\n");
    printf("  Number of variables: %d\n", formula->num_vars);
    printf("  Number of clauses: %d\n", formula->num_clauses);
    printf("  Number of givens: %d\n", num_givens);
    
    return 1;
}

// Print CNF statistics
void print_cnf_statistics(const Formula* formula) {
    if (formula == NULL) {
        printf("Formula is NULL\n");
        return;
    }
    
    printf("=== Percent Sudoku CNF Statistics ===\n");
    printf("Number of variables: %d\n", formula->num_vars);
    printf("Number of clauses: %d\n", formula->num_clauses);
    
    // Calculate average clause length
    int total_literals = 0;
    for (int i = 0; i < formula->num_clauses; i++) {
        total_literals += formula->clauses[i].length;
    }
    double avg_length = (double)total_literals / formula->num_clauses;
    printf("Average clause length: %.2f\n", avg_length);
    
    // Count clauses of different lengths
    int length_count[20] = {0};  // Assume clause length does not exceed 20
    for (int i = 0; i < formula->num_clauses; i++) {
        int length = formula->clauses[i].length;
        if (length < 20) {
            length_count[length]++;
        }
    }
    
    printf("Clause length distribution:\n");
    for (int i = 1; i < 20; i++) {
        if (length_count[i] > 0) {
            printf("  Length %d: %d clauses\n", i, length_count[i]);
        }
    }
    printf("=============================\n");
}
