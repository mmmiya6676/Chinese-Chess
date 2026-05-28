#ifndef LOADGAMEDIALOG_H
#define LOADGAMEDIALOG_H

#include <QDialog>         // 对话框基类
#include <QListWidget>     // 列表控件（像文件管理器里选文件那样）
#include <vector>
#include <string>

/*
 * LoadGameDialog —— 读档选择弹窗
 * ------------------------------------
 * 列出 saves/ 目录下所有 .txt 存档文件，用户选择一个加载。
 *
 * selectedFile() 返回用户选择的文件名（不含 "saves/" 前缀），
 * 如果取消或没有存档则返回空字符串。
 *
 * QListWidget：Qt 的列表控件
 *   addItem("文字") 添加一项
 *   currentRow() 返回当前选中的行号（从 0 开始）
 *   doubleClicked 信号：双击某一项时触发
 */
class LoadGameDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoadGameDialog(QWidget *parent = nullptr);
    std::string selectedFile() const;   // 返回选中文件名，未选返回空
private:
    QListWidget *m_list;                // 文件列表控件
    std::vector<std::string> m_files;   // 保存的文件名列表（和 m_list 的行号对应）
};

#endif
