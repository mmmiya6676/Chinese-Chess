/*
 * MouseInput.cpp — 鼠标点击走棋（每次调用处理一步棋或一个命令）
 *
 * 流程：
 *   第一阶段 — 左键点击己方棋子 → 选中，高亮可走位置
 *   第二阶段 — 左键点击目标位置 → 合法就走棋并返回，不合法就取消选中
 *   右键/K键 → 切回键盘模式
 *
 * 控制台坐标映射：
 *   棋盘在缓冲区的位置：棋盘行 i → Y = 2 + i，棋盘列 j → X = 3 + j*3
 *   鼠标坐标是窗口坐标，需加上缓冲区滚动偏移量才是真实缓冲区坐标
 */

#include "../include/MouseInput.h"
#include "../include/SaveManager.h"
#include "../include/Timer.h"
#include <iostream>
#include <sstream>
#include <windows.h>
#include <chrono>
#include <vector>
using namespace std;


// 控制台缓冲区顶部被滚动了多少行（窗口第一行 = 缓冲区第几行）
static int g_scrollY = 0;

// ---- 坐标转换 ----

static bool screenToBoard(int mouseX, int mouseY, int& row, int& col) {
    // 鼠标坐标 + 滚动偏移 = 缓冲区坐标
    int bufX = mouseX;
    int bufY = mouseY + g_scrollY;
    if (bufX < 3 || bufY < 2) return false;
    col = (bufX - 3) / 3;
    row = bufY - 2;
    return (row >= 0 && row < 10 && col >= 0 && col < 9);
}

// ---- 绘制带高亮的棋盘 ----

// 颜色常量（Windows 控制台属性，组合方式：背景色 | 前景色）
// 高4位=背景, 低4位=前景, INTENSITY=加亮
static const WORD CLR_DEFAULT  = 7;    // 白字黑底
static const WORD CLR_RED_PIECE = FOREGROUND_RED | FOREGROUND_INTENSITY;            // 红方棋子：亮红字
static const WORD CLR_SELECTED = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE; // 选中棋子：白底
static const WORD CLR_VALID_EMPTY = BACKGROUND_GREEN | BACKGROUND_INTENSITY;          // 可走空位：亮绿底
static const WORD CLR_VALID_CAPTURE = BACKGROUND_RED | BACKGROUND_INTENSITY;          // 可吃子位：亮红底

static void drawScreen(HANDLE hOut, Game& game,
                       const Position<int>& selected,
                       const vector<Position<int>>& validMoves,
                       int remaining) {
    COORD home = {0, 0};
    SetConsoleCursorPosition(hOut, home);

    const Board& board = game.getBoard();
    SetConsoleTextAttribute(hOut, CLR_DEFAULT);

    // 顶部
    cout << "  ================" << endl;
    cout << "  ";
    for (int j = 0; j < 9; ++j) cout << " " << j << " ";
    cout << endl;

    // 棋盘行
    for (int i = 0; i < 10; ++i) {
        // 每行开头复位颜色，防止上一行末尾棋子颜色漏到行号
        SetConsoleTextAttribute(hOut, CLR_DEFAULT);
        cout << " " << i << " ";
        for (int j = 0; j < 9; ++j) {
            ChessPiece* p = board.getPieceAt(Position<int>(i, j));
            WORD attr = CLR_DEFAULT;

            if (selected.getX() == i && selected.getY() == j) {
                attr = CLR_SELECTED;
                if (p && p->getColor() == Color::RED)
                    attr |= FOREGROUND_RED;
            } else {
                bool isTarget = false;
                for (auto& m : validMoves)
                    if (m.getX() == i && m.getY() == j) { isTarget = true; break; }
                if (isTarget) {
                    attr = p ? CLR_VALID_CAPTURE : CLR_VALID_EMPTY;
                } else if (p) {
                    attr = (p->getColor() == Color::RED) ? CLR_RED_PIECE : CLR_DEFAULT;
                }
            }

            SetConsoleTextAttribute(hOut, attr);
            cout << (p ? " " + p->getSymbol() : " . ");
        }
        // 行尾复位，防止漏到下一行的行号
        SetConsoleTextAttribute(hOut, CLR_DEFAULT);
        cout << endl;
    }

    // 底部 + 状态
    SetConsoleTextAttribute(hOut, CLR_DEFAULT);
    cout << "  ================" << endl;
    cout << "当前执棋方: " << (game.getCurrentPlayer() == Color::RED ? "红" : "黑") << endl;
    cout << "存档: " << game.getSaveFilename() << endl;
    cout << "剩余 " << remaining << " 秒    " << endl;

    if (selected.getX() >= 0)
        cout << "已选中，请点目标 | ESC=取消 | 右键/K=键盘模式" << endl;
    else
        cout << "左键选子 | 右键/K=键盘 | U=悔棋 S=保存 Q=退出" << endl;

    // 用空行填满剩余屏幕行，覆盖之前残留的旧文字
    for (int k = 0; k < 6; ++k)
        cout << "                                                                               " << endl;
}

InputResult mouseInputLoop(Game& game) {
    HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // ---- 获取控制台滚动偏移量 ----
    // 鼠标事件给出的坐标是窗口坐标，而棋盘画在缓冲区里。
    // 如果用户滚动了控制台，窗口显示的不是缓冲区顶部，
    // 需要把偏移量加上才能正确对应棋盘格子。
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hOut, &csbi);
        g_scrollY = csbi.srWindow.Top;  // 窗口顶部 = 缓冲区第几行
    }

    DWORD oldMode;
    GetConsoleMode(hIn, &oldMode);
    DWORD newMode = oldMode;
    newMode |=  ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    newMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_QUICK_EDIT_MODE);
    SetConsoleMode(hIn, newMode);
    FlushConsoleInputBuffer(hIn);

    Position<int> selected(-1, -1);
    vector<Position<int>> validMoves;
    auto startTime = chrono::steady_clock::now();
    bool needFullRedraw = true;
    int lastRemaining = -1;

    // 倒计时行在缓冲区中的 Y 坐标（由 drawScreen 确定：顶部2行 + 10行棋盘 + 底部1行 + 状态2行 = 15）
    const SHORT TIMER_LINE_Y = 15;

    while (true) {
        // ---- 计时 ----
        auto now = chrono::steady_clock::now();
        int elapsed = chrono::duration_cast<chrono::seconds>(now - startTime).count();
        int remaining = MOVE_TIME_LIMIT - elapsed;
        if (remaining <= 0) {
            system("cls");
            cout << game << endl;
            cout << "========================================" << endl;
            cout << "  " << (game.getCurrentPlayer() == Color::RED ? "红" : "黑")
                 << "方超时! 胜利者: "
                 << (game.getCurrentPlayer() == Color::RED ? "黑" : "红") << "方" << endl;
            cout << "========================================" << endl;
            system("pause");
            SetConsoleMode(hIn, oldMode);
            return InputResult::Move;
        }

        // ---- 全量重绘（仅在选子变化时） ----
        if (needFullRedraw) {
            COORD home = {0, 0};
            SetConsoleCursorPosition(hOut, home);
            drawScreen(hOut, game, selected, validMoves, remaining);
            needFullRedraw = false;
            lastRemaining = remaining;
        }
        // ---- 计时行更新（每秒刷一次，只改一行，不抢光标） ----
        else if (remaining != lastRemaining) {
            // 保存当前光标位置（鼠标可能正悬停在某个格子上）
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(hOut, &csbi);
            COORD savedPos = csbi.dwCursorPosition;

            // 跳到计时行，更新数字
            COORD timerPos = {0, TIMER_LINE_Y};
            SetConsoleCursorPosition(hOut, timerPos);
            SetConsoleTextAttribute(hOut, CLR_DEFAULT);
            cout << "\r剩余 " << remaining << " 秒    " << flush;

            // 恢复光标原位，不影响鼠标下一次点击
            SetConsoleCursorPosition(hOut, savedPos);

            lastRemaining = remaining;
        }

        // ---- 批量读取输入事件 ----
        DWORD numEvents;
        GetNumberOfConsoleInputEvents(hIn, &numEvents);
        if (numEvents > 0) {
            // 一次性读完所有积压的事件，避免鼠标移动事件堆积
            bool needTextInput = false;
            for (DWORD i = 0; i < numEvents; ++i) {
                INPUT_RECORD ir;
                DWORD read;
                ReadConsoleInput(hIn, &ir, 1, &read);

                if (ir.EventType == MOUSE_EVENT) {
                    auto& mouse = ir.Event.MouseEvent;

                    // 只处理按钮按下事件（dwEventFlags==0 表示普通按下，非移动/滚轮）
                    if (mouse.dwEventFlags != 0) continue;

                    if (mouse.dwButtonState == RIGHTMOST_BUTTON_PRESSED) {
                        needTextInput = true;
                    }

                    if (mouse.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
                        int row, col;
                        if (!screenToBoard(mouse.dwMousePosition.X,
                                           mouse.dwMousePosition.Y, row, col))
                            continue;

                        Position<int> clicked(row, col);
                        ChessPiece* piece = game.getPieceAt(clicked);

                        if (selected.getX() < 0) {
                            // 第一阶段：选子
                            if (piece && piece->getColor() == game.getCurrentPlayer()
                                     && piece->isAlive()) {
                                selected = clicked;
                                validMoves = piece->getValidMoves(game.getBoard());
                                needFullRedraw = true;
                            }
                        } else {
                            // 第二阶段：走棋
                            bool valid = false;
                            for (auto& m : validMoves)
                                if (m == clicked) { valid = true; break; }

                            if (valid) {
                                game.makeMove(selected, clicked);
                                SetConsoleMode(hIn, oldMode);
                                return InputResult::Move;  // 走完了
                            }

                            // 点另一己方棋子 → 换选
                            if (piece && piece->getColor() == game.getCurrentPlayer()
                                      && piece->isAlive()) {
                                selected = clicked;
                                validMoves = piece->getValidMoves(game.getBoard());
                            } else {
                                selected = Position<int>(-1, -1);
                                validMoves.clear();
                            }
                            needFullRedraw = true;
                        }
                    }
                }
                else if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
                    char c = ir.Event.KeyEvent.uChar.AsciiChar;
                    WORD vk = ir.Event.KeyEvent.wVirtualKeyCode;

                    if (c == 'k' || c == 'K') {
                        needTextInput = true;
                    }
                    if (vk == VK_ESCAPE) {
                        selected = Position<int>(-1, -1);
                        validMoves.clear();
                        needFullRedraw = true;
                        continue;
                    }
                    if (c == 'u' || c == 'U') {
                        game.undo();
                        selected = Position<int>(-1, -1);
                        validMoves.clear();
                        needFullRedraw = true;
                        continue;
                    }
                    if (c == 's' || c == 'S') {
                        ensureDir("saves");
                        game.saveGame(game.getSaveFilename());
                        needFullRedraw = true;
                        continue;
                    }
                    if (c == 'q' || c == 'Q') {
                        SetConsoleMode(hIn, oldMode);  // 临时切回普通模式读确认
                        cout << endl << "保存? (Y/N/其他): " << flush;
                        string cf; getline(cin, cf);
                        if (cf == "Y" || cf == "y") {
                            ensureDir("saves");
                            game.saveGame(game.getSaveFilename());
                        }
                        if (cf == "Y" || cf == "y" || cf == "N" || cf == "n") {
                            SetConsoleMode(hIn, oldMode);
                            return InputResult::Quit;
                        }
                        // 取消 → 恢复鼠标模式
                        DWORD restore = oldMode;
                        restore |=  ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
                        restore &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_QUICK_EDIT_MODE);
                        SetConsoleMode(hIn, restore);
                        needFullRedraw = true;
                        continue;
                    }
                }
            }

            // ---- 文字输入子模式（右键/K键触发） ----
            // 临时切回行输入模式：IME 输入法需要 LINE_INPUT 才能正常处理中文
            if (needTextInput) {
                SetConsoleMode(hIn, oldMode);  // 切到原始模式（含 LINE_INPUT，IME 可用）

                // 用普通 cout 输出提示（此时在行输入模式，cout 正常工作）
                COORD pp = {0, 20};
                SetConsoleCursorPosition(hOut, pp);
                cout << string(70, ' ') << flush;
                SetConsoleCursorPosition(hOut, {0, 17});
                cout << "--- 键盘输入模式 (ESC=取消，输空回车=返回) ---" << endl;
                cout << "坐标(x1 y1 x2 y2) | 记谱法(炮二平五) | 命令" << endl;
                cout << "U悔棋 R重做 S保存 L读档 F认输 P和棋 T排行 Q退出" << endl;
                // 重新计算实际剩余时间，避免因事件处理耗时导致计时偏差
                auto kbNow = chrono::steady_clock::now();
                int kbElapsed = chrono::duration_cast<chrono::seconds>(kbNow - startTime).count();
                int kbRemaining = MOVE_TIME_LIMIT - kbElapsed;
                if (kbRemaining <= 0) {
                    system("cls");
                    cout << game << endl;
                    cout << "========================================" << endl;
                    cout << "  " << (game.getCurrentPlayer() == Color::RED ? "红" : "黑")
                         << "方超时! 胜利者: "
                         << (game.getCurrentPlayer() == Color::RED ? "黑" : "红") << "方" << endl;
                    cout << "========================================" << endl;
                    system("pause");
                    SetConsoleMode(hIn, oldMode);
                    return InputResult::Move;
                }
                bool timeout = false;
                string kbInput = getInputWithTimer(kbRemaining, timeout);

                if (timeout) {
                    system("cls");
                    cout << game << endl;
                    cout << "========================================" << endl;
                    cout << "  " << (game.getCurrentPlayer() == Color::RED ? "红" : "黑")
                         << "方超时! 胜利者: "
                         << (game.getCurrentPlayer() == Color::RED ? "黑" : "红") << "方" << endl;
                    cout << "========================================" << endl;
                    system("pause");
                    SetConsoleMode(hIn, oldMode);
                    return InputResult::Move;
                }

                // 切回鼠标原始模式
                DWORD restore = oldMode;
                restore |=  ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
                restore &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_QUICK_EDIT_MODE);
                SetConsoleMode(hIn, restore);
                FlushConsoleInputBuffer(hIn);

                if (!kbInput.empty()) {
                    unsigned char fb = static_cast<unsigned char>(kbInput[0]);
                    if (fb >= 0x80) {
                        game.makeMove(kbInput);
                    } else if (kbInput[0] == 'u' || kbInput[0] == 'U') {
                        game.undo();
                    } else if (kbInput[0] == 'r' || kbInput[0] == 'R') {
                        game.redo();
                    } else if (kbInput[0] == 's' || kbInput[0] == 'S') {
                        ensureDir("saves");
                        game.saveGame(game.getSaveFilename());
                    } else if (kbInput[0] == 'l' || kbInput[0] == 'L') {
                        string fname = showLoadMenu();
                        if (!fname.empty() && game.loadGame(fname))
                            game.setSaveFilename(fname);
                    } else if (kbInput[0] == 't' || kbInput[0] == 'T') {
                        showLeaderboard();
                    } else if (kbInput[0] == 'f' || kbInput[0] == 'F') {
                        SetConsoleMode(hIn, oldMode);
                        COORD cp = {0, 22};
                        SetConsoleCursorPosition(hOut, cp);
                        cout << "确定认输? (Y/N): " << flush;
                        string cf; getline(cin, cf);
                        if (cf == "Y" || cf == "y") {
                            string fname = game.getSaveFilename();
                            string base = fname;
                            size_t sl = base.find('/');
                            if (sl != string::npos) base = base.substr(sl + 1);
                            size_t vp = base.find("_vs_"), pp = base.find("_play");
                            string rn = "红方", bn = "黑方", winner = "黑";
                            if (vp != string::npos && pp != string::npos) {
                                rn = getPlayerName(stoi(base.substr(0, vp)));
                                bn = getPlayerName(stoi(base.substr(vp + 4, pp - vp - 4)));
                            }
                            winner = (game.getCurrentPlayer() == Color::RED) ? "黑" : "红";
                            recordGameResult(rn, bn, winner);
                            SetConsoleMode(hIn, oldMode);
                            return InputResult::Quit;
                        }
                        SetConsoleMode(hIn, newMode);
                    } else if (kbInput[0] == 'p' || kbInput[0] == 'P') {
                        SetConsoleMode(hIn, oldMode);
                        COORD cp = {0, 22};
                        SetConsoleCursorPosition(hOut, cp);
                        cout << "确定和棋? (Y/N): " << flush;
                        string cf; getline(cin, cf);
                        if (cf == "Y" || cf == "y") {
                            // 从存档文件名提取玩家名记录平局
                            string fname = game.getSaveFilename();
                            string base = fname;
                            size_t sl = base.find('/');
                            if (sl != string::npos) base = base.substr(sl + 1);
                            size_t vp = base.find("_vs_"), pp = base.find("_play");
                            string rn = "红方", bn = "黑方";
                            if (vp != string::npos && pp != string::npos) {
                                rn = getPlayerName(stoi(base.substr(0, vp)));
                                bn = getPlayerName(stoi(base.substr(vp + 4, pp - vp - 4)));
                            }
                            recordGameDraw(rn, bn);
                            SetConsoleMode(hIn, oldMode);
                            return InputResult::Quit;
                        }
                        SetConsoleMode(hIn, newMode);
                    } else if (kbInput[0] == 'q' || kbInput[0] == 'Q') {
                        SetConsoleMode(hIn, oldMode);
                        cout << endl << "保存? (Y/N): " << flush;
                        string cf; getline(cin, cf);
                        if (cf == "Y" || cf == "y") {
                            ensureDir("saves");
                            game.saveGame(game.getSaveFilename());
                        }
                        if (cf == "Y" || cf == "y" || cf == "N" || cf == "n") {
                            SetConsoleMode(hIn, oldMode);
                            return InputResult::Quit;
                        }
                        // 恢复鼠标模式
                        DWORD restore = oldMode;
                        restore |=  ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
                        restore &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_QUICK_EDIT_MODE);
                        SetConsoleMode(hIn, restore);
                    } else {
                        istringstream iss(kbInput);
                        iss >> game;
                    }
                    selected = Position<int>(-1, -1);
                    validMoves.clear();
                    needFullRedraw = true;
                    startTime = chrono::steady_clock::now();
                }
            }
        } else {
            Sleep(50);
        }
    }
}
