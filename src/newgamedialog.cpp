#include "newgamedialog.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>

NewGameDialog::NewGameDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("新建游戏");
    setFixedSize(340, 260);

    setStyleSheet(
        "QDialog { background-color: #eeddcc; }"
        "QLabel { font: 14px '楷体'; color: black; }"
        "QLineEdit { font: 14px '楷体'; color: black; padding: 4px; border: 2px solid #8b7355; border-radius: 4px; }"
        "QCheckBox { font: 14px '楷体'; color: black; }"
        "QComboBox { font: 14px '楷体'; color: black; padding: 4px; border: 2px solid #8b7355; border-radius: 4px; background: #fff8f0; }"
        "QComboBox QAbstractItemView { color: black; selection-background-color: #8b7355; }"
        "QPushButton { font: 14px '楷体'; padding: 6px 20px;"
        "  background-color: #8b7355; color: white; border-radius: 4px; }"
        "QPushButton:hover { background-color: #6b5345; }"
    );

    auto *layout = new QVBoxLayout(this);

    // 名字输入
    auto *form = new QFormLayout;
    m_redEdit   = new QLineEdit;
    m_redEdit->setPlaceholderText("请输入红方名字");
    m_blackEdit = new QLineEdit;
    m_blackEdit->setPlaceholderText("请输入黑方名字");
    form->addRow("红方:", m_redEdit);
    form->addRow("黑方:", m_blackEdit);
    layout->addLayout(form);

    // AI 模式
    m_aiCheck = new QCheckBox("人机对战（AI 执黑）");
    layout->addWidget(m_aiCheck);

    // 难度
    auto *diffRow = new QHBoxLayout;
    diffRow->addWidget(new QLabel("难度:"));
    m_diffCombo = new QComboBox;
    m_diffCombo->addItem("初级");
    m_diffCombo->addItem("中级");
    m_diffCombo->addItem("高级");
    m_diffCombo->setCurrentIndex(1);
    diffRow->addWidget(m_diffCombo);
    diffRow->addStretch();
    layout->addLayout(diffRow);

    layout->addSpacing(8);

    // 按钮
    auto *btnLayout = new QHBoxLayout;
    auto *okBtn     = new QPushButton("开始游戏");
    auto *cancelBtn = new QPushButton("取消");
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    // AI 勾选时：隐藏黑方输入框（AI 不需要名字），红方也只需一个代号
    connect(m_aiCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_blackEdit->setVisible(!checked);
        if (checked) {
            m_redEdit->setPlaceholderText("请输入你的名字");
        } else {
            m_redEdit->setPlaceholderText("请输入红方名字");
            m_blackEdit->setPlaceholderText("请输入黑方名字");
        }
    });

    connect(okBtn, &QPushButton::clicked, this, [this]() {
        bool ai = m_aiCheck->isChecked();
        if (m_redEdit->text().trimmed().isEmpty() ||
            (!ai && m_blackEdit->text().trimmed().isEmpty())) {
            QMessageBox::warning(this, "提示", ai ? "请输入你的名字" : "请输入双方名字");
            return;
        }
        accept();
    });

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

QString NewGameDialog::redName() const {
    return m_redEdit->text().trimmed().isEmpty() ? "玩家" : m_redEdit->text().trimmed();
}
QString NewGameDialog::blackName() const {
    if (m_aiCheck->isChecked())
        return "电脑 (AI)";
    return m_blackEdit->text().trimmed().isEmpty() ? "玩家2" : m_blackEdit->text().trimmed();
}
bool NewGameDialog::isAIMode() const { return m_aiCheck->isChecked(); }
AIDifficulty NewGameDialog::aiDifficulty() const {
    switch (m_diffCombo->currentIndex()) {
        case 0: return AIDifficulty::Easy;
        case 2: return AIDifficulty::Hard;
        default: return AIDifficulty::Medium;
    }
}
