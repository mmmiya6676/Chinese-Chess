#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QWidget>       // Qt 控件基类，每个界面组件都继承它
#include <QPoint>        // Qt 的点类，存放 x、y 两个整数坐标
#include <vector>        // C++ 标准库动态数组
#include "Game.h"        // 我们写好的游戏逻辑主类
#include "Position.h"    // 坐标模板类 Position<int>

/*
 * BoardWidget —— 棋盘控件
 * ------------------------------------
 * 这是整个程序的核心可视化组件，继承自 Qt 的 QWidget。
 * 它负责三件事：
 *   1. 用 QPainter 画棋盘网格、棋子、高亮提示
 *   2. 接收鼠标点击，转化为游戏走棋操作
 *   3. 发射信号通知 MainWindow“走了一步棋”或“游戏结束”
 *
 * Q_OBJECT 宏是 Qt 信号/槽机制必需的，
 * 只要类里用了 signals、slots 就必须写这个宏。
 */
class BoardWidget : public QWidget {
    Q_OBJECT                         // Qt 元对象系统宏，启用信号/槽
public:
    explicit BoardWidget(QWidget *parent = nullptr);

    // 设置/获取绑定的 Game 对象
    // inline 写法：setGame 调用 update() 立即触发重绘
    void setGame(Game* game) { m_game = game; update(); }
    Game* game() const { return m_game; }

    // ---------- 坐标转换 ----------
    // 棋盘逻辑坐标 (row, col)  → 屏幕上像素坐标
    QPoint boardToPixel(int row, int col) const;
    // 屏幕上像素坐标 → 棋盘逻辑坐标（用在鼠标点击时反算）
    Position<int> pixelToBoard(const QPoint& pt) const;

    // ---------- 选中状态 ----------
    Position<int> selectedPos() const { return m_selected; }
    void clearSelection();           // 清除选中，同时清掉合法走法提示
    const std::vector<Position<int>>& validMoves() const { return m_validMoves; }

signals:  // Qt 信号：只声明不实现，由 MOC 自动生成实现代码
    void moveMade();                 // 走了一步棋 → MainWindow 更新计时、步数
    void gameOverSignal();           // 游戏结束  → MainWindow 弹出结果

protected:
    /*
     * paintEvent 是 QWidget 的虚函数，每次需要重绘时被 Qt 自动调用。
     * （窗口首次显示、update() 被调用、窗口被遮挡后恢复等）
     *
     * mousePressEvent 是鼠标点击回调。
     */
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    // 把绘制拆成三个步骤，paintEvent 里依次调用
    void drawBoard(QPainter& p);       // 画网格、楚河汉界、九宫斜线
    void drawPieces(QPainter& p);      // 画棋子（圆圈 + 中文字符）
    void drawHighlights(QPainter& p);  // 画选中高亮 + 合法走法提示

    Game* m_game = nullptr;            // 关联的游戏逻辑对象
    Position<int> m_selected{-1, -1};  // 当前选中的棋子（-1,-1 = 未选中）
    std::vector<Position<int>> m_validMoves;  // 选中棋子可以走的位置列表

    // 布局常量（像素），修改这里就能整体缩放棋盘
    static constexpr int MARGIN  = 50;   // 棋盘边距
    static constexpr int CELL    = 60;   // 格子边长
    static constexpr int PIECE_R = 26;   // 棋子圆圈半径
    static constexpr int COLS    = 9;    // 棋盘列数
    static constexpr int ROWS    = 10;   // 棋盘行数
};

#endif // BOARDWIDGET_H
