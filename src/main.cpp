/*
 * 中国象棋 - 主程序
 *
 * 功能：
 *  - 主菜单：新建游戏 / 加载存档 / 退出
 *  - 支持坐标输入 (x1 y1 x2 y2) 和中文记谱法 (炮二平五)
 *  - 每步限时 90 秒，实时倒计时显示
 *  - 存档管理（新建 / 加载 / 自动编号）由 SaveManager 模块负责
 *  - 计时功能由 Timer 模块负责
 */

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include "../include/Game.h"
#include "../include/Timer.h"
#include "../include/SaveManager.h"
using namespace std;

// ============================================================
//  游戏主循环
// ============================================================

/*
 * 一局游戏的完整循环：
 *  清屏 → 显示棋盘 → 显示状态 → 等待输入 → 执行操作 → 暂停 → 重复
 *
 * 输入判断逻辑：
 *  - 首字节 >= 0x80（非ASCII，即中文字符）→ 按记谱法解析
 *  - 首字节 < 0x80（ASCII）→ 可能是命令(U/S/L/R/Q)或坐标
 */
void playGame(Game& game, const string& redName, const string& blackName,
               int moveTimeLimit = MOVE_TIME_LIMIT) {
    while (!game.isGameOver()) {
        system("cls");  // 清屏，保持界面整洁
        cout << game << endl;  // 输出棋盘（调用 operator<<）

        // 显示当前状态和操作提示
        cout << "当前执棋方: " << (game.getCurrentPlayer() == Color::RED ? "红" : "黑") << endl;
        cout << "存档: " << game.getSaveFilename() << endl;
        cout << "每步限时: " << moveTimeLimit << " 秒" << endl;
        cout << "----------------------------------------" << endl;
        cout << "输入: 坐标(x1 y1 x2 y2) | 记谱法(炮二平五) | 命令" << endl;
        cout << "U悔棋  R重做  S保存  L读档  F认输  P和棋  T排行  Q退出" << endl;

        // 本回合限时倒计时（失败不走棋时时间不重置，继续倒数）
        auto turnStart = chrono::steady_clock::now();
        int turnRemaining = moveTimeLimit;
        bool moveSuccess = false;

        while (!moveSuccess) {
            // 带倒计时的输入（getline 支持 IME 中文输入法）
            bool timeout = false;
            string input = getInputWithTimer(turnRemaining, timeout);

        // 超时 → 当前走棋方判负
        if (timeout) {
            system("cls");
            cout << "========================================" << endl;
            cout << "  " << (game.getCurrentPlayer() == Color::RED ? "红" : "黑")
                 << "方超时! 胜利者: "
                 << (game.getCurrentPlayer() == Color::RED ? "黑" : "红") << "方" << endl;
            cout << "========================================" << endl;
            recordGameResult(redName, blackName,
                (game.getCurrentPlayer() == Color::RED) ? "黑" : "红");
            system("pause");
            return;
        }

            // 扣除本次输入消耗的时间
            auto now = chrono::steady_clock::now();
            int elapsed = chrono::duration_cast<chrono::seconds>(now - turnStart).count();
            turnRemaining = moveTimeLimit - elapsed;
            if (turnRemaining <= 0) turnRemaining = 1;

            if (cin.eof()) return;       // Ctrl+Z 退出
            if (input.empty()) continue;  // 空行跳过，时间继续走

        // 判断输入类型：看首字节
        // UTF-8 中，中文字符首字节 >= 0x80，ASCII 字符首字节 < 0x80
        unsigned char firstByte = static_cast<unsigned char>(input[0]);

        if (firstByte >= 0x80) {
            // ---- 中文记谱法走棋 ----
            if (game.makeMove(input)) {
                cout << "走棋成功!" << endl;
                moveSuccess = true;
            } else {
                cout << "无效走法!" << endl;
            }
            system("pause");
        }
        else if (input[0] == 'u' || input[0] == 'U') {
            // ---- 悔棋：撤销上一步 ----
            if (game.undo()) {
                cout << "悔棋成功!" << endl;
                moveSuccess = true;
            } else {
                cout << "无法悔棋!" << endl;
            }
            system("pause");
        }
        else if (input[0] == 'r' || input[0] == 'R') {
            // ---- 重做：恢复被悔的棋 ----
            if (game.redo()) {
                cout << "重做成功!" << endl;
                moveSuccess = true;
            } else {
                cout << "无法重做!" << endl;
            }
            system("pause");
        }
        else if (input[0] == 'f' || input[0] == 'F') {
            // ---- 认输：当前方认输，对方获胜 ----
            cout << "确定认输? 按 Y 确认，其他键取消: ";
            string confirm;
            getline(cin, confirm);
            if (confirm == "Y" || confirm == "y") {
                system("cls");
                cout << game << endl;
                cout << "========================================" << endl;
                cout << "  " << (game.getCurrentPlayer() == Color::RED ? "红" : "黑")
                     << "方认输! 胜利者: "
                     << (game.getCurrentPlayer() == Color::RED ? "黑" : "红") << "方" << endl;
                cout << "========================================" << endl;
                recordGameResult(redName, blackName,
                    (game.getCurrentPlayer() == Color::RED) ? "黑" : "红");
                system("pause");
                return;
            }
        }
        else if (input[0] == 'p' || input[0] == 'P') {
            // ---- 和棋：双方平手 ----
            cout << "确定和棋? 按 Y 确认，其他键取消: ";
            string confirm;
            getline(cin, confirm);
            if (confirm == "Y" || confirm == "y") {
                system("cls");
                cout << game << endl;
                cout << "========================================" << endl;
                cout << "  双方同意和棋 —— 平手!" << endl;
                cout << "========================================" << endl;
                recordGameDraw(redName, blackName);
                system("pause");
                return;
            }
        }
        else if (input[0] == 't' || input[0] == 'T') {
            // ---- 查看排行榜 ----
            showLeaderboard();
            system("pause");
        }
        else if (input[0] == 'q' || input[0] == 'Q') {
            // ---- 退出：询问是否保存 ----
            cout << "是否保存当前进度? (Y=保存并退出  N=不保存退出  其他键=取消): ";
            string confirm;
            getline(cin, confirm);
            if (confirm == "Y" || confirm == "y") {
                string fname = game.getSaveFilename();
                ensureDir("saves");
                game.saveGame(fname);
                cout << "已保存到: " << fname << endl;
                system("pause");
                return;
            }
            if (confirm == "N" || confirm == "n") return;
        }
        else if (input[0] == 's' || input[0] == 'S') {
            // ---- 保存：写入当前存档文件 ----
            string fname = game.getSaveFilename();
            ensureDir("saves");  // 确保 saves 目录存在
            if (game.saveGame(fname))
                cout << "保存成功! -> " << fname << endl;
            else
                cout << "保存失败!" << endl;
            system("pause");
        }
        else if (input[0] == 'l' || input[0] == 'L') {
            // ---- 读档：弹出存档列表，选择后加载 ----
            string fname = showLoadMenu();
            if (fname.empty()) continue;  // 用户取消
            if (game.loadGame(fname)) {
                game.setSaveFilename(fname);  // 更新存档路径为刚加载的文件
                cout << "读档成功!" << endl;
            } else {
                cout << "读档失败!" << endl;
            }
            system("pause");
        }
        else {
            // ---- 坐标输入：如 "7 1 7 4" ----
            istringstream iss(input);
            iss >> game;  // 调用 operator>>，内部调用 makeMove
            // 通过流状态判断走棋是否成功（operator>> 失败会设置 failbit）
            if (iss.fail()) {
                cout << "无效走法!" << endl;
            } else {
                cout << "走棋成功!" << endl;
                moveSuccess = true;
            }
            system("pause");
        }
        }
    }

    // 游戏结束（将死）—— 展示最终棋盘并记录排行榜
    system("cls");
    cout << game << endl;
    cout << "========================================" << endl;
    cout << "  游戏结束! 胜利者: "
         << (game.getWinner() == Color::RED ? "红" : "黑") << "方" << endl;
    cout << "========================================" << endl;
    recordGameResult(redName, blackName,
        (game.getWinner() == Color::RED) ? "红" : "黑");
    system("pause");
}

// ============================================================
//  程序入口 — 主菜单
// ============================================================

/*
 * 启动后显示主菜单，循环直到用户选"退出"：
 *  1. 新建游戏 → 输入双方姓名 → 自动生成存档路径 → 开始游戏
 *  2. 加载存档 → 列表选择 → 读取 → 继续游戏
 *  3. 退出
 */
int main() {
    system("chcp 65001 > nul");  // 设置控制台编码为 UTF-8，确保中文不乱码

    while (true) {
        system("cls");
        cout << "===== 中国象棋 =====" << endl;
        cout << "1. 新建游戏" << endl;
        cout << "2. 加载存档" << endl;
        cout << "3. 退出" << endl;
        cout << "请选择 (1-3): ";

        string choice;
        getline(cin, choice);

        if (choice == "1") {
            // ===== 新建游戏 =====
            system("cls");
            cout << "===== 新建游戏 =====" << endl;

            // 选择限时等级
            int timeLimit = MOVE_TIME_LIMIT;
            while (true) {
                cout << "请选择限时等级:" << endl;
                cout << "  1. 快棋 (10秒)" << endl;
                cout << "  2. 正常 (90秒)" << endl;
                cout << "  3. 慢棋 (180秒)" << endl;
                cout << "请选择 (1-3): ";
                string ts;
                getline(cin, ts);
                if (ts == "1")      { timeLimit = 10;  break; }
                else if (ts == "2") { timeLimit = 90;  break; }
                else if (ts == "3") { timeLimit = 180; break; }
            }

            // 输入玩家姓名
            cout << "请输入红方玩家姓名: ";
            string redName, blackName;
            getline(cin, redName);
            if (redName.empty()) redName = "红方";  // 没输入就用默认名

            cout << "请输入黑方玩家姓名: ";
            getline(cin, blackName);
            if (blackName.empty()) blackName = "黑方";

            // 自动编号：扫描已有存档，找最大 playN → N+1
            int n = nextPlayNumber(redName, blackName);

            // 确保 saves 目录存在
            ensureDir("saves");

            // 生成存档文件名 例：saves/张三 vs 李四 play0.txt
            string filename = "saves/" + redName + " vs " + blackName
                            + " play" + to_string(n) + ".txt";

            // 创建游戏并绑定存档路径
            Game game;
            game.start();
            game.setSaveFilename(filename);

            cout << "存档文件: " << filename << endl;
            cout << "按任意键开始游戏..." << endl;
            system("pause > nul");

            playGame(game, redName, blackName, timeLimit);
        }
        else if (choice == "2") {
            // ===== 加载存档 =====
            string fname = showLoadMenu();
            if (fname.empty()) continue;  // 用户取消，回到主菜单

            Game game;
            if (!game.loadGame(fname)) {
                cout << "加载失败! 文件可能已损坏。" << endl;
                system("pause");
                continue;
            }
            game.setSaveFilename(fname);
            // 从文件名提取玩家名: "saves/红方 vs 黑方 play0.txt"
            string base = fname;
            // 去掉 "saves/" 前缀
            size_t slash = base.find('/');
            if (slash != string::npos) base = base.substr(slash + 1);
            // 去掉 " playN.txt" 后缀
            size_t playPos = base.find(" play");
            if (playPos != string::npos) base = base.substr(0, playPos);
            // 按 " vs " 分割
            size_t vsPos = base.find(" vs ");
            string loadRed = "红方", loadBlack = "黑方";
            if (vsPos != string::npos) {
                loadRed  = base.substr(0, vsPos);
                loadBlack = base.substr(vsPos + 4);
            }
            playGame(game, loadRed, loadBlack);
        }
        else if (choice == "3") {
            // ===== 退出程序 =====
            cout << "再见!" << endl;
            break;
        }
    }
    return 0;
}
