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
     * 返回排序后的文件名列表。
     *
     * 文件名格式：1_vs_2_play1.txt（红方ID_vs_黑方ID_play编号.txt）
     * 显示时：
     *   1. 去掉 ".txt" 后缀
     *   2. 用玩家注册表把数字 ID 翻译成中文名
     *   例如：1_vs_2_play1 → 张三 vs 李四 play1
     */
    m_files = listSaveFiles();

    // 读取玩家注册表（ID → 名字映射）
    auto reg = loadPlayerRegistry();

    for (const auto& f : m_files) {
        std::string display = f;
        // 去掉 ".txt" 后缀
        if (display.size() > 4 && display.substr(display.size() - 4) == ".txt")
            display = display.substr(0, display.size() - 4);

        // 尝试解析 "1_vs_2_play1" 格式 → "张三 vs 李四 play1"
        size_t vsPos   = display.find("_vs_");
        size_t playPos = display.find("_play");
        if (vsPos != std::string::npos && playPos != std::string::npos) {
            try {
                int rid = stoi(display.substr(0, vsPos));
                int bid = stoi(display.substr(vsPos + 4, playPos - vsPos - 4));
                std::string rn = reg.count(rid) ? reg[rid] : ("玩家" + std::to_string(rid));
                std::string bn = reg.count(bid) ? reg[bid] : ("玩家" + std::to_string(bid));
                // 格式：张三 vs 李四 play1
                display = rn + " vs " + bn + " " + display.substr(playPos + 1);
            } catch (...) {
                // 解析失败就用原始显示名
            }
        }

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
