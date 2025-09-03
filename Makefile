# 编译器设置
CC := gcc
CFLAGS := -Wall -Wextra -g

# 源文件
SOURCES := main.c \
           src/common/common.c \
           src/core/core.c \
           src/parser/parser.c \
           src/solver/solver.c \
           src/percent_sudoku/percent_sudoku_core.c \
           src/percent_sudoku/percent_sudoku_cnf.c \
           src/percent_sudoku/percent_sudoku_solver.c \
           src/display/gui_display.c

# 目标文件
TARGET := main

# 默认目标
all: $(TARGET)

# 编译主程序
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) -lgdi32 -luser32 -lkernel32

# 清理
clean:
	rm -f $(TARGET)

.PHONY: all clean