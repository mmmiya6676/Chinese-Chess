/*
 * main.cpp — 中国象棋 Qt 版入口
 * ================================
 *
 * 【什么是 QApplication？】
 *   每个 Qt GUI 程序"必须且只能"创建一个 QApplication 对象。
 *   它的作用：
 *     1. 管理程序的"事件循环"（后面解释）
 *     2. 处理命令行参数（如 -style=fusion 切换主题）
 *     3. 管理全局设置（字体、样式、剪贴板等）
 *   QApplication 必须在创建任何窗口之前初始化。
 *
 * 【什么是事件循环（Event Loop）？】
 *   你可能会想：程序不是从上到下执行完就结束了吗？
 *   但 GUI 程序不能这样——窗口要一直显示着，等用户操作。
 *
 *   Qt 的方案：进入一个无限循环（事件循环），不断问系统
 *   "有新的鼠标点击吗？有键盘按下吗？定时器到期了吗？"
 *   有事件就分发给对应的控件处理，没有就继续等。
 *
 *   app.exec() 就是"进入事件循环"的命令。
 *   它会一直运行，直到所有窗口关闭。
 *
 * 【exec() 和 show() 的区别】
 *   - dialog.exec()："模态"运行，代码停在这行不动，
 *     直到用户关闭对话框才继续。就像弹窗问你"确定吗？"，
 *     不关掉就不能做其他事。
 *
 *   - widget.show()："非模态"显示，代码立刻继续往下走，
 *     窗口显示着，但程序继续执行后面的代码。
 *
 *   本程序的流程：
 *     while(true) {
 *         ① 显示初始菜单  (exec() — 等用户选择)
 *         ② 根据选择：
 *            - 新建/读档 → 创建 MainWindow → show() → app.exec()
 *            - 排行榜   → 显示排行榜 → 回到 ①
 *            - 退出     → return 0
 *     }
 *
 *   为什么外层有 while(true)？
 *     一盘棋下完后，MainWindow 关闭 → app.exec() 返回 →
 *     循环回到开头 → 再次显示初始菜单 → 玩家可以再来一局。
 *     只有点"退出"才会真正结束程序。
 */

#include "mainwindow.h"
#include "initialdialog.h"
#include "newgamedialog.h"
#include "loadgamedialog.h"
#include "leaderboarddialog.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("中国象棋");

    while (true) {
        // 创建初始菜单 —— 这是一个"模态"对话框
        InitialDialog initDlg;

        /*
         * exec() —— 以"模态"方式运行对话框
         *   代码停在这一行，直到用户关了对话框才继续。
         *   返回值：
         *     QDialog::Accepted  = 用户点了某个有效按钮（触发了 accept()）
         *     QDialog::Rejected  = 用户点了 X 关闭窗口
         *
         *   如果用户直接点 X 关闭 → break 跳出 while → 程序退出。
         */
        if (initDlg.exec() != QDialog::Accepted)
            break;

        switch (initDlg.result()) {

        // ===== 新建游戏 =====
        case InitialDialog::NewGame: {
            NewGameDialog nameDlg;   // 输入名字 / 选 AI 模式
            if (nameDlg.exec() == QDialog::Accepted) {
                /*
                 * 根据是否勾选 AI 模式，选择不同的构造函数：
                 *   人机对战 → new MainWindow(红方名, AI难度)
                 *   人人对战 → new MainWindow(红方名, 黑方名)
                 *
                 * WA_DeleteOnClose：窗口关闭时自动 delete 自己，
                 * 避免内存泄漏。因为这里用的是 new 而不是栈对象。
                 */
                MainWindow *w = nameDlg.isAIMode()
                    ? new MainWindow(nameDlg.redName(), nameDlg.aiDifficulty())
                    : new MainWindow(nameDlg.redName(), nameDlg.blackName());
                w->setAttribute(Qt::WA_DeleteOnClose);

                /*
                 * show() —— 非模态显示窗口
                 *   和 exec() 不同，show() 不会"卡住"代码。
                 *   它只是把窗口画出来，然后代码立刻继续往下走。
                 *
                 *   所以紧接着要调用 app.exec() 进入事件循环，
                 *   否则程序会直接退出（窗口闪一下就没了）。
                 */
                w->show();

                /*
                 * app.exec() —— 进入 Qt 事件循环
                 *   程序"卡"在这一行，直到 MainWindow 关闭。
                 *   期间 Qt 不断处理鼠标点击、键盘、定时器等事件。
                 *
                 *   窗口关闭后 app.exec() 返回 → while 循环回到开头 →
                 *   再次显示初始菜单。
                 */
                app.exec();
            }
            break;
        }

        // ===== 读档游戏 =====
        case InitialDialog::LoadGame: {
            LoadGameDialog loadDlg;
            if (loadDlg.exec() == QDialog::Accepted) {
                std::string file = loadDlg.selectedFile();
                if (!file.empty()) {
                    MainWindow *w = new MainWindow(file);
                    w->setAttribute(Qt::WA_DeleteOnClose);
                    w->show();
                    app.exec();
                }
            }
            break;
        }

        // ===== 排行榜（查看完回到初始菜单）=====
        case InitialDialog::Leaderboard: {
            LeaderboardDialog rankDlg;
            rankDlg.exec();   // 排行榜关闭后 → 回到 while 顶部 → 初始菜单
            break;            // ← 注意是 break 不是 return，所以不会退出 while
        }

        // ===== 退出 =====
        case InitialDialog::Quit:
            return 0;
        }
    }

    return 0;
}
