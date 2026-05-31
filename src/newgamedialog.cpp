/*
 * newgamedialog.cpp —— 新建游戏弹窗
 * =================================
 *
 * 【使用的 Qt 控件一览】
 *
 *   QLineEdit：单行文本输入框
 *     text() 获取输入内容
 *     setPlaceholderText("灰色提示") 设置占位提示文字
 *     setText("默认值") 设置默认文字
 *
 *   QCheckBox：复选框（勾选框）
 *     isChecked() 判断是否勾选
 *     toggled(bool) 信号：勾选状态改变时触发
 *
 *   QComboBox：下拉选择框
 *     addItem("选项文字") 添加选项
 *     currentIndex() 返回当前选中的序号（0 开始）
 *     setCurrentIndex(n) 设置默认选中项
 *
 *   QFormLayout：表单布局
 *     自动把"标签"和"输入控件"成对排列。
 *     addRow("标签:", widget) 添加一行，左边标签右边控件。
 *     比手写 QHBoxLayout 对齐要方便得多。
 *
 *   QMessageBox：消息弹窗
 *     QMessageBox::warning(this, "标题", "内容") — 警告弹窗
 *     QMessageBox::information(this, "标题", "内容") — 信息弹窗
 */

#include "newgamedialog.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>

NewGameDialog::NewGameDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("新建游戏");
    setFixedSize(340, 260);

    /*
     * QSS 样式表。本次特别设定了：
     *   QCheckBox、QLabel、QLineEdit、QComboBox 的文字都为黑色，
     *   解决浅色背景上文字看不清的问题。
     *
     *   QComboBox QAbstractItemView：这是下拉列表的内部控件，
     *   必须用这个选择器才能设置下拉项的样式。
     */
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

    // ---- 名字输入区（QFormLayout：标签+输入框自动对齐）----
    auto *form = new QFormLayout;
    m_redEdit   = new QLineEdit;
    m_redEdit->setPlaceholderText("请输入红方名字");
    m_blackEdit = new QLineEdit;
    m_blackEdit->setPlaceholderText("请输入黑方名字");
    form->addRow("红方:", m_redEdit);
    form->addRow("黑方:", m_blackEdit);
    layout->addLayout(form);

    // ---- AI 模式开关 ----
    m_aiCheck = new QCheckBox("人机对战（AI 执黑）");
    layout->addWidget(m_aiCheck);

    // ---- 难度选择 ----
    auto *diffRow = new QHBoxLayout;
    diffRow->addWidget(new QLabel("难度:"));
    m_diffCombo = new QComboBox;
    m_diffCombo->addItem("初级");
    m_diffCombo->addItem("中级");
    m_diffCombo->addItem("高级");
    m_diffCombo->setCurrentIndex(1);  // 默认选中"中级"（索引1）
    diffRow->addWidget(m_diffCombo);
    diffRow->addStretch();
    layout->addLayout(diffRow);

    layout->addSpacing(8);

    // ---- 按钮 ----
    auto *btnLayout = new QHBoxLayout;
    auto *okBtn     = new QPushButton("开始游戏");
    auto *cancelBtn = new QPushButton("取消");
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    /*
     * AI 勾选状态变化时：
     *   勾了 → 隐藏黑方输入框（AI 不需要你给他取名）
     *   不勾 → 恢复黑方输入框
     */
    connect(m_aiCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_blackEdit->setVisible(!checked);
        if (checked) {
            m_redEdit->setPlaceholderText("请输入你的名字");
        } else {
            m_redEdit->setPlaceholderText("请输入红方名字");
            m_blackEdit->setPlaceholderText("请输入黑方名字");
        }
    });

    // "开始游戏"按钮：校验输入 → accept()
    connect(okBtn, &QPushButton::clicked, this, [this]() {
        bool ai = m_aiCheck->isChecked();
        // AI 模式只需要红方名，双人模式都需要
        if (m_redEdit->text().trimmed().isEmpty() ||
            (!ai && m_blackEdit->text().trimmed().isEmpty())) {
            QMessageBox::warning(this, "提示", ai ? "请输入你的名字" : "请输入双方名字");
            return;  // 不关闭对话框，让用户重新输入
        }
        accept();   // 校验通过，关闭对话框
    });

    // "取消"按钮：reject() → exec() 返回 Rejected
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

QString NewGameDialog::redName() const {
    // 如果用户没输入，给个默认名
    return m_redEdit->text().trimmed().isEmpty() ? "玩家" : m_redEdit->text().trimmed();
}
QString NewGameDialog::blackName() const {
    if (m_aiCheck->isChecked())
        return "电脑 (AI)";   // AI 模式下自动命名
    return m_blackEdit->text().trimmed().isEmpty() ? "玩家2" : m_blackEdit->text().trimmed();
}
bool NewGameDialog::isAIMode() const { return m_aiCheck->isChecked(); }

AIDifficulty NewGameDialog::aiDifficulty() const {
    // QComboBox 的 currentIndex 返回选中项的序号
    switch (m_diffCombo->currentIndex()) {
        case 0: return AIDifficulty::Easy;
        case 2: return AIDifficulty::Hard;
        default: return AIDifficulty::Medium;
    }
}
