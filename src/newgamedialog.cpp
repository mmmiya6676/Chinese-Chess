#include "newgamedialog.h"
#include <QPushButton>       // 按钮控件
#include <QVBoxLayout>       // 垂直布局
#include <QFormLayout>       // 表单布局（标签-输入框成对排列）
#include <QHBoxLayout>       // 水平布局
#include <QMessageBox>       // 消息提示弹窗

/*
 * NewGameDialog —— 新建游戏弹窗实现
 * ------------------------------------
 * 界面布局：
 *   ┌──────────────────┐
 *   │   红方: [______] │  ← QFormLayout 自动对齐标签和输入框
 *   │   黑方: [______] │
 *   │                  │
 *   │     [开始] [取消] │  ← 按钮右对齐
 *   └──────────────────┘
 *
 * QFormLayout：专门做"标签+输入框"的表单。
 *   addRow("标签:", widget) 自动添加一行，左边标签右边控件。
 *
 * trimmed()：QString 方法，去掉首尾空格。
 *   用户不小心打了空格不影响实际名字。
 */
NewGameDialog::NewGameDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("新建游戏");
    setFixedSize(320, 200);

    setStyleSheet(
        "QDialog { background-color: #eeddcc; }"
        "QLabel { font: 14px '楷体'; color: #5a3520; }"
        "QLineEdit { font: 14px '楷体'; padding: 4px; border: 2px solid #8b7355; border-radius: 4px; }"
        "QPushButton { font: 14px '楷体'; padding: 6px 20px;"
        "  background-color: #8b7355; color: white; border-radius: 4px; }"
        "QPushButton:hover { background-color: #6b5345; }"
    );

    auto *layout = new QVBoxLayout(this);

    // 表单布局：标签 + 输入框 成对排列
    auto *form = new QFormLayout;
    m_redEdit   = new QLineEdit;
    m_redEdit->setPlaceholderText("请输入红方名字");
    m_blackEdit = new QLineEdit;
    m_blackEdit->setPlaceholderText("请输入黑方名字");
    form->addRow("红方:", m_redEdit);
    form->addRow("黑方:", m_blackEdit);
    layout->addLayout(form);

    // 按钮行
    auto *btnLayout = new QHBoxLayout;
    auto *okBtn     = new QPushButton("开始游戏");
    auto *cancelBtn = new QPushButton("取消");
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    // "开始游戏"按钮：检查名字非空 → accept() 关闭对话框
    connect(okBtn, &QPushButton::clicked, this, [this]() {
        if (m_redEdit->text().trimmed().isEmpty() || m_blackEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "提示", "请输入双方名字");
            return;
        }
        accept();  // 关闭对话框，返回 Accepted
    });

    // "取消"按钮：reject() 关闭对话框，返回 Rejected
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

QString NewGameDialog::redName()   const { return m_redEdit->text().trimmed(); }
QString NewGameDialog::blackName() const { return m_blackEdit->text().trimmed(); }
