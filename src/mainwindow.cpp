#include "mainwindow.h"
#include "leaderboarddialog.h"
#include "boardwidget.h"
#include "SaveManager.h"
#include <QVBoxLayout>       // 垂直布局：控件从上到下排列
#include <QHBoxLayout>       // 水平布局：控件从左到右排列
#include <QMessageBox>       // 消息弹窗（信息/警告/确认）
#include <QMenu>             // 弹出菜单（设置按钮的下拉菜单）
#include <QApplication>      // Qt 应用程序类

/*
 * extern —— 告诉编译器：MOVE_TIME_LIMIT 定义在 Timer.cpp 里，
 * 这里只是引用它。值是 90（每步 90 秒倒计时）。
 */
extern const int MOVE_TIME_LIMIT;

// =========================== 构造函数 ===========================

/*
 * 新建游戏构造函数
 * 参数：
 *   redName  — 红方玩家名
 *   blackName — 黑方玩家名
 *   parent   — Qt 的父子关系机制（这里传 nullptr，独立窗口）
 *
 * 流程：
 *   1. 创建 Game 对象并初始化棋盘
 *   2. 设置双方倒计时为 90 秒
 *   3. 通过玩家 ID 生成存档文件名
 *   4. 搭建界面
 *   5. 启动 1 秒定时器
 */
MainWindow::MainWindow(const QString& redName, const QString& blackName, QWidget *parent)
    : QMainWindow(parent), m_redName(redName), m_blackName(blackName)
{
    m_game = new Game;
    m_game->start();               // 初始化棋盘
    m_redTime = MOVE_TIME_LIMIT;   // 初始 90 秒
    m_blackTime = MOVE_TIME_LIMIT;

    // 为两个玩家生成数字 ID，构建存档文件名（避免中文路径问题）
    int redID = getOrCreatePlayerID(redName.toStdString());
    int blackID = getOrCreatePlayerID(blackName.toStdString());
    int playN = nextPlayNumber(redID, blackID);
    std::string filename = buildSaveFilename(redID, blackID, playN);
    m_game->setSaveFilename(filename);   // 存档文件名为 "1_vs_2_play1.txt" 形式

    setupUI();         // 创建控件和布局
    setupStyle();      // 应用古风 QSS 样式表
    m_board->setGame(m_game);   // 把 Game 对象交给棋盘控件

    /*
     * QTimer —— Qt 定时器
     *   参数 this：定时器的父对象（MainWindow 销毁时自动销毁定时器）
     *   timeout() 信号：每次定时结束发射（每 1000 毫秒 = 1 秒）
     *   start(1000)：启动定时器，间隔 1000 毫秒
     */
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerTick);
    m_timer->start(1000);
}

/*
 * 读档游戏构造函数
 * 流程跟新建差不多，区别是第一步调用 loadGame 加载存档，
 * 而不是 start() 初始化棋盘。
 *
 * m_game->loadGame(saveFile)：从文件恢复棋盘状态 + 步数 + 走棋方。
 * 存档文件路径格式："saves/1_vs_2_play1.txt"
 */
MainWindow::MainWindow(const std::string& saveFile, QWidget *parent)
    : QMainWindow(parent)
{
    m_game = new Game;
    m_game->loadGame("saves/" + saveFile);   // loadGame 需要 "saves/" 前缀
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
    updateDisplay();   // 加载后立即刷新显示
}

/*
 * 析构函数 —— 释放 Game 对象
 *
 * QMainWindow 的子控件（m_board、m_timer 等）Qt 会自动释放，
 * 因为 Qt 的父子关系机制：父对象销毁时自动 delete 所有子对象。
 * 但 m_game 不是 QWidget 子类，需要手动 delete。
 */
MainWindow::~MainWindow() {
    delete m_game;
}

// =========================== UI 搭建 ===========================

/*
 * setupUI —— 用代码创建所有控件并排版
 * ------------------------------------------------
 * 布局系统（QLayout）：
 *   Qt 有三种常用布局：
 *     QVBoxLayout  — 垂直排列（从上到下）
 *     QHBoxLayout  — 水平排列（从左到右）
 *     QGridLayout  — 网格排列
 *
 *   布局是嵌套的：大布局里套小布局，形成完整界面。
 *
 *   addStretch() 在布局里加"弹簧"——可以拉伸的空白，
 *   两边的 stretch 相等就能让中间的控件居中。
 *
 * 整体结构：
 *   QVBoxLayout(主)          ← 所有内容从最外层垂直排列
 *     ├ QHBoxLayout(顶行)     ← [设置]          步数: 5
 *     ├ QLabel(黑方名)        ← 黑方: 玩家名（居中）
 *     ├ QLabel(黑方计时)      ← 剩余 85 秒（居中）
 *     ├ QHBoxLayout(棋盘行)   ← stretch + m_board + stretch（居中）
 *     ├ QLabel(红方计时)      ← 剩余 80 秒（居中）
 *     ├ QLabel(红方名)        ← 红方: 玩家名（居中）
 *     └ QHBoxLayout(底行)     ← stretch + [悔棋][下一步][投降] + stretch
 */
void MainWindow::setupUI() {
    setWindowTitle("中国象棋");
    resize(780, 680);            // 初始窗口尺寸
    setMinimumSize(720, 620);    // 最小窗口尺寸，防止缩太小

    // centralWidget：QMainWindow 需要一个中心控件，所有内容放在它上面
    auto *central = new QWidget;
    setCentralWidget(central);

    auto *vLayout = new QVBoxLayout(central);
    vLayout->setSpacing(4);      // 控件之间的间距 4 像素

    // ---- 顶部行：[设置]              步数: 5 ----
    auto *topRow = new QHBoxLayout;
    m_settingsBtn = new QPushButton("设置");
    m_moveCount = new QLabel("步数: 0");
    m_moveCount->setObjectName("moveCount");  // setObjectName 让 QSS 能按名称定位
    topRow->addWidget(m_settingsBtn);
    topRow->addStretch();          // 弹簧：把设置按钮推到左边，步数推到右边
    topRow->addWidget(m_moveCount);
    vLayout->addLayout(topRow);

    // ---- 黑方信息 ----
    auto *blackRow = new QHBoxLayout;
    blackRow->addStretch();
    m_blackNameLabel = new QLabel(m_blackName);
    m_blackNameLabel->setObjectName("blackName");
    blackRow->addWidget(m_blackNameLabel);
    blackRow->addStretch();        // 居中显示
    vLayout->addLayout(blackRow);

    m_blackTimer = new QLabel("剩余 90 秒");
    m_blackTimer->setObjectName("timer");
    m_blackTimer->setAlignment(Qt::AlignCenter);  // 文字居中对齐
    vLayout->addWidget(m_blackTimer);

    // ---- 棋盘（居中） ----
    m_board = new BoardWidget;
    m_board->setFixedSize(50*2 + 60*8 + 20, 50*2 + 60*9 + 20);
    auto *boardRow = new QHBoxLayout;
    boardRow->addStretch();
    boardRow->addWidget(m_board);
    boardRow->addStretch();        // 棋盘水平居中
    vLayout->addLayout(boardRow);

    // ---- 红方信息 ----
    m_redTimer = new QLabel("剩余 90 秒");
    m_redTimer->setObjectName("timer");
    m_redTimer->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(m_redTimer);

    auto *redRow = new QHBoxLayout;
    redRow->addStretch();
    m_redNameLabel = new QLabel(m_redName);
    m_redNameLabel->setObjectName("redName");
    m_turnDot = new QLabel;
    m_turnDot->setObjectName("turnDot");
    redRow->addWidget(m_turnDot);   // 走棋方指示圆点
    redRow->addWidget(m_redNameLabel);
    redRow->addStretch();           // 居中
    vLayout->addLayout(redRow);

    // ---- 底部按钮：[悔棋] [下一步] [投降] ----
    auto *btnRow = new QHBoxLayout;
    m_undoBtn = new QPushButton("悔棋");
    m_redoBtn = new QPushButton("下一步");
    m_surrenderBtn = new QPushButton("投降");
    btnRow->addStretch();
    btnRow->addWidget(m_undoBtn);
    btnRow->addWidget(m_redoBtn);
    btnRow->addWidget(m_surrenderBtn);
    btnRow->addStretch();          // 按钮组居中
    vLayout->addLayout(btnRow);

    // ===== 连接信号-槽 =====
    /*
     * connect(发送者, 信号, 接收者, 槽)
     *   发送者发出信号 → 接收者的槽函数被自动调用
     *
     * 这里：m_board 走了一步棋 → MainWindow 更新界面
     */
    connect(m_board, &BoardWidget::moveMade,       this, &MainWindow::onMoveMade);
    connect(m_board, &BoardWidget::gameOverSignal,  this, &MainWindow::onGameOver);
    connect(m_undoBtn,      &QPushButton::clicked,  this, &MainWindow::onUndo);
    connect(m_redoBtn,      &QPushButton::clicked,  this, &MainWindow::onRedo);
    connect(m_surrenderBtn, &QPushButton::clicked,  this, &MainWindow::onSurrender);
    connect(m_settingsBtn,  &QPushButton::clicked,  this, &MainWindow::onSettings);
}

/*
 * setupStyle —— QSS（Qt Style Sheets）样式表
 * ------------------------------------------------
 * QSS 语法和 CSS（网页样式表）几乎一样：
 *   选择器 { 属性: 值; 属性: 值; }
 *
 * 选择器类型：
 *   QMainWindow    — 按类名选择
 *   QLabel#redName — 按类名 + 对象名选择（# 号后面是 setObjectName 设定的名字）
 *   QPushButton:hover — 鼠标悬停状态
 *   QPushButton:disabled — 禁用状态
 *
 * 这里用的配色主题：古风木色调
 *   背景：檀木色 #d4b896
 *   按钮：棕色 #8b7355
 *   文字：深褐 #3a2010
 *   红方名：暗红 #b42222
 *   黑方名：纯黑 #1a1a1a
 */
void MainWindow::setupStyle() {
    setStyleSheet(
        "QMainWindow { background-color: #d4b896; }"    // 主窗口背景
        "QWidget { background-color: #d4b896; }"         // 所有控件背景
        "QLabel { font: 14px '楷体'; color: #3a2010; }" // 标签默认样式
        "QLabel#redName { font: bold 18px '楷体'; color: #b42222; }"  // 红方名：大号红字
        "QLabel#blackName { font: bold 18px '楷体'; color: #1a1a1a; }" // 黑方名：大号黑字
        "QLabel#timer { font: bold 16px '楷体'; color: #5a3520; }"    // 计时器
        "QLabel#moveCount { font: bold 16px '楷体'; color: #5a3520; }" // 步数
        "QPushButton { font: 14px '楷体'; padding: 6px 16px;"
        "  background-color: #8b7355; color: #fff8f0;"   // 按钮：棕底白字
        "  border: 2px solid #6b5345; border-radius: 4px; }"  // 圆角边框
        "QPushButton:hover { background-color: #6b5345; }"     // 鼠标悬停：颜色加深
        "QPushButton:disabled { background-color: #c4b096; border-color: #a09080; }"  // 禁用时灰色
    );
}

// =========================== 界面更新 ===========================

/*
 * updateDisplay —— 刷新界面所有信息
 * 每走一步棋、每秒计时都会调用。
 */
void MainWindow::updateDisplay() {
    // 步数 = (历史步数 + 1) / 2，因为红黑各算一步
    // 例如 moveCount=3 表示红走了 2 步，黑走了 1 步，显示 "步数: 2"
    int count = m_game->getMoveCount();
    m_moveCount->setText(QString("步数: %1").arg((count + 1) / 2));

    // 走棋方指示
    Color cur = m_game->getCurrentPlayer();
    m_turnDot->setStyleSheet(cur == Color::RED
        ? "color: #b42222; font: bold 20px;"   // 红方时显示红点
        : "color: #1a1a1a; font: bold 20px;");  // 黑方时显示黑点

    m_redTimer->setText(QString("剩余 %1 秒").arg(m_redTime));
    m_blackTimer->setText(QString("剩余 %1 秒").arg(m_blackTime));

    // 悔棋/下一步按钮：只有在有可悔/可重做的步时才启用
    m_undoBtn->setEnabled(m_game->canUndo());
    m_redoBtn->setEnabled(m_game->canRedo());
}

// resetTimer —— 走完一步后，把当前走棋方的计时重置为 90 秒
void MainWindow::resetTimer() {
    if (m_game->getCurrentPlayer() == Color::RED)
        m_redTime = MOVE_TIME_LIMIT;
    else
        m_blackTime = MOVE_TIME_LIMIT;
    updateDisplay();
}

// =========================== 计时 ===========================

/*
 * onTimerTick —— QTimer 每 1 秒触发
 * 减少当前走棋方的剩余时间，到 0 时判负。
 */
void MainWindow::onTimerTick() {
    if (m_game->isGameOver()) return;

    if (m_game->getCurrentPlayer() == Color::RED) {
        m_redTime--;
        if (m_redTime <= 0) { m_redTime = 0; updateDisplay();
            showResult(QString("%1（红方）超时！%2（黑方）获胜！").arg(m_redName, m_blackName));
            saveFileAndRecord(m_redName.toStdString());
            return;
        }
    } else {
        m_blackTime--;
        if (m_blackTime <= 0) { m_blackTime = 0; updateDisplay();
            showResult(QString("%1（黑方）超时！%2（红方）获胜！").arg(m_blackName, m_redName));
            saveFileAndRecord(m_blackName.toStdString());
            return;
        }
    }
    updateDisplay();  // 刷新计时显示
}

// =========================== 走棋/游戏结束回调 ===========================

// BoardWidget 发来 "走了一步棋" → 重置倒计时
void MainWindow::onMoveMade() { resetTimer(); updateDisplay(); }

// BoardWidget 发来 "将死了" → 停止计时、显示结果、记录战绩
void MainWindow::onGameOver() {
    m_timer->stop();
    QString winner = (m_game->getWinner() == Color::RED) ? m_redName : m_blackName;
    QString loser  = (m_game->getWinner() == Color::RED) ? m_blackName : m_redName;
    showResult(QString("将死！%1 获胜！").arg(winner));
    saveFileAndRecord(loser.toStdString());
}

// =========================== 按钮槽函数 ===========================

// 悔棋：Game::undo() 回退一步，清除选中，重置计时
void MainWindow::onUndo()    { m_game->undo(); m_board->clearSelection(); resetTimer(); updateDisplay(); }
// 下一步（重做）：Game::redo() 恢复悔棋撤销的一步
void MainWindow::onRedo()    { m_game->redo(); m_board->clearSelection(); resetTimer(); updateDisplay(); }

/*
 * 投降：弹出确认框 → 当前走棋方判负 → 停止计时 → 记录战绩
 *
 * QMessageBox::question 弹出一个带"是/否"按钮的确认对话框，
 * 返回 QMessageBox::Yes 或 QMessageBox::No。
 */
void MainWindow::onSurrender() {
    auto reply = QMessageBox::question(this, "认输",
        "确定要认输吗？", QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QString loser  = (m_game->getCurrentPlayer() == Color::RED) ? m_redName : m_blackName;
    QString winner = (m_game->getCurrentPlayer() == Color::RED) ? m_blackName : m_redName;
    m_timer->stop();
    showResult(QString("%1 认输！%2 获胜！").arg(loser, winner));
    saveFileAndRecord(loser.toStdString());
}

// =========================== 设置菜单 ===========================

/*
 * onSettings —— "设置"按钮点击 → 弹出一个下拉菜单
 *
 * QMenu：Qt 的弹出菜单控件
 *   addAction("文字") 返回 QAction 指针
 *   connect(action, triggered, ...) 绑定菜单项点击事件
 *   exec(QPoint) 在指定屏幕坐标位置显示菜单
 *
 * mapToGlobal：把控件内的相对坐标转换为屏幕绝对坐标
 *   参数 (0, height) 表示按钮左下角
 */
void MainWindow::onSettings() {
    QMenu menu;
    QAction *saveAct = menu.addAction("保存游戏");
    menu.addSeparator();                           // 分隔线
    QAction *rankAct = menu.addAction("排行榜");
    QAction *quitAct = menu.addAction("退出");

    connect(saveAct, &QAction::triggered, this, &MainWindow::onSave);
    connect(rankAct, &QAction::triggered, this, &MainWindow::onLeaderboard);
    connect(quitAct, &QAction::triggered, this, &MainWindow::onQuit);

    menu.exec(m_settingsBtn->mapToGlobal(QPoint(0, m_settingsBtn->height())));
}

void MainWindow::onSave() {
    ensureDir("saves");   // 确保 saves/ 目录存在
    QString f = QString::fromStdString(m_game->getSaveFilename());
    QMessageBox::information(this, "保存",
        m_game->saveGame("saves/" + f.toStdString())    // 存档写入 saves/ 目录
        ? QString("已保存: %1").arg(f) : "保存失败！");
}

void MainWindow::onLeaderboard() {
    LeaderboardDialog(this).exec();    // 模态弹窗：必须关闭排行榜才能继续操作
}

void MainWindow::onQuit() {
    if (QMessageBox::question(this, "退出", "确定退出？未保存的进度将丢失。",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        close();    // QMainWindow::close() — 关闭窗口（会触发析构函数）
}

// =========================== 辅助函数 ===========================

void MainWindow::showResult(const QString& msg) {
    QMessageBox::information(this, "游戏结束", msg);
}

/*
 * saveFileAndRecord —— 保存游戏并记录战绩
 *
 * recordGameResult：写入排行榜数据文件，
 *   参数格式：(红方名, 黑方名, 胜者名)
 */
void MainWindow::saveFileAndRecord(const std::string& loser) {
    ensureDir("saves");
    m_game->saveGame("saves/" + m_game->getSaveFilename());
    std::string winnerName = (loser == m_redName.toStdString())
                             ? m_blackName.toStdString() : m_redName.toStdString();
    recordGameResult(winnerName, loser, winnerName);
}
