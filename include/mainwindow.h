#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>     // Qt 的主窗口类（自带菜单栏、状态栏、中心区域）
#include <QLabel>          // 标签控件，用于显示文字
#include <QPushButton>     // 按钮控件
#include <QTimer>          // 定时器，用于每秒更新倒计时
#include <string>
#include "boardwidget.h"   // 我们的棋盘控件
#include "Game.h"          // 游戏逻辑

/*
 * MainWindow —— 主游戏窗口
 * ------------------------------------
 * 继承 QMainWindow，是玩家进入游戏后的主界面。
 * 布局从上到下：
 *   [设置]                     步数: 5
 *          黑方: 玩家名
 *          剩余 85 秒
 *        ┌─── 棋盘 ───┐
 *        │            │
 *        └────────────┘
 *          剩余 80 秒
 *          红方: 玩家名
 *      [悔棋] [下一步] [投降]
 *
 * 计时机制：
 *   用 QTimer 每 1000 毫秒（1 秒）触发一次 onTimerTick()，
 *   减少当前走棋方的剩余时间，到 0 则判负。
 *
 * 两个构造函数：
 *   新建游戏：传入红方名、黑方名
 *   读档游戏：传入存档文件路径
 *
 * 信号-槽简介（Qt 核心机制）：
 *   信号(signal)：一个对象发出通知，如 BoardWidget 说"我走了一步棋"
 *   槽(slot)：另一个对象响应通知，如 MainWindow 收到后更新计时和步数
 *   连接(connect)：signal → slot 的绑定
 *   例如：connect(m_board, &BoardWidget::moveMade, this, &MainWindow::onMoveMade);
 */
class MainWindow : public QMainWindow {
    Q_OBJECT    // Qt 元对象宏，启用信号/槽
public:
    // 新建游戏
    explicit MainWindow(const QString& redName, const QString& blackName,
                        QWidget *parent = nullptr);
    // 从存档加载
    explicit MainWindow(const std::string& saveFile, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:                     // Qt 槽函数 —— 被信号触发后调用的函数
    void onTimerTick();            // QTimer 超时回调 → 减少剩余时间
    void onMoveMade();             // 走了一步棋 → 重置计时、更新步数
    void onGameOver();             // 将死 → 停止计时、弹出结果
    void onUndo();                 // 悔棋按钮
    void onRedo();                 // 下一步按钮
    void onSurrender();            // 投降按钮 → 确认后判负
    void onSettings();             // 设置按钮 → 弹出下拉菜单
    void onSave();                 // 保存游戏 → 写入存档文件
    void onLeaderboard();          // 打开排行榜弹窗
    void onQuit();                 // 退出游戏 → 确认后关闭窗口

private:
    void setupUI();                // 搭建界面布局（创建控件、摆放位置）
    void setupStyle();             // 设置 QSS 样式表（颜色、字体等）
    void updateDisplay();          // 刷新所有界面文字（计时、步数、按钮状态）
    void resetTimer();             // 重置当前走棋方的倒计时为 90 秒
    void showResult(const QString& msg);               // 弹出游戏结果
    void saveFileAndRecord(const std::string& loser);  // 保存游戏 + 记录战绩

    // ---- 控件成员 ----
    BoardWidget *m_board;          // 棋盘控件（自定义 QWidget）
    Game *m_game;                  // 游戏逻辑对象（堆分配，析构时 delete）

    QLabel *m_blackNameLabel;      // 显示黑方玩家名
    QLabel *m_redNameLabel;        // 显示红方玩家名
    QLabel *m_blackTimer;          // 黑方倒计时 "剩余 85 秒"
    QLabel *m_redTimer;            // 红方倒计时
    QLabel *m_moveCount;           // 步数 "步数: 5"
    QLabel *m_turnDot;             // 当前走棋方指示（红点/黑点）

    QPushButton *m_undoBtn;        // 悔棋按钮
    QPushButton *m_redoBtn;        // 下一步按钮
    QPushButton *m_surrenderBtn;   // 投降按钮
    QPushButton *m_settingsBtn;    // 设置按钮

    // ---- 计时成员 ----
    QTimer *m_timer;               // Qt 定时器，每 1000ms 触发一次
    int m_redTime;                 // 红方剩余秒数
    int m_blackTime;               // 黑方剩余秒数
    QString m_redName;             // 红方玩家名
    QString m_blackName;           // 黑方玩家名
};

#endif
