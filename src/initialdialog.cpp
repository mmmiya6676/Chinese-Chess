#include "initialdialog.h"
#include <QVBoxLayout>    // 垂直布局
#include <QPushButton>    // 按钮
#include <QLabel>         // 文字标签

/*
 * InitialDialog —— 初始菜单实现
 * ------------------------------------
 * 界面：
 *   ┌──────────────────┐
 *   │                  │
 *   │   中 国 象 棋    │  ← 大号楷体标题
 *   │                  │
 *   │  ┌────────────┐  │
 *   │  │ 新 建 游 戏 │  │  ← 点击 → 设置 m_result=NewGame，关闭对话框
 *   │  └────────────┘  │
 *   │  ┌────────────┐  │
 *   │  │ 读 档 游 戏 │  │
 *   │  └────────────┘  │
 *   │  ┌────────────┐  │
 *   │  │  排 行 榜  │  │
 *   │  └────────────┘  │
 *   │  ┌────────────┐  │
 *   │  │   退  出   │  │
 *   │  └────────────┘  │
 *   └──────────────────┘
 *
 * connect 用了 lambda 表达式（C++11 特性）：
 *   [this]() { ... } 意思是"捕获 this 指针的匿名函数"。
 *   点击按钮 → lambda 执行 → 设置 m_result → 调用 accept() 关闭对话框。
 *
 *   accept()：QDialog 的方法，关闭对话框并返回 QDialog::Accepted。
 */
InitialDialog::InitialDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("中国象棋");
    setFixedSize(360, 400);    // 固定尺寸，不能拉伸

    // QSS 样式：木色背景 + 楷体字 + 圆角按钮
    setStyleSheet(
        "QDialog { background-color: #eeddcc; }"
        "QLabel#title { font: bold 28px '楷体'; color: #5a3520; padding: 20px; }"
        "QPushButton { font: bold 16px '楷体'; padding: 12px 40px; margin: 6px 30px;"
        "  background-color: #8b7355; color: #fff8f0; border: 2px solid #6b5345; border-radius: 6px; }"
        "QPushButton:hover { background-color: #6b5345; }"
    );

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(8);  // 控件间距

    // 标题
    auto *title = new QLabel("中 国 象 棋");
    title->setObjectName("title");       // 给 QSS 选择器用的名称
    title->setAlignment(Qt::AlignCenter);
    layout->addSpacing(30);
    layout->addWidget(title);
    layout->addSpacing(20);

    // 四个按钮
    auto *newBtn  = new QPushButton("新 建 游 戏");
    auto *loadBtn = new QPushButton("读 档 游 戏");
    auto *rankBtn = new QPushButton("排 行 榜");
    auto *quitBtn = new QPushButton("退 出");

    layout->addWidget(newBtn);
    layout->addWidget(loadBtn);
    layout->addWidget(rankBtn);
    layout->addWidget(quitBtn);
    layout->addStretch();  // 底部弹簧，把按钮往上推

    // 连接按钮点击事件 → lambda 设置结果 + 关闭对话框
    connect(newBtn,  &QPushButton::clicked, this, [this]() { m_result = NewGame;     accept(); });
    connect(loadBtn, &QPushButton::clicked, this, [this]() { m_result = LoadGame;    accept(); });
    connect(rankBtn, &QPushButton::clicked, this, [this]() { m_result = Leaderboard; accept(); });
    connect(quitBtn, &QPushButton::clicked, this, [this]() { m_result = Quit;        accept(); });
    // accept() → QDialog::exec() 返回 QDialog::Accepted → main.cpp 检查 result() 执行对应操作
}
