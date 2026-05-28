#ifndef INITIALDIALOG_H
#define INITIALDIALOG_H

#include <QDialog>       // Qt 对话框基类（模态弹窗）

/*
 * InitialDialog —— 初始菜单
 * ------------------------------------
 * 游戏启动后显示的第一个界面，提供四个选项。
 *
 * QDialog 默认是"模态"的：弹出来后必须关闭才能操作其他窗口，
 * 这确保用户做出选择后才能继续。
 *
 * exec() 运行对话框，返回 QDialog::Accepted（点了某个按钮触发了 accept()）
 * 或 QDialog::Rejected（点 X 关闭窗口）。
 *
 * result() 返回用户具体选了哪个选项（枚举值）。
 */
class InitialDialog : public QDialog {
    Q_OBJECT
public:
    explicit InitialDialog(QWidget *parent = nullptr);

    // 四个菜单选项
    enum Result { NewGame, LoadGame, Leaderboard, Quit };
    Result result() const { return m_result; }

private:
    Result m_result = Quit;  // 默认退出（点 X 关闭 = 退出）
};

#endif
