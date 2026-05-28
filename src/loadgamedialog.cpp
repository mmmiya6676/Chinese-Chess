#include "loadgamedialog.h"
#include "SaveManager.h"    // listSaveFiles() 等存档管理函数
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>       // 按钮控件
#include <QLabel>

/*
 * LoadGameDialog —— 读档选择弹窗实现
 * ------------------------------------
 * 界面：
 *   ┌──────────────────────┐
 *   │  === 存档列表 ===    │
 *   │  ┌────────────────┐  │
 *   │  │ 1_vs_2_play1   │  │  ← QListWidget 列表
 *   │  │ 1_vs_2_play2   │  │  ← 双击直接读档
 *   │  │ ...            │  │
 *   │  └────────────────┘  │
 *   │       [读档] [取消]   │
 *   └──────────────────────┘
 *
 * m_files 和 m_list 的关系：
 *   m_files[0] = "1_vs_2_play1.txt"   ←→ m_list->item(0) 显示 "1_vs_2_play1"
 *   m_files[1] = "1_vs_2_play2.txt"   ←→ m_list->item(1) 显示 "1_vs_2_play2"
 *
 *   显示时去掉 .txt 后缀更清爽。
 *   选择时用 currentRow() 作为索引，从 m_files 取出完整文件名。
 */
LoadGameDialog::LoadGameDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("读档游戏");
    setFixedSize(420, 350);
    setStyleSheet(
        "QDialog { background-color: #eeddcc; }"
        "QLabel { font: 16px '楷体'; color: #5a3520; }"
        "QListWidget { font: 14px '楷体'; color: black;"
        "  background: #fff8f0; border: 2px solid #8b7355; }"
        "QPushButton { font: 14px '楷体'; padding: 6px 20px;"
        "  background-color: #8b7355; color: white; border-radius: 4px; }"
    );

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("=== 存档列表 ==="));

    // 创建列表控件
    m_list = new QListWidget;
    layout->addWidget(m_list);

    // listSaveFiles() 扫描 saves/ 目录，返回文件名列表
    m_files = listSaveFiles();
    for (const auto& f : m_files) {
        // 去掉 ".txt" 后缀再显示（列表里看起来更整洁）
        std::string display = f;
        if (display.size() > 4 && display.substr(display.size() - 4) == ".txt")
            display = display.substr(0, display.size() - 4);
        m_list->addItem(QString::fromStdString(display));
    }
    if (m_files.empty())
        m_list->addItem("（暂无存档文件）");

    // 按钮
    auto *loadBtn   = new QPushButton("读档");
    auto *cancelBtn = new QPushButton("取消");
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(loadBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    // "读档"按钮 → accept() → exec() 返回 Accepted
    connect(loadBtn,   &QPushButton::clicked, this, &QDialog::accept);
    // "取消"按钮 → reject() → exec() 返回 Rejected
    connect(cancelBtn, &QPushButton::clicked, this, [this]() { reject(); });
    // 双击列表项也能读档
    connect(m_list, &QListWidget::doubleClicked, this, &QDialog::accept);
}

// 返回用户选择的完整文件名（含 .txt），未选返回空字符串
std::string LoadGameDialog::selectedFile() const {
    int row = m_list->currentRow();   // 返回当前选中的行号，未选返回 -1
    if (row < 0 || row >= (int)m_files.size()) return "";
    return m_files[row];
}
