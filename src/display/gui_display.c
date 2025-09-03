#include "gui_display.h"
#include "../percent_sudoku/percent_sudoku_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 全局变量
static GuiDisplay* g_gui = NULL;

// 创建GUI显示对象
GuiDisplay* create_gui_display(void) {
    GuiDisplay* gui = (GuiDisplay*)safe_malloc(sizeof(GuiDisplay));
    memset(gui, 0, sizeof(GuiDisplay));
    
    gui->selected_row = -1;
    gui->selected_col = -1;
    gui->difficulty = 1;
    gui->show_solution = 0;
    gui->show_constraints = 1;
    gui->game_mode = 0;
    
    return gui;
}

// 释放GUI显示对象
void free_gui_display(GuiDisplay* gui) {
    if (gui == NULL) return;
    
    if (gui->game != NULL) {
        free_percent_sudoku_game(gui->game);
    }
    
    if (gui->working_grid != NULL) {
        free_percent_sudoku_grid(gui->working_grid);
    }
    
    destroy_resources(gui);
    safe_free(gui);
}

// 运行GUI显示
int run_gui_display(GuiDisplay* gui) {
    if (gui == NULL) return 0;
    
    g_gui = gui;
    
    // 注册窗口类
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "PercentSudokuWindow";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wc)) {
        show_error("Failed to register window class");
        return 0;
    }
    
    // 创建窗口
    gui->hwnd = CreateWindowEx(
        0,
        "PercentSudokuWindow",
        "Percent Sudoku Game",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL, NULL,
        GetModuleHandle(NULL),
        gui
    );
    
    if (gui->hwnd == NULL) {
        show_error("Failed to create window");
        return 0;
    }
    
    // 创建资源
    create_resources(gui);
    
    // 生成初始游戏
    generate_new_puzzle(gui);
    
    // 显示窗口
    ShowWindow(gui->hwnd, SW_SHOW);
    UpdateWindow(gui->hwnd);
    
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 1;
}

// 显示百分号数独GUI
void display_percent_sudoku_gui(PercentSudokuGame* game, int difficulty) {
    GuiDisplay* gui = create_gui_display();
    if (gui == NULL) {
        show_error("Failed to create GUI display");
        return;
    }
    
    gui->game = game;
    gui->difficulty = difficulty;
    
    // 创建工作网格
    gui->working_grid = create_percent_sudoku_grid();
    if (gui->working_grid != NULL && game != NULL) {
        copy_percent_sudoku_grid(&game->puzzle, gui->working_grid);
    }
    
    run_gui_display(gui);
    free_gui_display(gui);
}

// 窗口过程
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    GuiDisplay* gui = (GuiDisplay*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uMsg) {
        case WM_CREATE: {
            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            gui = (GuiDisplay*)cs->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)gui);
            gui->hwnd = hwnd;
            break;
        }
        
        case WM_PAINT:
            on_paint(gui);
            break;
            
        case WM_LBUTTONDOWN:
            on_lbutton_down(gui, LOWORD(lParam), HIWORD(lParam));
            break;
            
        case WM_KEYDOWN:
            on_key_down(gui, wParam);
            break;
            
        case WM_COMMAND:
            on_command(gui, wParam, lParam);
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}

// 绘制函数
void on_paint(GuiDisplay* gui) {
    if (gui == NULL) return;
    
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(gui->hwnd, &ps);
    
    RECT client_rect;
    GetClientRect(gui->hwnd, &client_rect);
    
    // 绘制网格
    draw_grid(hdc, gui, &client_rect);
    
    EndPaint(gui->hwnd, &ps);
}

void draw_grid(HDC hdc, GuiDisplay* gui, RECT* rect) {
    if (gui == NULL || gui->working_grid == NULL) return;
    
    // 计算网格大小和位置
    int grid_size = min(rect->right - rect->left - 40, rect->bottom - rect->top - 100);
    int cell_size = grid_size / 9;
    int start_x = (rect->right - rect->left - grid_size) / 2;
    int start_y = 50;
    
    // 绘制背景
    HBRUSH bg_brush = CreateSolidBrush(PERCENT_SUDOKU_COLOR_BACKGROUND);
    RECT bg_rect = {start_x - 5, start_y - 5, start_x + grid_size + 5, start_y + grid_size + 5};
    FillRect(hdc, &bg_rect, bg_brush);
    DeleteObject(bg_brush);
    
    // 绘制单元格
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            RECT cell_rect = {
                start_x + j * cell_size,
                start_y + i * cell_size,
                start_x + (j + 1) * cell_size,
                start_y + (i + 1) * cell_size
            };
            draw_cell(hdc, gui, i, j, &cell_rect);
        }
    }
    
    // 绘制网格线
    SelectObject(hdc, gui->grid_pen);
    
    // 绘制细线
    for (int i = 0; i <= 9; i++) {
        int x = start_x + i * cell_size;
        int y = start_y + i * cell_size;
        
        // 垂直线
        MoveToEx(hdc, x, start_y, NULL);
        LineTo(hdc, x, start_y + grid_size);
        
        // 水平线
        MoveToEx(hdc, start_x, y, NULL);
        LineTo(hdc, start_x + grid_size, y);
    }
    
    // 绘制粗线（3x3宫格）
    SelectObject(hdc, gui->thick_pen);
    for (int i = 0; i <= 3; i++) {
        int x = start_x + i * cell_size * 3;
        int y = start_y + i * cell_size * 3;
        
        // 垂直线
        MoveToEx(hdc, x, start_y, NULL);
        LineTo(hdc, x, start_y + grid_size);
        
        // 水平线
        MoveToEx(hdc, start_x, y, NULL);
        LineTo(hdc, start_x + grid_size, y);
    }
    
    // 绘制约束区域
    if (gui->show_constraints) {
        draw_constraints(hdc, gui, rect);
    }
}

void draw_cell(HDC hdc, GuiDisplay* gui, int row, int col, RECT* cell_rect) {
    if (gui == NULL || gui->working_grid == NULL) return;
    
    // 选择背景色
    HBRUSH bg_brush = NULL;
    COLORREF text_color = RGB(0, 0, 0);
    HFONT font_to_use = gui->font;
    int is_bold = 0;
    
    // 检查是否是选中单元格
    if (row == gui->selected_row && col == gui->selected_col) {
        bg_brush = gui->selected_brush;
    }
    // 检查是否在约束区域内
    else if (gui->show_constraints && is_in_constraint_area(row, col)) {
        bg_brush = gui->constraint_brush;  // 黄色背景
        // 在百分号区域内，根据是否预先填充选择字体样式
        if (gui->working_grid->is_given[row][col]) {
            text_color = PERCENT_SUDOKU_COLOR_GIVEN_TEXT;  // 深色字体
            font_to_use = gui->font_bold;  // 粗体字体
            is_bold = 1;
        } else {
            text_color = PERCENT_SUDOKU_COLOR_USER_TEXT;  // 浅色字体
        }
    }
    // 其他区域（包括预先填充的格子）
    else {
        bg_brush = gui->solved_brush;  // 白色背景
        // 根据是否预先填充选择字体样式
        if (gui->working_grid->is_given[row][col]) {
            text_color = PERCENT_SUDOKU_COLOR_GIVEN_TEXT;  // 深色字体
            font_to_use = gui->font_bold;  // 粗体字体
            is_bold = 1;
        } else {
            text_color = PERCENT_SUDOKU_COLOR_USER_TEXT;  // 浅色字体
        }
    }
    
    // 填充背景
    FillRect(hdc, cell_rect, bg_brush);
    
    // 绘制数字
    int number = gui->working_grid->grid[row][col];
    if (number != PERCENT_SUDOKU_EMPTY) {
        draw_number(hdc, number, cell_rect, font_to_use, text_color, is_bold);
    }
}

void draw_number(HDC hdc, int number, RECT* rect, HFONT font, COLORREF color, int is_bold) {
    (void)is_bold; // 避免未使用参数警告，粗体效果通过传入不同字体实现
    char num_str[2];
    sprintf(num_str, "%d", number);
    
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, font);
    
    DrawTextA(hdc, num_str, -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void draw_constraints(HDC hdc, GuiDisplay* gui, RECT* rect) {
    if (gui == NULL) return;
    
    // 绘制简洁的约束说明
    RECT text_rect = {10, rect->bottom - 30, rect->right - 10, rect->bottom - 10};
    
    SetTextColor(hdc, RGB(100, 100, 100));
    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, gui->small_font);
    
    const char* constraint_text = "Percent Sudoku: Percent Symbol Constraints";
    DrawTextA(hdc, constraint_text, -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// 事件处理
void on_lbutton_down(GuiDisplay* gui, int x, int y) {
    if (gui == NULL || gui->working_grid == NULL) return;
    
    int row, col;
    if (get_cell_from_point(gui, x, y, &row, &col)) {
        select_cell(gui, row, col);
    }
}

void on_key_down(GuiDisplay* gui, WPARAM wParam) {
    if (gui == NULL || gui->selected_row == -1 || gui->selected_col == -1) return;
    
    switch (wParam) {
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
            set_cell_value(gui, gui->selected_row, gui->selected_col, wParam - '0');
            break;
            
        case VK_DELETE:
        case VK_BACK:
        case '0':
            clear_cell(gui, gui->selected_row, gui->selected_col);
            break;
            
        case VK_UP:
            if (gui->selected_row > 0) select_cell(gui, gui->selected_row - 1, gui->selected_col);
            break;
        case VK_DOWN:
            if (gui->selected_row < 8) select_cell(gui, gui->selected_row + 1, gui->selected_col);
            break;
        case VK_LEFT:
            if (gui->selected_col > 0) select_cell(gui, gui->selected_row, gui->selected_col - 1);
            break;
        case VK_RIGHT:
            if (gui->selected_col < 8) select_cell(gui, gui->selected_row, gui->selected_col + 1);
            break;
    }
}

void on_command(GuiDisplay* gui, WPARAM wParam, LPARAM lParam) {
    (void)lParam; // 避免未使用参数警告
    switch (LOWORD(wParam)) {
        case ID_BUTTON_SOLVE:
            solve_puzzle(gui);
            break;
        case ID_BUTTON_NEW:
            generate_new_puzzle(gui);
            break;
        case ID_BUTTON_CHECK:
            check_solution(gui);
            break;
        case ID_BUTTON_CLEAR:
            // 清除所有非给定数字
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    if (!gui->working_grid->is_given[i][j]) {
                        clear_cell(gui, i, j);
                    }
                }
            }
            InvalidateRect(gui->hwnd, NULL, TRUE);
            break;
        case ID_BUTTON_HINT:
            show_hint(gui);
            break;
    }
}

// 游戏逻辑
void select_cell(GuiDisplay* gui, int row, int col) {
    if (gui == NULL) return;
    
    gui->selected_row = row;
    gui->selected_col = col;
    InvalidateRect(gui->hwnd, NULL, TRUE);
}

void set_cell_value(GuiDisplay* gui, int row, int col, int value) {
    if (gui == NULL || gui->working_grid == NULL) return;
    
    // 不能修改给定数字
    if (gui->working_grid->is_given[row][col]) return;
    
    gui->working_grid->grid[row][col] = value;
    InvalidateRect(gui->hwnd, NULL, TRUE);
}

void clear_cell(GuiDisplay* gui, int row, int col) {
    if (gui == NULL || gui->working_grid == NULL) return;
    
    // 不能清除给定数字
    if (gui->working_grid->is_given[row][col]) return;
    
    gui->working_grid->grid[row][col] = PERCENT_SUDOKU_EMPTY;
    InvalidateRect(gui->hwnd, NULL, TRUE);
}

int is_valid_move(GuiDisplay* gui, int row, int col, int value) {
    if (gui == NULL || gui->working_grid == NULL) return 0;
    
    // 临时设置值
    int original = gui->working_grid->grid[row][col];
    gui->working_grid->grid[row][col] = value;
    
    // 检查是否有效
    int valid = is_valid_percent_sudoku(gui->working_grid);
    
    // 恢复原值
    gui->working_grid->grid[row][col] = original;
    
    return valid;
}

void check_solution(GuiDisplay* gui) {
    if (gui == NULL || gui->working_grid == NULL) return;
    
    if (is_complete_percent_sudoku(gui->working_grid) && is_valid_percent_sudoku(gui->working_grid)) {
        show_message("Congratulations!", "You solved the puzzle correctly!");
    } else {
        show_message("Not Complete", "The puzzle is not solved yet or contains errors.");
    }
}

void solve_puzzle(GuiDisplay* gui) {
    if (gui == NULL || gui->game == NULL) return;
    
    // 复制解答到工作网格
    copy_percent_sudoku_grid(&gui->game->solution, gui->working_grid);
    InvalidateRect(gui->hwnd, NULL, TRUE);
    
    show_message("Solved", "The puzzle has been solved!");
}

void generate_new_puzzle(GuiDisplay* gui) {
    if (gui == NULL) return;
    
    // 释放旧游戏
    if (gui->game != NULL) {
        free_percent_sudoku_game(gui->game);
    }
    
    // 生成新游戏
    gui->game = generate_percent_sudoku_puzzle(gui->difficulty);
    if (gui->game != NULL) {
        copy_percent_sudoku_grid(&gui->game->puzzle, gui->working_grid);
        gui->selected_row = -1;
        gui->selected_col = -1;
        InvalidateRect(gui->hwnd, NULL, TRUE);
    }
}

void show_hint(GuiDisplay* gui) {
    if (gui == NULL || gui->game == NULL || gui->working_grid == NULL) return;
    
    // 找到第一个空单元格并填入解答
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (gui->working_grid->grid[i][j] == PERCENT_SUDOKU_EMPTY) {
                gui->working_grid->grid[i][j] = gui->game->solution.grid[i][j];
                InvalidateRect(gui->hwnd, NULL, TRUE);
                return;
            }
        }
    }
    
    show_message("No Hint", "All cells are filled!");
}

// 工具函数
void get_cell_rect(GuiDisplay* gui, int row, int col, RECT* rect) {
    (void)gui; (void)row; (void)col; (void)rect; // 避免未使用参数警告
    // 实现获取单元格矩形区域的逻辑
}

int get_cell_from_point(GuiDisplay* gui, int x, int y, int* row, int* col) {
    if (gui == NULL) return 0;
    
    RECT client_rect;
    GetClientRect(gui->hwnd, &client_rect);
    
    int grid_size = min(client_rect.right - client_rect.left - 40, client_rect.bottom - client_rect.top - 100);
    int cell_size = grid_size / 9;
    int start_x = (client_rect.right - client_rect.left - grid_size) / 2;
    int start_y = 50;
    
    if (x >= start_x && x < start_x + grid_size && y >= start_y && y < start_y + grid_size) {
        *col = (x - start_x) / cell_size;
        *row = (y - start_y) / cell_size;
        return 1;
    }
    
    return 0;
}

int is_in_constraint_area(int row, int col) {
    // 检查是否在撇对角线上
    if (row + col == 8) return 1;
    
    // 检查是否在上方窗口
    if (row >= 1 && row <= 3 && col >= 1 && col <= 3) return 1;
    
    // 检查是否在下方窗口
    if (row >= 5 && row <= 7 && col >= 5 && col <= 7) return 1;
    
    return 0;
}

void show_message(const char* title, const char* message) {
    MessageBoxA(NULL, message, title, MB_OK | MB_ICONINFORMATION);
}

void show_error(const char* message) {
    MessageBoxA(NULL, message, "Error", MB_OK | MB_ICONERROR);
}

// 资源管理
void create_resources(GuiDisplay* gui) {
    if (gui == NULL) return;
    
    // 创建字体
    gui->font = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    
    gui->font_bold = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    
    gui->small_font = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    
    // 创建画刷
    gui->given_brush = CreateSolidBrush(PERCENT_SUDOKU_COLOR_GIVEN_BG);
    gui->solved_brush = CreateSolidBrush(PERCENT_SUDOKU_COLOR_SOLVED_BG);
    gui->error_brush = CreateSolidBrush(PERCENT_SUDOKU_COLOR_ERROR_BG);
    gui->selected_brush = CreateSolidBrush(PERCENT_SUDOKU_COLOR_SELECTED_BG);
    gui->constraint_brush = CreateSolidBrush(PERCENT_SUDOKU_COLOR_CONSTRAINT_BG);
    
    // 创建画笔
    gui->grid_pen = CreatePen(PS_SOLID, 1, PERCENT_SUDOKU_COLOR_GRID_LINE);
    gui->thick_pen = CreatePen(PS_SOLID, 2, PERCENT_SUDOKU_COLOR_GRID_LINE);
}

void destroy_resources(GuiDisplay* gui) {
    if (gui == NULL) return;
    
    if (gui->font) DeleteObject(gui->font);
    if (gui->font_bold) DeleteObject(gui->font_bold);
    if (gui->small_font) DeleteObject(gui->small_font);
    if (gui->given_brush) DeleteObject(gui->given_brush);
    if (gui->solved_brush) DeleteObject(gui->solved_brush);
    if (gui->error_brush) DeleteObject(gui->error_brush);
    if (gui->selected_brush) DeleteObject(gui->selected_brush);
    if (gui->constraint_brush) DeleteObject(gui->constraint_brush);
    if (gui->grid_pen) DeleteObject(gui->grid_pen);
    if (gui->thick_pen) DeleteObject(gui->thick_pen);
}
