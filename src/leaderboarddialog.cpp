#include "leaderboarddialog.h"
#include "SaveManager.h"       // loadLeaderboardData() 返回排行榜数据
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>         // 表头控件（设置列宽模式等）

/*
 * LeaderboardDialog —— 排行榜弹窗实现
 * ------------------------------------
 * 界面：
 *   ┌──────────────────────────────────┐
 *   │      ===== 排行榜 =====          │
 *   │  ┌────┬──────────┬──────┬──────┐ │
 *   │  │排名│   玩家   │ 胜负 │ 胜率 │ │  ← QTableWidget
 *   │  ├────┼──────────┼──────┼──────┤ │
 *   │  │ 1. │ 张三     │ 5/10 │ 50%  │ │
 *   │  │ 2. │ 李四     │ 3/8  │ 38%  │ │
 *   │  └────┴──────────┴──────┴──────┘ │
 *   │                       [关闭]     │
 *   └──────────────────────────────────┘
 *
 * QHeaderView::Stretch：让列宽自动均分填满表格宽度。
 * setEditTriggers(NoEditTriggers)：禁止编辑单元格。
 * setSelectionMode(NoSelection)：禁止选中单元格。
 */
LeaderboardDialog::LeaderboardDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("排行榜");
    setFixedSize(500, 400);

    // QSS 样式：木色背景 + 楷体字 + 深色表头
    setStyleSheet(
        "QDialog { background-color: #eeddcc; }"
        "QLabel { font: bold 18px '楷体'; color: #5a3520; }"
        "QTableWidget { font: 14px '楷体'; color: black;"
        "  background: #fff8f0; border: 2px solid #8b7355; gridline-color: #d4b896; }"
        "QHeaderView::section { font: bold 14px '楷体';"
        "  background: #8b7355; color: white; padding: 4px; }"
        "QPushButton { font: 14px '楷体'; padding: 6px 20px;"
        "  background-color: #8b7355; color: white; border-radius: 4px; }"
    );

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("===== 排行榜 ====="));
    layout->addSpacing(8);

    // 创建表格：4 列 = {排名, 玩家, 胜场/总场, 胜率}
    auto *table = new QTableWidget;
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"排名", "玩家", "胜场/总场", "胜率"});

    /*
     * QHeaderView::Stretch：让所有列自动均分宽度填满表格，
     * 即使窗口大小改变也会自适应。
     */
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);     // 禁止双击编辑
    table->setSelectionMode(QAbstractItemView::NoSelection);       // 禁止选中
    table->verticalHeader()->setVisible(false);                    // 隐藏行号

    // loadLeaderboardData() 从文件读取排行榜，已按胜率排序
    auto data = loadLeaderboardData();
    table->setRowCount(data.empty() ? 1 : (int)data.size());

    if (data.empty()) {
        // 没有记录时显示提示
        table->setItem(0, 0, new QTableWidgetItem("-"));
        table->setItem(0, 1, new QTableWidgetItem("（暂无对局记录）"));
        table->setItem(0, 2, new QTableWidgetItem("-"));
        table->setItem(0, 3, new QTableWidgetItem("-"));
    } else {
        // 填充每一行数据
        for (int i = 0; i < (int)data.size(); i++) {
            double rate = data[i].total > 0
                          ? 100.0 * data[i].wins / data[i].total : 0;

            table->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1) + "."));
            table->setItem(i, 1, new QTableWidgetItem(
                QString::fromStdString(data[i].name)));
            table->setItem(i, 2, new QTableWidgetItem(
                QString("%1 / %2").arg(data[i].wins).arg(data[i].total)));
            table->setItem(i, 3, new QTableWidgetItem(
                QString("%1%").arg((int)(rate + 0.5))));   // +0.5 四舍五入
        }
    }
    layout->addWidget(table);

    auto *closeBtn = new QPushButton("关闭");
    layout->addWidget(closeBtn, 0, Qt::AlignRight);     // 右对齐
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}
