#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>     // Qt 的主窗口类（自带菜单栏、状态栏、中心区域）
#include <QLabel>          // 标签控件，用于显示文字
#include <QPushButton>     // 按钮控件
#include <QTimer>          // 定时器，用于每秒更新倒计时
#include <string>
#include "boardwidget.h"   // 我们的棋盘控件
#include "Game.h"          // 游戏逻辑
#include "ChessAI.h"       // AI 引擎

/*
 * MainWindow —— 主游戏窗口（含 AI 人机对战）
 * ------------------------------------
 * 三种构造函数：
 *   1. 双人对战：传入红方名、黑方名
 *   2. 人机对战：传入红方名、AI 难度
 *   3. 读档游戏：传入存档文件路径
 *
 * AI 驱动机制：
 *   人类走棋 → onMoveMade() → 判断是否 AI 回合 →
 *   QTimer::singleShot(500ms) → doAIMove() →
 *   AI 走完 → onMoveMade() → 回到人类回合
 */
class MainWindow : public QMainWindow {
    Q_OBJECT    // Qt 元对象宏，启用信号/槽
public:
    // 双人对战
    explicit MainWindow(const QString& redName, const QString& blackName,
                        QWidget *parent = nullptr);
    // 人机对战（redName=人类, AI 执黑）
    explicit MainWindow(const QString& redName, AIDifficulty diff,
                        QWidget *parent = nullptr);
    // 从存档加载
    explicit MainWindow(const std::string& saveFile, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
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
    void doAIMove();               // AI 计算并走一步
    void onHint();                 // 提示最佳走法

private:
    void setupUI();                // 搭建界面布局
    void setupStyle();             // 设置 QSS 样式表
    void updateDisplay();          // 刷新所有界面文字
    void resetTimer();             // 重置当前走棋方的倒计时为 90 秒
    void showResult(const QString& msg);
    void saveFileAndRecord(const std::string& loser);

    BoardWidget *m_board;
    Game *m_game;

    QLabel *m_blackNameLabel;
    QLabel *m_redNameLabel;
    QLabel *m_blackTimer;
    QLabel *m_redTimer;
    QLabel *m_moveCount;
    QLabel *m_turnDot;

    QPushButton *m_undoBtn;
    QPushButton *m_redoBtn;
    QPushButton *m_surrenderBtn;
    QPushButton *m_hintBtn;
    QPushButton *m_settingsBtn;

    QTimer *m_timer;
    int m_redTime;
    int m_blackTime;
    QString m_redName;
    QString m_blackName;

    // ---- AI 成员 ----
    ChessAI *m_ai = nullptr;       // AI 引擎（仅人机模式启用）
    bool m_aiMode = false;         // 是否人机对战模式
};

#endif
