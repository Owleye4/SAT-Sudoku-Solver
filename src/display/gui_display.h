#ifndef GUI_DISPLAY_H
#define GUI_DISPLAY_H

#include <windows.h>
#include "../common/common.h"
#include "../percent_sudoku/percent_sudoku_core.h"

// 窗口控制ID
#define ID_GRID_BASE 1000
#define ID_BUTTON_SOLVE 2000
#define ID_BUTTON_NEW 2001
#define ID_BUTTON_CHECK 2002
#define ID_BUTTON_CLEAR 2003
#define ID_BUTTON_HINT 2004
#define ID_DIFFICULTY_COMBO 2005
#define ID_TIMER 2006

// 颜色定义
#define PERCENT_SUDOKU_COLOR_BACKGROUND RGB(240, 240, 240)
#define PERCENT_SUDOKU_COLOR_GRID_LINE RGB(0, 0, 0)
#define PERCENT_SUDOKU_COLOR_GIVEN_BG RGB(220, 220, 220)
#define PERCENT_SUDOKU_COLOR_SOLVED_BG RGB(255, 255, 255)
#define PERCENT_SUDOKU_COLOR_ERROR_BG RGB(255, 200, 200)
#define PERCENT_SUDOKU_COLOR_SELECTED_BG RGB(200, 200, 255)
#define PERCENT_SUDOKU_COLOR_CONSTRAINT_BG RGB(255, 255, 180)  // 浅黄色背景
#define PERCENT_SUDOKU_COLOR_USER_TEXT RGB(150, 150, 150)    // 用户填充的浅色字体
#define PERCENT_SUDOKU_COLOR_GIVEN_TEXT RGB(0, 0, 0)         // 预先填充的深色字体

// 结构体定义
typedef struct {
    HWND hwnd;
    PercentSudokuGame* game;
    PercentSudokuGrid* working_grid;
    int selected_row;
    int selected_col;
    int difficulty;
    int show_solution;
    int show_constraints;
    int game_mode; // 0: play mode, 1: solve mode
    HFONT font;
    HFONT font_bold;  // 粗体字体用于预先填充的数字
    HFONT small_font;
    HBRUSH given_brush;
    HBRUSH solved_brush;
    HBRUSH error_brush;
    HBRUSH selected_brush;
    HBRUSH constraint_brush;
    HPEN grid_pen;
    HPEN thick_pen;
} GuiDisplay;

// 函数声明
GuiDisplay* create_gui_display(void);
void free_gui_display(GuiDisplay* gui);
int run_gui_display(GuiDisplay* gui);
void display_percent_sudoku_gui(PercentSudokuGame* game, int difficulty);

// 窗口过程
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 绘制函数
void draw_grid(HDC hdc, GuiDisplay* gui, RECT* rect);
void draw_cell(HDC hdc, GuiDisplay* gui, int row, int col, RECT* cell_rect);
void draw_number(HDC hdc, int number, RECT* rect, HFONT font, COLORREF color, int is_bold);
void draw_constraints(HDC hdc, GuiDisplay* gui, RECT* rect);

// 事件处理
void on_paint(GuiDisplay* gui);
void on_lbutton_down(GuiDisplay* gui, int x, int y);
void on_key_down(GuiDisplay* gui, WPARAM wParam);
void on_command(GuiDisplay* gui, WPARAM wParam, LPARAM lParam);

// 游戏逻辑
void select_cell(GuiDisplay* gui, int row, int col);
void set_cell_value(GuiDisplay* gui, int row, int col, int value);
void clear_cell(GuiDisplay* gui, int row, int col);
int is_valid_move(GuiDisplay* gui, int row, int col, int value);
void check_solution(GuiDisplay* gui);
void solve_puzzle(GuiDisplay* gui);
void generate_new_puzzle(GuiDisplay* gui);
void show_hint(GuiDisplay* gui);

// 工具函数
void get_cell_rect(GuiDisplay* gui, int row, int col, RECT* rect);
int get_cell_from_point(GuiDisplay* gui, int x, int y, int* row, int* col);
int is_in_constraint_area(int row, int col);
void show_message(const char* title, const char* message);
void show_error(const char* message);

// 资源管理
void create_resources(GuiDisplay* gui);
void destroy_resources(GuiDisplay* gui);

#endif // GUI_DISPLAY_H
