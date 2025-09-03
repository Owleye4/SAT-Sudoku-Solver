# SAT Sudoku Solver

## 项目概述

这是一个基于SAT（布尔可满足性）求解技术的百分号数独求解器项目。项目实现了完整的DPLL算法用于SAT求解，并将百分号数独问题转换为CNF（合取范式）形式进行求解。同时提供了图形用户界面，支持交互式游戏体验。

## 项目特色

- **SAT求解器**：实现了完整的DPLL算法，包含单子句传播、分支选择等优化
- **百分号数独**：实现了特殊的数独变种，包含撇对角线约束和窗口约束
- **图形界面**：提供Windows GUI界面，支持交互式游戏
- **多种模式**：支持生成、求解、测试、演示等多种运行模式

## 开发环境

- **编译器**：GCC
- **平台**：Windows
- **依赖库**：Windows API (GDI32, User32, Kernel32)
- **构建工具**：Make

## 百分号数独规则

百分号数独是标准数独的变种，在标准数独规则基础上增加了两个特殊约束：

1. **撇对角线约束**：从左上到右下的对角线（位置编码：19, 28, 37, 46, 55, 64, 73, 82, 91）上每个数字1-9只能出现一次
2. **窗口约束**：
   - 上方窗口（位置编码：22, 23, 24, 32, 33, 34, 42, 43, 44）
   - 下方窗口（位置编码：66, 67, 68, 76, 77, 78, 86, 87, 88）
   - 每个窗口内每个数字1-9只能出现一次

这些约束区域在视觉上形成"%"符号的形状，因此称为百分号数独。

## 编译和运行

### 编译
```bash
make
```

### 运行

#### 1. CNF求解模式
```bash
# 基本用法
./main <cnf_file> [timeout_seconds]

# 示例
./main tests/cases/small/small_sat_1.cnf
./main tests/cases/medium/medium_sat_1.cnf 60
./main tests/cases/large/large_sat_1.cnf 300
```
#### 2. 百分号数独模式

**生成谜题**
```bash
./main --percent-sudoku generate [difficulty] [timeout]
# 示例
./main --percent-sudoku generate 3 60
```

**求解谜题**
```bash
./main --percent-sudoku solve <puzzle_file> [timeout]
# 示例
./main --percent-sudoku solve puzzle.percent_sudoku 30
```

**图形界面**
```bash
./main --percent-sudoku gui [difficulty]
# 示例
./main --percent-sudoku gui 2
```

**测试模式**
```bash
./main --percent-sudoku test
```

**演示模式**
```bash
./main --percent-sudoku demo
```

## 参数说明

- `cnf_file`: CNF文件路径
- `timeout_seconds`: 超时时间（秒），默认300秒
- `difficulty`: 百分号数独难度（1-5），默认3
- `puzzle_file`: 百分号数独文件路径

## 文件格式

### CNF文件格式
遵循DIMACS标准格式：
```
c 注释行
p cnf <变量数> <子句数>
<子句1>
<子句2>
...
```

### 百分号数独文件格式
```
# Percent Sudoku Game File
difficulty 3
num_givens 25

# Puzzle
0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0
...

# Solution
1 2 3 4 5 6 7 8 9
...
```
## 测试用例

项目包含三个级别的测试用例：
- **小型测试**：变量数<100，用于功能验证
- **中型测试**：变量数100-1000，用于性能测试
- **大型测试**：变量数>1000，用于压力测试


## 技术特点

### SAT求解器特性
- **DPLL算法**：完整的Davis-Putnam-Logemann-Loveland算法实现
- **单子句传播**：自动推导和赋值
- **分支选择**：智能变量选择策略
- **回溯机制**：完整的回溯和剪枝

### 百分号数独求解特性
- **CNF转换**：将约束转换为布尔公式
- **变量编码**：位置(i,j)的数字k对应变量(i*9+j)*9+k
- **约束生成**：自动生成所有约束条件
- **解还原**：从SAT解还原为数独解答

### 性能优化
- **内存管理**：安全的内存分配和释放
- **时间测量**：精确的性能统计
- **超时控制**：防止无限循环
- **错误处理**：完善的错误检测和报告
