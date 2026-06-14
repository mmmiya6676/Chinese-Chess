/*
 * mainwindow.cpp —— 主游戏窗口
 * ============================
 *
 * 【QMainWindow 是什么？】
 *   Qt 提供的主窗口框架，自带菜单栏、工具栏、状态栏等结构。
 *   所有内容放在"中心控件"（centralWidget）上。
 *
 *   QMainWindow 的结构（从上到下）：
 *     ┌──────────────────┐
 *     │    菜单栏 (可选)   │
 *     │    工具栏 (可选)   │
 *     ├──────────────────┤
 *     │                  │
 *     │   中心控件区域     │  ← 放我们的棋盘、按钮、标签
 *     │                  │
 *     ├──────────────────┤
 *     │    状态栏 (可选)   │
 *     └──────────────────┘
 *
 * 【QTimer —— Qt 定时器】
 *   QTimer 每隔一段毫秒数发射 timeout() 信号。
 *   m_timer->start(1000)  = 每 1000 毫秒（1 秒）发射一次。
 *   我们把 timeout() 连到 onTimerTick()，就实现了每秒倒计时。
 *
 *   QTimer::singleShot(ms, receiver, slot)：
 *     "一次性"定时器，只触发一次，适合"延迟执行"。
 *     我们用它让 AI 在人类走棋后延迟 300ms 再出手，
 *     这样人类走棋的高亮效果先显示出来，AI 再思考。
 *
 * 【QSS（Qt Style Sheets）—— 样式表】
 *   语法和 CSS 一样。支持的选择器：
 *     QMainWindow                     → 按类名
 *     QLabel#redName                  → 按类名 + 对象名（setObjectName 设定的）
 *     QPushButton:hover               → 鼠标悬停状态
 *     QPushButton:disabled            → 禁用状态
 *     属性: 值;                        → 和 CSS 一样的键值对
 *
 *   本程序使用古风木色主题：
 *     背景 #d4b896（檀木色）
 *     按钮 #8b7355（棕色）
 *     红方名 #b42222（暗红）
 *     黑方名 #1a1a1a（纯黑）
 *
 * 【布局系统（QLayout）—— 代码排版】
 *   三种常用布局：
 *     QVBoxLayout  — 垂直排列（从上到下）
 *     QHBoxLayout  — 水平排列（从左到右）
 *     QFormLayout  — 标签+控件 成对排列
 *
 *   布局可以嵌套（大布局里套小布局），形成复杂界面。
 *   addStretch() 添加"弹簧"——可伸缩的空白区域，用于居中或推挤。
 *
 *   整体布局树：
 *     QVBoxLayout(主)
 *       ├ QHBoxLayout     ← [设置]   ...   步数:5
 *       ├ QLabel          ← 黑方: xxx
 *       ├ QLabel          ← 剩余 85 秒
 *       ├ QHBoxLayout     ← stretch + BoardWidget + stretch (居中)
 *       ├ QLabel          ← 剩余 80 秒
 *       ├ QLabel          ← 红方: xxx
 *       └ QHBoxLayout     ← stretch + [悔棋][下一步][提示][投降] + stretch
 *
 * 【析构函数与 Qt 的父子关系】
 *   delete m_game   — Game 不是 QWidget，需要手动 delete。
 *   delete m_ai     — ChessAI 同理。
 *   m_board/m_timer 等  — 不需要手动 delete！
 *     Qt 的"父子关系"机制：所有 QObject 子类在创建时指定 parent，
 *     父对象销毁时自动递归 delete 所有子对象。
 *     所以 new QPushButton(this) 中的 this 就是 parent，
 *     MainWindow 销毁时它们全部自动释放。
 */

#include "mainwindow.h"
#include "leaderboarddialog.h"
#include "boardwidget.h"
#include "SaveManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMenu>
#include <QMouseEvent>
#include <QApplication>
#include <chrono>

// MOVE_TIME_LIMIT 定义在 Game.h 中 (90 秒)
extern const int MOVE_TIME_LIMIT;

// =========================== 构造函数 ===========================

MainWindow::MainWindow(const QString& redName, const QString& blackName, QWidget *parent)
    : QMainWindow(parent), m_redName(redName), m_blackName(blackName)
{
    m_game = new Game;
    m_game->start();               // 初始化棋盘（摆好所有棋子）
    m_redTime = MOVE_TIME_LIMIT;
    m_blackTime = MOVE_TIME_LIMIT;

    // 为两个玩家生成存档文件名（用数字 ID 避免中文路径问题）
    int redID = getOrCreatePlayerID(redName.toStdString());
    int blackID = getOrCreatePlayerID(blackName.toStdString());
    int playN = nextPlayNumber(redID, blackID);
    m_game->setSaveFilename(buildSaveFilename(redID, blackID, playN));

    setupUI();         // 搭建界面（创建控件+布局）
    setupStyle();      // 应用 QSS 样式表
    m_board->setGame(m_game);   // 把 Game 对象交给棋盘控件

    // 启动每秒倒计时定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerTick);
    m_timer->start(1000);   // 1000 毫秒 = 1 秒
}

// 人机对战构造函数（红方=人类，黑方=AI）
MainWindow::MainWindow(const QString& redName, AIDifficulty diff, QWidget *parent)
    : QMainWindow(parent), m_redName(redName), m_blackName("电脑 (AI)"), m_aiMode(true)
{
    m_game = new Game;
    m_game->start();
    m_redTime   = MOVE_TIME_LIMIT;
    m_blackTime = MOVE_TIME_LIMIT;

    int redID   = getOrCreatePlayerID(redName.toStdString());
    int blackID = getOrCreatePlayerID("AI");
    int playN   = nextPlayNumber(redID, blackID);
    m_game->setSaveFilename(buildSaveFilename(redID, blackID, playN));

    m_ai = new ChessAI(Color::BLACK);
    m_ai->setDifficulty(diff);

    setupUI();
    setupStyle();
    m_board->setGame(m_game);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerTick);
    m_timer->start(1000);
}

// 读档游戏构造函数
MainWindow::MainWindow(const std::string& saveFile, QWidget *parent)
    : QMainWindow(parent)
{
    m_game = new Game;
    m_game->loadGame("saves/" + saveFile);
    m_redTime = MOVE_TIME_LIMIT;
    m_blackTime = MOVE_TIME_LIMIT;
    m_redName = "红方";
    m_blackName = "黑方";

    setupUI();
    setupStyle();
    m_board->setGame(m_game);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerTick);
    m_timer->start(1000);
    updateDisplay();
}

MainWindow::~MainWindow() {
    delete m_game;   // Game 不是 QWidget，需手动释放
    delete m_ai;     // ChessAI 同理
    // m_board、m_timer 等 QWidget 子类由 Qt 父子关系自动释放
}

// =========================== UI 搭建 ===========================

void MainWindow::setupUI() {
    setWindowTitle("中国象棋");
    resize(780, 680);
    setMinimumSize(720, 620);

    // QMainWindow 需要设置中心控件，所有内容放在它上面
    auto *central = new QWidget;
    setCentralWidget(central);

    auto *vLayout = new QVBoxLayout(central);
    vLayout->setSpacing(4);

    // ---- 顶部行：[设置]   弹簧    步数: 5 ----
    auto *topRow = new QHBoxLayout;
    m_settingsBtn = new QPushButton("设置");
    m_moveCount = new QLabel("步数: 0");
    m_moveCount->setObjectName("moveCount");  // 给 QSS 选择器用的名称
    topRow->addWidget(m_settingsBtn);
    topRow->addStretch();   // 弹簧把设置推左，步数推右
    topRow->addWidget(m_moveCount);
    vLayout->addLayout(topRow);

    // ---- 黑方信息（居中）----
    auto *blackRow = new QHBoxLayout;
    blackRow->addStretch();
    m_blackNameLabel = new QLabel(m_blackName);
    m_blackNameLabel->setObjectName("blackName");
    blackRow->addWidget(m_blackNameLabel);
    blackRow->addStretch();   // 左右都有弹簧 = 居中
    vLayout->addLayout(blackRow);

    m_blackTimer = new QLabel("剩余 90 秒");
    m_blackTimer->setObjectName("timer");
    m_blackTimer->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(m_blackTimer);

    // ---- 棋盘（水平居中）----
    m_board = new BoardWidget;
    m_board->setFixedSize(50*2 + 60*8 + 20, 50*2 + 60*9 + 20);
    auto *boardRow = new QHBoxLayout;
    boardRow->addStretch();
    boardRow->addWidget(m_board);
    boardRow->addStretch();   // 左右弹簧让棋盘居中
    vLayout->addLayout(boardRow);

    // ---- 红方信息（居中）----
    m_redTimer = new QLabel("剩余 90 秒");
    m_redTimer->setObjectName("timer");
    m_redTimer->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(m_redTimer);

    auto *redRow = new QHBoxLayout;
    redRow->addStretch();
    m_redNameLabel = new QLabel(m_redName);
    m_redNameLabel->setObjectName("redName");
    m_turnDot = new QLabel;    // 走棋方指示圆点
    m_turnDot->setObjectName("turnDot");
    redRow->addWidget(m_turnDot);
    redRow->addWidget(m_redNameLabel);
    redRow->addStretch();
    vLayout->addLayout(redRow);

    // ---- 底部按钮（居中）----
    auto *btnRow = new QHBoxLayout;
    m_undoBtn     = new QPushButton("悔棋");
    m_redoBtn     = new QPushButton("下一步");
    m_surrenderBtn = new QPushButton("投降");
    m_hintBtn     = new QPushButton("提示");
    btnRow->addStretch();
    btnRow->addWidget(m_undoBtn);
    btnRow->addWidget(m_redoBtn);
    btnRow->addWidget(m_hintBtn);
    btnRow->addWidget(m_surrenderBtn);
    btnRow->addStretch();
    vLayout->addLayout(btnRow);

    // ===== 连接信号-槽 =====
    /*
     * connect 参数：(发送者, 信号, 接收者, 槽函数)
     * 例：m_board 发射 moveMade → MainWindow::onMoveMade 被调用
     */
    connect(m_board, &BoardWidget::moveMade,       this, &MainWindow::onMoveMade);
    connect(m_board, &BoardWidget::gameOverSignal,  this, &MainWindow::onGameOver);
    connect(m_undoBtn,      &QPushButton::clicked,  this, &MainWindow::onUndo);
    connect(m_redoBtn,      &QPushButton::clicked,  this, &MainWindow::onRedo);
    connect(m_surrenderBtn, &QPushButton::clicked,  this, &MainWindow::onSurrender);
    connect(m_hintBtn,      &QPushButton::clicked,  this, &MainWindow::onHint);
    connect(m_settingsBtn,  &QPushButton::clicked,  this, &MainWindow::onSettings);
}

void MainWindow::setupStyle() {
    setStyleSheet(
        "QMainWindow { background-color: #d4b896; }"
        "QWidget { background-color: #d4b896; }"
        "QLabel { font: 14px '楷体'; color: #3a2010; }"
        "QLabel#redName { font: bold 18px '楷体'; color: #b42222; }"
        "QLabel#blackName { font: bold 18px '楷体'; color: #1a1a1a; }"
        "QLabel#timer { font: bold 16px '楷体'; color: #5a3520; }"
        "QLabel#moveCount { font: bold 16px '楷体'; color: #5a3520; }"
        "QPushButton { font: 14px '楷体'; padding: 6px 16px;"
        "  background-color: #8b7355; color: #fff8f0; border: 2px solid #6b5345; border-radius: 4px; }"
        "QPushButton:hover { background-color: #6b5345; }"
        "QPushButton:disabled { background-color: #c4b096; border-color: #a09080; }"
    );
}

// =========================== 界面更新 ===========================

void MainWindow::updateDisplay() {
    int count = m_game->getMoveCount();
    // 步数 = (历史总步数 + 1) / 2（红黑各算一步）
    m_moveCount->setText(QString("步数: %1").arg((count + 1) / 2));

    // 走棋方指示（红点/黑点）
    Color cur = m_game->getCurrentPlayer();
    m_turnDot->setStyleSheet(cur == Color::RED
        ? "color: #b42222; font: bold 20px;" : "color: #1a1a1a; font: bold 20px;");

    m_redTimer->setText(QString("剩余 %1 秒").arg(m_redTime));
    m_blackTimer->setText(QString("剩余 %1 秒").arg(m_blackTime));
    m_undoBtn->setEnabled(m_game->canUndo());
    m_redoBtn->setEnabled(m_game->canRedo());
}

void MainWindow::resetTimer() {
    if (m_game->getCurrentPlayer() == Color::RED)
        m_redTime = MOVE_TIME_LIMIT;
    else
        m_blackTime = MOVE_TIME_LIMIT;
    updateDisplay();
}

// =========================== 计时 ===========================

void MainWindow::onTimerTick() {
    if (m_game->isGameOver()) return;

    if (m_game->getCurrentPlayer() == Color::RED) {
        m_redTime--;
        if (m_redTime <= 0) { m_redTime = 0; updateDisplay();
            showResult(QString("%1（红方）超时！%2（黑方）获胜！").arg(m_redName, m_blackName));
            saveFileAndRecord(m_redName.toStdString()); return;
        }
    } else {
        m_blackTime--;
        if (m_blackTime <= 0) { m_blackTime = 0; updateDisplay();
            showResult(QString("%1（黑方）超时！%2（红方）获胜！").arg(m_blackName, m_redName));
            saveFileAndRecord(m_blackName.toStdString()); return;
        }
    }
    updateDisplay();
}

// =========================== 走棋/游戏结束回调 ===========================

void MainWindow::onMoveMade() {
    resetTimer();
    updateDisplay();

    // 人机模式：如果轮到 AI（黑方），延迟 300ms 触发
    if (m_aiMode && !m_game->isGameOver() &&
        m_game->getCurrentPlayer() == Color::BLACK) {
        m_undoBtn->setEnabled(false);
        m_redoBtn->setEnabled(false);
        m_surrenderBtn->setEnabled(false);
        m_hintBtn->setEnabled(false);
        QTimer::singleShot(300, this, &MainWindow::doAIMove);
    }
}

void MainWindow::onGameOver() {
    m_timer->stop();
    QString winner = (m_game->getWinner() == Color::RED) ? m_redName : m_blackName;
    QString loser  = (m_game->getWinner() == Color::RED) ? m_blackName : m_redName;
    showResult(QString("将死！%1 获胜！").arg(winner));
    saveFileAndRecord(loser.toStdString());
}

// =========================== 按钮 ===========================

void MainWindow::onUndo()    { m_game->undo(); m_board->clearSelection(); resetTimer(); updateDisplay(); }
void MainWindow::onRedo()    { m_game->redo(); m_board->clearSelection(); resetTimer(); updateDisplay(); }

void MainWindow::onSurrender() {
    auto reply = QMessageBox::question(this, "认输", "确定要认输吗？",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    QString loser  = (m_game->getCurrentPlayer() == Color::RED) ? m_redName : m_blackName;
    QString winner = (m_game->getCurrentPlayer() == Color::RED) ? m_blackName : m_redName;
    m_timer->stop();
    showResult(QString("%1 认输！%2 获胜！").arg(loser, winner));
    saveFileAndRecord(loser.toStdString());
}

// =========================== 设置菜单 ===========================

/*
 * QMenu：弹出菜单控件。
 *   addAction("文字") → 返回 QAction*
 *   addSeparator()    → 添加分隔线
 *   exec(QPoint)      → 在指定屏幕坐标显示菜单
 *
 * mapToGlobal(QPoint(0, height))：
 *   把控件内部的相对坐标 (0, 按钮高度) 转换为屏幕绝对坐标。
 *   效果：菜单从按钮左下角弹出。
 */
void MainWindow::onSettings() {
    QMenu menu;
    QAction *saveAct = menu.addAction("保存游戏");
    menu.addSeparator();
    QAction *rankAct = menu.addAction("排行榜");
    QAction *quitAct = menu.addAction("退出");
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSave);
    connect(rankAct, &QAction::triggered, this, &MainWindow::onLeaderboard);
    connect(quitAct, &QAction::triggered, this, &MainWindow::onQuit);
    menu.exec(m_settingsBtn->mapToGlobal(QPoint(0, m_settingsBtn->height())));
}

void MainWindow::onSave() {
    ensureDir("saves");
    QString f = QString::fromStdString(m_game->getSaveFilename());
    QMessageBox::information(this, "保存",
        m_game->saveGame(f.toStdString()) ? QString("已保存: %1").arg(f) : "保存失败！");
}

void MainWindow::onLeaderboard() {
    LeaderboardDialog(this).exec();
}

void MainWindow::onQuit() {
    if (QMessageBox::question(this, "退出", "确定退出？未保存的进度将丢失。",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        close();   // QMainWindow::close() 关闭窗口，如有 WA_DeleteOnClose 则自动 delete
}

// =========================== 结果处理 ===========================

void MainWindow::showResult(const QString& msg) {
    QMessageBox::information(this, "游戏结束", msg);
}

void MainWindow::saveFileAndRecord(const std::string& loser) {
    ensureDir("saves");
    m_game->saveGame(m_game->getSaveFilename());
    if (m_aiMode) return;   // 人机对战不计入排行榜
    std::string winnerName = (loser == m_redName.toStdString())
                             ? m_blackName.toStdString() : m_redName.toStdString();
    recordGameResult(m_redName.toStdString(), m_blackName.toStdString(), winnerName);
}

// =========================== 提示最佳走法 ===========================

void MainWindow::onHint() {
    if (m_game->isGameOver()) return;

    ChessAI hintAI(m_game->getCurrentPlayer());
    hintAI.setDifficulty(AIDifficulty::Medium);
    ChessAI::AIMove best = hintAI.findBestMove(*m_game);
    if (best.from.getX() < 0) return;

    // 构造一个假鼠标点击，让 BoardWidget 选中 AI 推荐的棋子
    // sendEvent 直接把事件发给目标控件，不走正常的用户交互流程
    QPoint pt = m_board->boardToPixel(best.from.getX(), best.from.getY());
    QMouseEvent pressEvent(QEvent::MouseButtonPress, pt,
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_board, &pressEvent);
    updateDisplay();
}

// =========================== AI 走棋 ===========================

void MainWindow::doAIMove() {
    if (!m_ai || m_game->isGameOver()) return;

    // AI 思考前记录时间，结束后扣除耗时（因为 minimax 会阻塞事件循环）
    auto thinkStart = std::chrono::steady_clock::now();
    int& aiTimer = (m_game->getCurrentPlayer() == Color::RED) ? m_redTime : m_blackTime;

    ChessAI::AIMove best = m_ai->findBestMove(*m_game);

    auto thinkEnd = std::chrono::steady_clock::now();
    int elapsed = std::chrono::duration_cast<std::chrono::seconds>(thinkEnd - thinkStart).count();
    aiTimer -= elapsed;

    if (aiTimer <= 0) {
        aiTimer = 0; updateDisplay(); m_timer->stop();
        showResult(QString("电脑超时！%1 获胜！").arg(m_redName));
        saveFileAndRecord("电脑 (AI)"); return;
    }

    if (best.from.getX() < 0) {
        m_timer->stop();
        showResult(QString("电脑无子可走！%1 获胜！").arg(m_redName));
        saveFileAndRecord("电脑 (AI)"); return;
    }

    m_game->makeMove(best.from, best.to);
    m_board->clearSelection();
    m_board->update();
    m_board->repaint();      // 立即刷新，不走事件队列排队

    resetTimer();
    updateDisplay();

    if (m_game->isGameOver())
        onGameOver();
}
