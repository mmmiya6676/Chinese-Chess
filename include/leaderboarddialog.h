#ifndef LEADERBOARDDIALOG_H
#define LEADERBOARDDIALOG_H

#include <QDialog>        // 对话框基类
#include <QTableWidget>   // 表格控件（类似 Excel 那样）

/*
 * LeaderboardDialog —— 排行榜弹窗
 * ------------------------------------
 * 用 QTableWidget 显示排行榜数据：排名、玩家名、胜场/总场、胜率
 *
 * QTableWidget：Qt 的表格控件
 *   行 = row，列 = column
 *   setRowCount(n) 设置行数
 *   setColumnCount(n) 设置列数
 *   setHorizontalHeaderLabels({"列1", "列2", ...}) 设置表头
 *   setItem(row, col, new QTableWidgetItem("文字")) 设置单元格内容
 */
class LeaderboardDialog : public QDialog {
    Q_OBJECT
public:
    explicit LeaderboardDialog(QWidget *parent = nullptr);
};

#endif
