/*
 * initialdialog.cpp —— 初始菜单
 * ==============================
 *
 * 【什么是 QDialog？】
 *   QDialog 是 Qt 的"对话框"基类。对话框是一种特殊的窗口：
 *   - 默认是"模态"的：弹出来后必须关掉才能操作程序的其他部分
 *   - 通常用于获取用户输入或选择（登录框、设置窗、确认框等）
 *   - exec() 运行对话框，accept()/reject() 关闭它
 *
 * 【Q_OBJECT 宏是什么？】
 *   这不是 C++ 语法，而是 Qt 特有的"标记"。
 *   Qt 有个叫 MOC（Meta-Object Compiler）的工具，在编译前扫描
 *   所有写了 Q_OBJECT 的 .h 文件，自动生成信号/槽的底层代码。
 *   只要你的类用到了 signals: 或 slots: 或 connect()，就必须写 Q_OBJECT。
 *   忘记写的话，链接时会报 "undefined reference to vtable" 错误。
 *
 * 【什么是信号（signal）和槽（slot）？】
 *   这是 Qt 对象之间通信的核心机制。
 *
 *   - 信号（signal）：一个对象发出的"通知"。
 *     比如按钮被点击 → 按钮发出 clicked() 信号。
 *     信号只声明不实现，由 MOC 自动生成代码。
 *
 *   - 槽（slot）：一个响应信号的普通函数。
 *     比如 "当按钮被点击时，关闭对话框"。
 *     槽就是一个普通 C++ 函数，只是能被信号触发。
 *
 *   - connect(发送者, 信号, 接收者, 槽)：
 *     把信号和槽"绑"在一起。
 *
 *     新式（我们用的）：
 *        connect(sender, &SenderClass::signalName,
 *                receiver, &ReceiverClass::slotName);
 *        （编译期类型检查，写错了编译报错，更安全）
 *
 *     用 lambda（匿名函数）做槽（我们主要用的）：
 *        connect(sender, &SenderClass::signalName, this, [this]() {
 *            // 信号触发时要执行的代码
 *        });
 *        灵活方便，不需要单独写槽函数。
 *
 * 【lambda 表达式是什么？】
 *   C++11 引入的"匿名函数"：
 *     [捕获列表](参数列表) { 函数体 }
 *
 *   [this]() { m_result = NewGame; accept(); }
 *     ↑         ↑
 *     |         └ 无参数
 *     └ 捕获 this（可以访问类的成员变量和函数）
 *
 *   为什么用 lambda？ 因为写起来比单独定义槽函数快。
 *   如果回调逻辑只有一两行，lambda 是最简洁的选择。
 */

#include "initialdialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

InitialDialog::InitialDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("中国象棋");
    /*
     * setFixedSize(width, height)：固定窗口大小。
     * 用户不能拖拽边缘改变尺寸，保证布局不变形。
     */
    setFixedSize(360, 400);

    /*
     * setStyleSheet —— QSS（Qt Style Sheets）
     * 语法和 CSS 几乎一样：选择器 { 属性: 值; }
     *
     * 常用选择器：
     *   QDialog     — 按类名选（所有 QDialog）
     *   QLabel#title — 按类名 + 对象名（id）选特定 Label
     *   QPushButton:hover — 鼠标悬停状态
     *
     * 这里定义了古风木色配色：
     *   背景 #eeddcc（米黄木色）
     *   文字 #5a3520（深褐）
     *   按钮 #8b7355（棕色）+ 悬停加深 #6b5345
     */
    setStyleSheet(
        "QDialog { background-color: #eeddcc; }"
        "QLabel#title { font: bold 28px '楷体'; color: #5a3520; padding: 20px; }"
        "QPushButton { font: bold 16px '楷体'; padding: 12px 40px; margin: 6px 30px;"
        "  background-color: #8b7355; color: #fff8f0; border: 2px solid #6b5345; border-radius: 6px; }"
        "QPushButton:hover { background-color: #6c5446; }"
    );

    /*
     * QVBoxLayout —— 垂直布局
     * 把控件从上到下排列。addStretch() 在底部加"弹簧"（可伸缩的空白），
     * 把按钮推到靠上的位置。
     */
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(8);  // 控件之间 8 像素间距

    auto *title = new QLabel("中 国 象 棋");
    title->setObjectName("title");       // 设定名称 = CSS 的 #id
    title->setAlignment(Qt::AlignCenter); // 水平居中
    layout->addSpacing(30);
    layout->addWidget(title);
    layout->addSpacing(20);

    // 四个按钮，文字故意加空格让它们视觉上更宽松
    auto *newBtn  = new QPushButton("新 建 游 戏");
    auto *loadBtn = new QPushButton("读 档 游 戏");
    auto *rankBtn = new QPushButton("排 行 榜");
    auto *quitBtn = new QPushButton("退 出");

    layout->addWidget(newBtn);
    layout->addWidget(loadBtn);
    layout->addWidget(rankBtn);
    layout->addWidget(quitBtn);
    layout->addStretch();  // 底部弹簧

    /*
     * 把四个按钮的 clicked 信号连接到 lambda 槽：
     *   点击 → 设置 m_result → accept() 关闭对话框
     *
     * accept() 是 QDialog 的方法：
     *   1. 关闭对话框
     *   2. 让 exec() 返回 QDialog::Accepted
     *   3. main.cpp 中的 if (exec() == Accepted) 就能检测到用户选择了什么
     *
     * 如果不调 accept()，对话框会一直显示着不关闭。
     */
    connect(newBtn,  &QPushButton::clicked, this, [this]() { m_result = NewGame;     accept(); });
    connect(loadBtn, &QPushButton::clicked, this, [this]() { m_result = LoadGame;    accept(); });
    connect(rankBtn, &QPushButton::clicked, this, [this]() { m_result = Leaderboard; accept(); });
    connect(quitBtn, &QPushButton::clicked, this, [this]() { m_result = Quit;        accept(); });
}
