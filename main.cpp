/*
 * main.cpp —— 中国象棋 Qt 版入口
 * ------------------------------------------------
 * 程序启动流程：
 *   1. 创建 QApplication（每个 Qt 程序必须有且仅有一个）
 *   2. 进入无限循环（玩家可以在初始菜单和游戏之间反复切换）：
 *      a) 显示 InitialDialog（初始菜单）
 *      b) 根据用户选择：
 *         - 新建游戏 → 弹出 NewGameDialog 输入名字 → 打开 MainWindow
 *         - 读档游戏 → 弹出 LoadGameDialog 选存档 → 打开 MainWindow
 *         - 排行榜   → 弹出 LeaderboardDialog
 *         - 退出     → return 0 结束程序
 *   3. MainWindow 关闭后，循环回到步骤 2a，再次显示初始菜单
 *
 * QApplication::exec() —— 进入 Qt 事件循环
 *   Qt 程序启动后进入"事件循环"——无限循环等待用户操作
 *   （鼠标点击、键盘按下、计时器到期等），直到窗口关闭。
 *
 *   QDialog::exec() 也会进入局部事件循环，直到对话框被关闭。
 */

#include "mainwindow.h"
#include "initialdialog.h"
#include "newgamedialog.h"
#include "loadgamedialog.h"
#include "leaderboarddialog.h"
#include <QApplication>       // Qt 应用程序类（必须有）

int main(int argc, char *argv[]) {
    /*
     * QApplication：每个 Qt GUI 程序必须创建一个 QApplication 对象。
     *
     * argc/argv 是 main 函数接收的命令行参数，传给 QApplication 可以
     * 支持一些 Qt 内置的命令行选项（如 -style=fusion 切换主题）。
     */
    QApplication app(argc, argv);
    app.setApplicationName("中国象棋");

    /*
     * 外层 while(true) 循环：
     *   用户关闭游戏窗口后 → 回到初始菜单（而不是直接退出程序）。
     *   用户点"退出" → return 0 跳出循环。
     *
     * 循环每次迭代的流程：
     *   显示初始菜单 → 根据选择操作 → 如果开了游戏窗口，等它关闭 → 循环
     */
    while (true) {
        InitialDialog initDlg;       // 创建初始菜单对话框

        /*
         * exec()：以"模态"方式运行对话框。
         *   - 这会进入一个局部事件循环
         *   - 用户必须关闭此对话框才能继续执行后面的代码
         *   - 返回 QDialog::Accepted（点了某个按钮触发了 accept()）
         *     或 QDialog::Rejected（点了 X 关闭窗口）
         */
        if (initDlg.exec() != QDialog::Accepted)
            break;                   // 用户点 X 关闭 → 退出程序

        switch (initDlg.result()) {
        // ===== 新建游戏 =====
        case InitialDialog::NewGame: {
            // 弹出名字输入框
            NewGameDialog nameDlg;
            if (nameDlg.exec() == QDialog::Accepted) {
                // 用输入的名字创建主游戏窗口
                MainWindow w(nameDlg.redName(), nameDlg.blackName());

                /*
                 * show()：显示窗口（非模态，不阻塞代码执行）
                 * 与 exec() 不同，show() 不会进入事件循环，
                 * 控制权仍然在调用方。
                 */
                w.show();

                /*
                 * app.exec()：进入 Qt 主事件循环。
                 *   这个函数会"卡住"直到所有窗口关闭或调用 QApplication::quit()。
                 *   w.show() 只显示窗口，事件循环从 app.exec() 开始。
                 *
                 *   当 MainWindow 关闭（w.close()），app.exec() 返回，
                 *   程序继续执行 switch 后面的代码 → while 循环回到开头 →
                 *   再次显示初始菜单。
                 */
                app.exec();
            }
            break;
        }

        // ===== 读档游戏 =====
        case InitialDialog::LoadGame: {
            LoadGameDialog loadDlg;            // 读档选择弹窗
            if (loadDlg.exec() == QDialog::Accepted) {
                std::string file = loadDlg.selectedFile();
                if (!file.empty()) {
                    MainWindow w(file);        // 用存档构造（触发 loadGame）
                    w.show();
                    app.exec();
                }
            }
            break;
        }

        // ===== 排行榜 =====
        case InitialDialog::Leaderboard: {
            LeaderboardDialog rankDlg;
            rankDlg.exec();                    // 关闭排行榜后回到初始菜单
            break;                             // ← 注意：回到 while 循环顶部
        }

        // ===== 退出程序 =====
        case InitialDialog::Quit:
            return 0;
        }
    }

    return 0;
}
