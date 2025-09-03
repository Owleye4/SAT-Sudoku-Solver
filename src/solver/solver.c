#include "solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 检查子句是否满足（至少有一个文字为真）
int is_clause_satisfied(const Clause* clause, const int* assignments) {
    for (int i = 0; i < clause->length; i++) {
        int literal = clause->literals[i];
        int var = abs(literal);
        int value = assignments[var - 1]; // 变量索引从1开始，数组索引从0开始
        
        if (value == UNASSIGNED) {
            continue; // 未赋值的变量不影响子句的满足性
        }
        
        // 如果文字为正且变量为真，或文字为负且变量为假，则子句满足
        if ((literal > 0 && value == TRUE) || (literal < 0 && value == FALSE)) {
            return 1;
        }
    }
    return 0; // 没有文字为真
}

// 检查子句是否冲突（所有已赋值的文字都为假）
int is_clause_conflict(const Clause* clause, const int* assignments) {
    int has_unassigned = 0;
    
    for (int i = 0; i < clause->length; i++) {
        int literal = clause->literals[i];
        int var = abs(literal);
        int value = assignments[var - 1];
        
        if (value == UNASSIGNED) {
            has_unassigned = 1;
            continue;
        }
        
        // 如果文字为正且变量为真，或文字为负且变量为假，则子句满足
        if ((literal > 0 && value == TRUE) || (literal < 0 && value == FALSE)) {
            return 0; // 子句满足，不是冲突
        }
    }
    
    // 如果没有未赋值的变量且所有已赋值的文字都为假，则冲突
    return !has_unassigned;
}

// 检查子句是否为单子句（只有一个未赋值的文字，其他都为假）
int is_unit_clause(const Clause* clause, const int* assignments, int* unit_literal) {
    int unassigned_count = 0;
    int unassigned_literal = 0;
    
    for (int i = 0; i < clause->length; i++) {
        int literal = clause->literals[i];
        int var = abs(literal);
        int value = assignments[var - 1];
        
        if (value == UNASSIGNED) {
            unassigned_count++;
            unassigned_literal = literal;
        } else {
            // 如果已赋值的文字为真，则子句满足，不是单子句
            if ((literal > 0 && value == TRUE) || (literal < 0 && value == FALSE)) {
                return 0;
            }
        }
    }
    
    // 只有一个未赋值的文字，其他都为假
    if (unassigned_count == 1) {
        *unit_literal = unassigned_literal;
        return 1;
    }
    
    return 0;
}

// 单子句传播
int unit_propagation(Formula* formula, int* assignments) {
    int changed = 1;
    int iteration = 0;
    
    printf("Starting unit propagation...\n");
    
    while (changed) {
        changed = 0;
        iteration++;
        printf("  Propagation iteration %d\n", iteration);
        
        for (int i = 0; i < formula->num_clauses; i++) {
            const Clause* clause = &formula->clauses[i];
            
            // 跳过已满足的子句
            if (is_clause_satisfied(clause, assignments)) {
                continue;
            }
            
            // 检查冲突
            if (is_clause_conflict(clause, assignments)) {
                printf("  Conflict detected! Clause %d all literals are false\n", i);
                return UNSAT;
            }
            
            // 检查单子句
            int unit_literal;
            if (is_unit_clause(clause, assignments, &unit_literal)) {
                int var = abs(unit_literal);
                int value = (unit_literal > 0) ? TRUE : FALSE;
                
                printf("  Unit clause found! Clause %d: literal %d, variable %d assigned to %s\n", 
                       i, unit_literal, var, (value == TRUE) ? "TRUE" : "FALSE");
                
                assignments[var - 1] = value;
                changed = 1;
                
                // 检查是否所有子句都满足
                if (is_formula_satisfied(formula, assignments)) {
                    printf("  All clauses are satisfied!\n");
                    return SAT;
                }
            }
        }
        
        if (changed) {
            printf("  Assignment updated, continuing propagation...\n");
        }
    }
    
    printf("Unit propagation completed, %d iterations total\n", iteration);
    return -1; // 既不是SAT也不是UNSAT，需要继续分支
}

// 检查公式是否完全满足
int is_formula_satisfied(const Formula* formula, const int* assignments) {
    for (int i = 0; i < formula->num_clauses; i++) {
        if (!is_clause_satisfied(&formula->clauses[i], assignments)) {
            return 0;
        }
    }
    return 1;
}

// 选择分支变量（选择第一个未赋值的变量）
int select_branching_variable(const Formula* formula, const int* assignments) {
    for (int i = 0; i < formula->num_vars; i++) {
        if (assignments[i] == UNASSIGNED) {
            return i + 1; // 返回变量编号（从1开始）
        }
    }
    return -1; // 所有变量都已赋值
}

// 创建赋值数组的副本
int* copy_assignments(const Formula* formula, const int* assignments) {
    int* new_assignments = (int*)safe_malloc(formula->num_vars * sizeof(int));
    
    memcpy(new_assignments, assignments, formula->num_vars * sizeof(int));
    return new_assignments;
}

// 打印当前赋值状态
void print_assignments(const Formula* formula, const int* assignments) {
    printf("Current assignment status: ");
    for (int i = 0; i < formula->num_vars; i++) {
        if (assignments[i] == TRUE) {
            printf("%d=T ", i + 1);
        } else if (assignments[i] == FALSE) {
            printf("%d=F ", i + 1);
        } else {
            printf("%d=? ", i + 1);
        }
    }
    printf("\n");
}

// 主要的DPLL函数
int DPLL(Formula* formula, int* assignments) {
    static int recursion_depth = 0;
    recursion_depth++;
    
    printf("\n=== DPLL Recursive Call %d ===\n", recursion_depth);
    print_assignments(formula, assignments);
    
    // 步骤1: 单子句传播
    int propagation_result = unit_propagation(formula, assignments);
    if (propagation_result == SAT) {
        printf("=== Solution found! ===\n");
        recursion_depth--;
        return SAT;
    }
    if (propagation_result == UNSAT) {
        printf("=== Conflict detected, backtracking ===\n");
        recursion_depth--;
        return UNSAT;
    }
    
    // 步骤2: 选择分支变量
    int branch_var = select_branching_variable(formula, assignments);
    if (branch_var == -1) {
        printf("=== All variables assigned, checking satisfaction ===\n");
        if (is_formula_satisfied(formula, assignments)) {
            printf("=== Solution found! ===\n");
            recursion_depth--;
            return SAT;
        } else {
            printf("=== Not satisfied, backtracking ===\n");
            recursion_depth--;
            return UNSAT;
        }
    }
    
    printf("Selecting branching variable: %d\n", branch_var);
    
    // 步骤3: 尝试分支变量为真
    printf("Trying variable %d = TRUE\n", branch_var);
    int* new_assignments = copy_assignments(formula, assignments);
    new_assignments[branch_var - 1] = TRUE;
    
    int result = DPLL(formula, new_assignments);
    if (result == SAT) {
        // 复制解到原始赋值数组
        memcpy(assignments, new_assignments, formula->num_vars * sizeof(int));
        safe_free(new_assignments);
        printf("=== Variable %d = TRUE found solution! ===\n", branch_var);
        recursion_depth--;
        return SAT;
    }
    
    // 步骤4: 尝试分支变量为假
    printf("Variable %d = TRUE failed, trying variable %d = FALSE\n", branch_var, branch_var);
    new_assignments[branch_var - 1] = FALSE;
    
    result = DPLL(formula, new_assignments);
    if (result == SAT) {
        // 复制解到原始赋值数组
        memcpy(assignments, new_assignments, formula->num_vars * sizeof(int));
        safe_free(new_assignments);
        printf("=== Variable %d = FALSE found solution! ===\n", branch_var);
        recursion_depth--;
        return SAT;
    }
    
    // 两个分支都失败
    safe_free(new_assignments);
    printf("=== Both branches of variable %d failed, backtracking ===\n", branch_var);
    recursion_depth--;
    return UNSAT;
}
