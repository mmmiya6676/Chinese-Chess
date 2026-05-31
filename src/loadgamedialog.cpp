/*
 * loadgamedialog.cpp —— 读档选择弹窗
 * ===================================
 *
 * 【QListWidget 是什么？】
 *   列表控件，像一个文件列表。常用方法：
 *     addItem("文字")   — 添加一项
 *     count()           — 总共有几项
 *     currentRow()      — 当前选中第几行（0 开始，未选 = -1）
 *     doubleClicked 信号 — 双击某行时触发
 */

#include "loadgamedialog.h"
#include "SaveManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

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

    m_list = new QListWidget;
    layout->addWidget(m_list);

    /*
     * listSaveFiles() 扫描 saves/ 目录下所有 .txt 文件，
     * 返回排序后的文件名列表（不含路径前缀）。
     *
     * 显示时去掉 ".txt" 后缀让列表更干净。
     * m_files 和 m_list 通过行号（index）对应：
     *   m_files[0] = "1_vs_2_play1.txt"
     *   m_list 第0行显示 "1_vs_2_play1"
     */
    m_files = listSaveFiles();
    for (const auto& f : m_files) {
        std::string display = f;
        if (display.size() > 4 && display.substr(display.size() - 4) == ".txt")
            display = display.substr(0, display.size() - 4);
        m_list->addItem(QString::fromStdString(display));
    }
    if (m_files.empty())
        m_list->addItem("（暂无存档文件）");

    auto *loadBtn   = new QPushButton("读档");
    auto *cancelBtn = new QPushButton("取消");
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(loadBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    // accept() → exec() 返回 Accepted → main.cpp 继续处理
    connect(loadBtn,   &QPushButton::clicked, this, &QDialog::accept);
    // reject() → exec() 返回 Rejected → main.cpp 不处理
    connect(cancelBtn, &QPushButton::clicked, this, [this]() { reject(); });
    // 双击列表项也等同于点"读档"
    connect(m_list, &QListWidget::doubleClicked, this, &QDialog::accept);
}

std::string LoadGameDialog::selectedFile() const {
    int row = m_list->currentRow();
    if (row < 0 || row >= (int)m_files.size()) return "";
    return m_files[row];
}
