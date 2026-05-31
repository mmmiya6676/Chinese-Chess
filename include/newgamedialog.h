#ifndef NEWGAMEDIALOG_H
#define NEWGAMEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include "ChessAI.h"

/*
 * NewGameDialog —— 新建游戏弹窗
 * ------------------------------------
 * 输入红方和黑方的玩家名，可选 AI 模式。
 *
 * AI 模式：勾选后黑方由 AI 控制，可选择难度（初级/中级/高级）。
 */
class NewGameDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewGameDialog(QWidget *parent = nullptr);
    QString redName() const;
    QString blackName() const;
    bool    isAIMode() const;
    AIDifficulty aiDifficulty() const;

private:
    QLineEdit *m_redEdit;
    QLineEdit *m_blackEdit;
    QCheckBox *m_aiCheck;
    QComboBox *m_diffCombo;
};

#endif
