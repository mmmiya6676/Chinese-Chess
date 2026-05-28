#ifndef NEWGAMEDIALOG_H
#define NEWGAMEDIALOG_H

#include <QDialog>        // 对话框基类
#include <QLineEdit>      // 单行文本输入框（相当于网页的 input 框）
#include <QLabel>         // 文字标签

/*
 * NewGameDialog —— 新建游戏弹窗
 * ------------------------------------
 * 输入红方和黑方的玩家名。
 * redName() / blackName() 返回去掉首尾空格的输入内容。
 *
 * QLineEdit：Qt 的单行文本输入控件
 *   text() 方法获取当前输入的文字
 *   setPlaceholderText("请输入") 设置灰色占位提示文字
 */
class NewGameDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewGameDialog(QWidget *parent = nullptr);
    QString redName() const;      // 获取红方名字（去除首尾空格）
    QString blackName() const;    // 获取黑方名字
private:
    QLineEdit *m_redEdit;         // 红方名字输入框
    QLineEdit *m_blackEdit;       // 黑方名字输入框
};

#endif
