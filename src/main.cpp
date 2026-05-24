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
#include "../include/MouseInput.h"
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
        InputResult res = mouseInputLoop(game);
        if (res == InputResult::Quit) return;
        // Move: 走了一步，继续循环
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

            // 玩家名 → 数字ID（中文名也能安全用作文件名）
            int redID   = getOrCreatePlayerID(redName);
            int blackID = getOrCreatePlayerID(blackName);
            int n = nextPlayNumber(redID, blackID);

            // 确保 saves 目录存在
            ensureDir("saves");

            // 生成存档文件名 例：saves/1_vs_2_play0.txt
            string filename = buildSaveFilename(redID, blackID, n);

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
            // 从文件名提取玩家名: "saves/1_vs_2_play0.txt" → "张三" "李四"
            string loadRed = "红方", loadBlack = "黑方";
            string base = fname;
            size_t slash = base.find('/');
            if (slash != string::npos) base = base.substr(slash + 1);  // 去 "saves/"
            size_t vsPos = base.find("_vs_");
            size_t playPos = base.find("_play");
            if (vsPos != string::npos && playPos != string::npos) {
                int rID = stoi(base.substr(0, vsPos));
                int bID = stoi(base.substr(vsPos + 4, playPos - vsPos - 4));
                loadRed   = getPlayerName(rID);
                loadBlack = getPlayerName(bID);
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
