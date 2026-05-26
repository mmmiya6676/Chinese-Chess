/*
 * 中国象棋 - 主程序
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include "../include/Game.h"
#include "../include/SaveManager.h"
#include "../include/MouseInput.h"
using namespace std;

void playGame(Game& game, const string& redName, const string& blackName) {
    system("cls");  // 清掉菜单残留文字
    while (!game.isGameOver()) {
        InputResult res = mouseInputLoop(game);
        if (res == InputResult::Quit) return;
    }

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

int main() {
    system("chcp 65001 > nul");

    while (true) {
        system("cls");
        cout << "===== 中国象棋 =====" << endl;
        cout << "1. 新建游戏" << endl;
        cout << "2. 加载存档" << endl;
        cout << "3. 排行榜" << endl;
        cout << "4. 退出" << endl;
        cout << "请选择 (1-4): ";

        string choice;
        getline(cin, choice);

        if (choice == "1") {
            system("cls");
            cout << "===== 新建游戏 =====" << endl;

            cout << "请输入红方玩家姓名: ";
            string redName, blackName;
            getline(cin, redName);
            if (redName.empty()) redName = "红方";

            cout << "请输入黑方玩家姓名: ";
            getline(cin, blackName);
            if (blackName.empty()) blackName = "黑方";

            int redID   = getOrCreatePlayerID(redName);
            int blackID = getOrCreatePlayerID(blackName);
            int n = nextPlayNumber(redID, blackID);
            ensureDir("saves");
            string filename = buildSaveFilename(redID, blackID, n);

            Game game;
            game.start();
            game.setSaveFilename(filename);

            cout << "存档文件: " << filename << endl;
            cout << "按任意键开始游戏..." << endl;
            system("pause > nul");

            playGame(game, redName, blackName);
        }
        else if (choice == "2") {
            string fname = showLoadMenu();
            if (fname.empty()) continue;

            Game game;
            if (!game.loadGame(fname)) {
                cout << "加载失败! 文件可能已损坏。" << endl;
                system("pause");
                continue;
            }
            game.setSaveFilename(fname);

            string loadRed = "红方", loadBlack = "黑方";
            string base = fname;
            size_t slash = base.find('/');
            if (slash != string::npos) base = base.substr(slash + 1);
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
            showLeaderboard();
            cout << endl << "请按任意键返回主菜单..." << endl;
            system("pause > nul");
        }
        else if (choice == "4") {
            cout << "再见!" << endl;
            break;
        }
    }
    return 0;
}
