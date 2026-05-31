/*
 * leaderboarddialog.cpp —— 排行榜弹窗
 * ===================================
 *
 * 【QTableWidget 是什么？】
 *   Qt 的表格控件，类似 Excel 表格。常用方法：
 *
 *   setRowCount(n)     — 设置行数
 *   setColumnCount(n)  — 设置列数
 *   setHorizontalHeaderLabels({"列1","列2",...})  — 设置表头
 *   setItem(row, col, new QTableWidgetItem("文字"))  — 填充单元格
 *
 *   每个单元格是一个 QTableWidgetItem 对象。
 *
 * 【QHeaderView 是什么？】
 *   表头控件。horizontalHeader() 返回水平表头。
 *   setSectionResizeMode(QHeaderView::Stretch)：
 *     让所有列自动均分宽度填满表格，窗口缩放时自适应。
 */

#include "leaderboarddialog.h"
#include "SaveManager.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>

LeaderboardDialog::LeaderboardDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("排行榜");
    setFixedSize(500, 400);
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

    // 创建 4 列表格：排名 | 玩家 | 胜场/总场 | 胜率
    auto *table = new QTableWidget;
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"排名", "玩家", "胜场/总场", "胜率"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 双击不编辑
    table->setSelectionMode(QAbstractItemView::NoSelection);     // 点击不选中
    table->verticalHeader()->setVisible(false);                  // 隐藏行号

    // loadLeaderboardData() 从文件读取排行榜，已按胜率降序排列
    auto data = loadLeaderboardData();
    table->setRowCount(data.empty() ? 1 : (int)data.size());

    if (data.empty()) {
        table->setItem(0, 0, new QTableWidgetItem("-"));
        table->setItem(0, 1, new QTableWidgetItem("（暂无对局记录）"));
        table->setItem(0, 2, new QTableWidgetItem("-"));
        table->setItem(0, 3, new QTableWidgetItem("-"));
    } else {
        for (int i = 0; i < (int)data.size(); i++) {
            // 胜率 = 胜场 / 总场 * 100，+0.5 是为了四舍五入
            double rate = data[i].total > 0
                          ? 100.0 * data[i].wins / data[i].total : 0;

            table->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1) + "."));
            table->setItem(i, 1, new QTableWidgetItem(
                QString::fromStdString(data[i].name)));
            table->setItem(i, 2, new QTableWidgetItem(
                QString("%1 / %2").arg(data[i].wins).arg(data[i].total)));
            table->setItem(i, 3, new QTableWidgetItem(
                QString("%1%").arg((int)(rate + 0.5))));
        }
    }
    layout->addWidget(table);

    auto *closeBtn = new QPushButton("关闭");
    layout->addWidget(closeBtn, 0, Qt::AlignRight);     // 靠右对齐
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}
