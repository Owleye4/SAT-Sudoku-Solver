#include "core.h"

// 创建子句
Clause* create_clause(int initial_capacity) {
    Clause* clause = (Clause*)safe_malloc(sizeof(Clause));
    clause->literals = (int*)safe_malloc(initial_capacity * sizeof(int));
    
    clause->length = 0;
    clause->capacity = initial_capacity;
    
    return clause;
}

// 向子句添加文字
void add_literal(Clause* clause, int literal) {
    if (clause == NULL) {
        print_error("add_literal", COMMON_ERROR_INVALID_PARAMETER, "Clause pointer is NULL");
        return;
    }
    
    // 如果容量不够，重新分配内存
    if (clause->length >= clause->capacity) {
        int new_capacity = clause->capacity * 2;
        clause->literals = (int*)safe_realloc(clause->literals, new_capacity * sizeof(int));
        clause->capacity = new_capacity;
    }
    
    clause->literals[clause->length] = literal;
    clause->length++;
}

// 释放子句内存
void free_clause(Clause* clause) {
    if (clause != NULL) {
        if (clause->literals != NULL) {
            safe_free(clause->literals);
        }
        safe_free(clause);
    }
}

// 创建公式
Formula* create_formula(int num_vars, int num_clauses) {
    Formula* formula = (Formula*)safe_malloc(sizeof(Formula));
    formula->clauses = (Clause*)safe_malloc(num_clauses * sizeof(Clause));
    formula->var_states = (int*)safe_malloc(num_vars * sizeof(int));
    
    // 初始化var_states为0
    for (int i = 0; i < num_vars; i++) {
        formula->var_states[i] = 0;
    }
    
    formula->num_clauses = 0;
    formula->num_vars = num_vars;
    
    // 初始化所有子句
    for (int i = 0; i < num_clauses; i++) {
        formula->clauses[i].literals = NULL;
        formula->clauses[i].length = 0;
        formula->clauses[i].capacity = 0;
    }
    
    return formula;
}

// 释放公式内存
void free_formula(Formula* formula) {
    if (formula != NULL) {
        if (formula->clauses != NULL) {
            // 释放每个子句的文字数组
            for (int i = 0; i < formula->num_clauses; i++) {
                if (formula->clauses[i].literals != NULL) {
                    safe_free(formula->clauses[i].literals);
                }
            }
            safe_free(formula->clauses);
        }
        
        if (formula->var_states != NULL) {
            safe_free(formula->var_states);
        }
        
        safe_free(formula);
    }
}

// 打印公式结构，用于调试和验证
void print_formula(const Formula* formula) {
    if (formula == NULL) {
        printf("Formula is empty\n");
        return;
    }
    
    printf("=== Parsed CNF Formula ===\n");
    printf("Number of variables: %d\n", formula->num_vars);
    printf("Number of clauses: %d\n", formula->num_clauses);
    printf("\n");
    
    // 打印问题行（CNF格式）
    printf("p cnf %d %d\n", formula->num_vars, formula->num_clauses);
    printf("\n");
    
    // 打印所有子句
    for (int i = 0; i < formula->num_clauses; i++) {
        const Clause* clause = &formula->clauses[i];
        
        if (clause->literals == NULL || clause->length == 0) {
            printf("Clause %d: (empty)\n", i);
            continue;
        }
        
        printf("Clause %d: ", i);
        for (int j = 0; j < clause->length; j++) {
            printf("%d ", clause->literals[j]);
        }
        printf("0\n");
    }
    
    printf("\n=== Detailed Structure Information ===\n");
    for (int i = 0; i < formula->num_clauses; i++) {
        const Clause* clause = &formula->clauses[i];
        printf("Clause %d: length=%d, capacity=%d, literals=[", 
               i, clause->length, clause->capacity);
        
        if (clause->literals != NULL) {
            for (int j = 0; j < clause->length; j++) {
                printf("%d", clause->literals[j]);
                if (j < clause->length - 1) printf(" ");
            }
        }
        printf("]\n");
    }
    
    printf("\n=== Variable States ===\n");
    for (int i = 0; i < formula->num_vars; i++) {
        printf("Variable %d: %d\n", i + 1, formula->var_states[i]);
    }
    printf("========================\n\n");
}
