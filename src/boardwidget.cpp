#include "boardwidget.h"
#include <QPainter>       // Qt 绘图引擎，所有画线/画圆/写文字都靠它
#include <QMouseEvent>    // 鼠标事件类，event->pos() 获取点击坐标
#include <QFont>          // 字体描述对象
#include <QPen>           // 画笔（控制线条颜色、粗细）

// =========================== 构造函数 ===========================

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent)     // 必须调用父类 QWidget 的构造函数
{
    /*
     * setMinimumSize：设置控件的最小尺寸，布局系统不会把它缩得比这个还小。
     * 这里用棋盘常量计算出像素尺寸：
     *   宽 = 左右边距 + 8 个格子间距
     *   高 = 上下边距 + 9 个格子间距
     */
    setMinimumSize(MARGIN * 2 + CELL * (COLS - 1) + 20,
                   MARGIN * 2 + CELL * (ROWS - 1) + 20);

    // setMouseTracking(false): 默认关闭鼠标追踪，只有按下时才接收移动事件，节省性能
    setMouseTracking(false);
}

// =========================== 坐标转换 ===========================

/*
 * boardToPixel —— 棋盘坐标 → 屏幕像素坐标
 * 棋盘坐标体系的原点 (0,0) 是左上角第一个交叉点。
 * 像素坐标换算：x = 边距 + 列号 × 格子边长
 *              y = 边距 + 行号 × 格子边长
 */
QPoint BoardWidget::boardToPixel(int row, int col) const {
    int x = MARGIN + col * CELL;
    int y = MARGIN + row * CELL;
    return QPoint(x, y);
}

/*
 * pixelToBoard —— 屏幕像素坐标 → 棋盘坐标
 * 鼠标点击时 event->pos() 给出像素坐标，用这个函数反算出是哪一格。
 *
 * CELL / 2.0 是半格偏移，让点击"格子中心附近"都算点中那格，提高容错。
 * 返回 {-1, -1} 表示点击在棋盘之外。
 */
Position<int> BoardWidget::pixelToBoard(const QPoint& pt) const {
    int col = (pt.x() - MARGIN + CELL / 2.0) / CELL;
    int row = (pt.y() - MARGIN + CELL / 2.0) / CELL;
    if (col < 0 || col >= COLS || row < 0 || row >= ROWS)
        return {-1, -1};
    return {row, col};
}

/*
 * clearSelection —— 清除当前选中的棋子
 * 把 m_selected 设为无效值，清空合法走法列表，
 * 然后调用 update() 触发重绘（高亮会消失）。
 */
void BoardWidget::clearSelection() {
    m_selected = {-1, -1};
    m_validMoves.clear();
    update();
}

// =========================== 绘制 ===========================

/*
 * paintEvent —— Qt 核心绘制函数（QWidget 虚函数）
 * ------------------------------------------------
 * Qt 的绘制是"被动"的——你不是主动画，而是等着 Qt 来叫你画。
 * 以下情况会触发 paintEvent：
 *   1. 窗口第一次显示
 *   2. 调用了 update()
 *   3. 窗口从最小化恢复 / 被其他窗口遮住后露出来
 *
 * QPainter 是 Qt 的绘图引擎，所有绘制操作必须通过它。
 * 这里把绘制分成三步：棋盘格线 → 高亮提示 → 棋子。
 * 绘制顺序决定了图层：先画的东西会被后画的盖住。
 */
void BoardWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);    // QPainter 必须在 paintEvent 内创建（它依赖当前窗口的绘制上下文）

    /*
     * setRenderHint：开启抗锯齿（Anti-Aliasing），让线条和文字边缘更平滑。
     * Antialiasing = true 时，Qt 会在边缘多画几个半透明像素。
     */
    p.setRenderHint(QPainter::Antialiasing, true);

    // 整个控件填充木色背景
    p.fillRect(rect(), QColor(238, 207, 161));

    drawBoard(p);        // 第一步：画棋盘网格
    drawHighlights(p);   // 第二步：画选中高亮、合法走法提示
    drawPieces(p);       // 第三步：画棋子（最后画，保证覆盖在上面）
}

/*
 * drawBoard —— 画棋盘网格
 * ------------------------------------------------
 * 中国象棋棋盘的特点：
 *   1. 9 列 × 10 行的矩阵
 *   2. 中间有"楚河 汉界"隔断竖线
 *   3. 上下各有九宫格（用交叉斜线标记）
 *   4. 左右边界竖线和上下边界横线是连续的
 *
 * QPen   —— 画笔，控制线条的颜色和粗细
 * QFont  —— 字体，"楷体"是一种书法字体，适合中国风
 */
void BoardWidget::drawBoard(QPainter& p) {
    QPen gridPen(QColor(80, 50, 20), 1.5);    // 网格线：深褐色，1.5 像素粗
    QPen borderPen(QColor(60, 30, 10), 3);     // 外框线：更深更粗

    // ---- 外框 ----
    p.setPen(borderPen);
    p.drawRect(MARGIN - 8, MARGIN - 8,
               CELL * (COLS - 1) + 16, CELL * (ROWS - 1) + 16);

    // ---- 横线（10 条） ----
    p.setPen(gridPen);
    for (int r = 0; r < ROWS; r++) {
        QPoint left  = boardToPixel(r, 0);
        QPoint right = boardToPixel(r, COLS - 1);
        p.drawLine(left.x(), left.y(), right.x(), right.y());
    }

    // ---- 竖线（9 条） ----
    // 中间 5 条竖线在河界处断开（第 4 行和第 5 行之间）
    for (int c = 0; c < COLS; c++) {
        if (c == 0 || c == COLS - 1) {
            // 左右边界：通长竖线
            QPoint top = boardToPixel(0, c);
            QPoint bot = boardToPixel(ROWS - 1, c);
            p.drawLine(top.x(), top.y(), bot.x(), bot.y());
        } else {
            // 其他：上半段 + 下半段，中间（河）断开
            QPoint top1 = boardToPixel(0, c),  top2 = boardToPixel(4, c);
            p.drawLine(top1.x(), top1.y(), top2.x(), top2.y());
            QPoint bot1 = boardToPixel(5, c),  bot2 = boardToPixel(ROWS - 1, c);
            p.drawLine(bot1.x(), bot1.y(), bot2.x(), bot2.y());
        }
    }

    // ---- 九宫格交叉斜线 ----
    // 用 lambda 避免重复代码
    QPen dashPen(QColor(80, 50, 20), 1.2);
    p.setPen(dashPen);
    auto drawPalace = [&](int r1, int r2) {
        QPoint tl = boardToPixel(r1, 3), tr = boardToPixel(r1, 5);
        QPoint bl = boardToPixel(r2, 3), br = boardToPixel(r2, 5);
        p.drawLine(tl.x(), tl.y(), br.x(), br.y());
        p.drawLine(tr.x(), tr.y(), bl.x(), bl.y());
    };
    drawPalace(0, 2);    // 黑方九宫（上方）
    drawPalace(7, 9);    // 红方九宫（下方）

    // ---- 楚河 汉界 ----
    p.setFont(QFont("楷体", 22, QFont::Bold));
    p.setPen(QColor(60, 30, 10));
    QPoint riverL = boardToPixel(4, 0);
    QPoint riverR = boardToPixel(5, COLS - 1);
    int riverY = (riverL.y() + riverR.y()) / 2;   // 河界中线 Y 坐标
    int cx = MARGIN + CELL * (COLS - 1) / 2;       // 棋盘水平中点
    p.drawText(cx - CELL * 2, riverY - 4, "楚  河");  // 左半写"楚河"
    p.drawText(cx + CELL * 1, riverY - 4, "汉  界");  // 右半写"汉界"
}

/*
 * drawHighlights —— 画选中高亮和合法走法提示
 * --------------------------------------------------
 * 选中棋子：黄色光圈（像荧光笔圈起来）
 * 合法走法-空位：绿色小圆点（提示可以走到这里）
 * 合法走法-吃子：红色圈（提示可以吃掉这个棋子）
 *
 * QPen   = 边框(颜色+粗细)，QPen(Qt::NoPen) 表示不要边框
 * QBrush = 填充(颜色+透明度)，alpha=60 表示很透明
 */
void BoardWidget::drawHighlights(QPainter& p) {
    if (m_selected.getX() < 0) return;   // 没选中棋子，不画高亮

    // 选中的棋子 — 黄色半透明圈
    QPoint sel = boardToPixel(m_selected.getX(), m_selected.getY());
    p.setPen(QPen(QColor(255, 215, 0), 3));       // 金色边框
    p.setBrush(QColor(255, 255, 0, 60));           // 黄色半透明填充
    p.drawEllipse(QPointF(sel), PIECE_R + 3, PIECE_R + 3);

    // 合法走法列表
    for (const auto& mv : m_validMoves) {
        QPoint pt = boardToPixel(mv.getX(), mv.getY());
        ChessPiece* piece = m_game->getBoard().getPieceAt(mv);

        if (piece) {
            // 该位置有敌方棋子（可以吃）— 红色圈
            p.setPen(QPen(QColor(220, 50, 50), 3));
            p.setBrush(QColor(255, 100, 100, 60));
        } else {
            // 该位置为空 — 绿色小圆点
            p.setPen(Qt::NoPen);                    // 不要边框
            p.setBrush(QColor(0, 180, 0, 120));
        }
        // 吃子画大圈，空位画小圆点
        p.drawEllipse(QPointF(pt),
                      piece ? PIECE_R + 3 : 8,
                      piece ? PIECE_R + 3 : 8);
    }
}

/*
 * drawPieces —— 画棋子
 * ------------------------------------------------
 * 每个棋子是一个圆形：
 *   1. 用 QRadialGradient 做径向渐变（中心亮→边缘暗，模拟立体感）
 *   2. 画外圈边框
 *   3. 写文字（帅/将/仕/士...），红方用红色字，黑方用黑色字
 *
 * QRadialGradient：径向渐变，从圆心向外辐射。
 *   参数 center = 圆心像素坐标，radius = 渐变半径。
 *   setColorAt(0, ...) = 圆心处的颜色（亮）
 *   setColorAt(1, ...) = 边缘处的颜色（暗）
 *
 * Qt::AlignCenter：文字居中对齐。
 */
void BoardWidget::drawPieces(QPainter& p) {
    if (!m_game) return;

    const Board& board = m_game->getBoard();
    p.setFont(QFont("楷体", 22, QFont::Bold));   // 棋子用 22 号楷体

    // 遍历棋盘的 10×9 每个格子
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            ChessPiece* piece = board.getPieceAt({r, c});
            if (!piece) continue;    // 空位跳过

            QPoint center = boardToPixel(r, c);

            // 径向渐变模拟木制棋子立体感
            QRadialGradient grad(center, PIECE_R);
            grad.setColorAt(0, QColor(255, 245, 220));  // 中心：浅米色
            grad.setColorAt(1, QColor(200, 160, 100));  // 边缘：深木色
            p.setBrush(grad);

            // 外圈边框
            p.setPen(QPen(QColor(80, 50, 20), 2));
            p.drawEllipse(QPointF(center), PIECE_R, PIECE_R);

            // 文字 — std::string → QString 转换
            std::string sym = piece->getSymbol();
            QString text = QString::fromStdString(sym);

            // 红方用深红色，黑方用纯黑色
            QColor textColor = (piece->getColor() == Color::RED)
                               ? QColor(180, 30, 30)   // 红方：暗红
                               : QColor(20, 20, 20);    // 黑方：黑色
            p.setPen(textColor);

            /*
             * QRect(x, y, w, h) 创建一个以棋子圆心为中心的矩形区域。
             * Qt::AlignCenter 让文字在这个区域内水平和垂直都居中。
             * 矩形的宽高 = 棋子直径（PIECE_R*2）
             */
            p.drawText(QRect(center.x() - PIECE_R, center.y() - PIECE_R,
                             PIECE_R * 2, PIECE_R * 2),
                       Qt::AlignCenter, text);
        }
    }
}

// =========================== 鼠标交互 ===========================

/*
 * mousePressEvent —— 鼠标点击处理
 * ------------------------------------------------
 * Qt 把鼠标操作封装成 QMouseEvent 对象。
 * event->pos() 返回点击位置在控件内的像素坐标。
 *
 * 处理逻辑分四种情况：
 *
 *   [当前状态]              [点击了]           → [结果]
 *   ───────────────────────────────────────────────────
 *   未选中棋子              己方棋子           → 选中它，显示合法走法
 *   未选中棋子              空地/敌方棋子       → 什么都不做
 *   已选中棋子              同一棋子           → 取消选中
 *   已选中棋子              另一己方棋子        → 切换选中
 *   已选中棋子              合法走法目标        → 执行走棋
 *   已选中棋子              其他位置            → 取消选中
 *
 * emit moveMade() / emit gameOverSignal()
 *   发射 Qt 信号，MainWindow 连接了这些信号来更新 UI。
 *   信号-槽是 Qt 最核心的通信机制：一个对象发信号，另一个对象接收。
 */
void BoardWidget::mousePressEvent(QMouseEvent *event) {
    // 游戏已结束或无游戏对象：把事件交给父类处理（什么都不做）
    if (!m_game || m_game->isGameOver()) {
        QWidget::mousePressEvent(event);
        return;
    }

    Position<int> clicked = pixelToBoard(event->pos());
    if (clicked.getX() < 0) return;   // 点在棋盘外

    ChessPiece* clickedPiece = m_game->getBoard().getPieceAt(clicked);

    // ===== 情况1：没有选中棋子 → 选中己方棋子 =====
    if (m_selected.getX() < 0) {
        // 只有当前走棋方的棋子才能被选中
        if (clickedPiece && clickedPiece->getColor() == m_game->getCurrentPlayer()) {
            m_selected = clicked;    // 记录选中位置
            m_validMoves.clear();

            // 获取这个棋子理论上能走的所有位置
            auto all = clickedPiece->getValidMoves(m_game->getBoard());
            /*
             * 过滤掉"走了之后己方被将军"的非法走法。
             * （比如自己的将面前有个車，就不能把挡着的士移开）
             *
             * wouldKingBeInCheck：模拟走这一步，判断走完后己方将帅是否被攻击
             */
            for (const auto& mv : all) {
                Game::Move m{m_selected, mv};
                ChessPiece* moved = m_game->getBoard().getPieceAt(m_selected);
                m.captured = m_game->getBoard().getPieceAt(mv);
                m.movedPiece = moved;
                if (!m_game->wouldKingBeInCheck(m))
                    m_validMoves.push_back(mv);
            }
        }
        update();   // 触发重绘，显示高亮
        return;
    }

    // ===== 情况2：点击已选中的棋子 → 取消选中 =====
    if (clicked == m_selected) {
        clearSelection();
        return;
    }

    // 点击己方另一棋子 → 切换选中
    if (clickedPiece && clickedPiece->getColor() == m_game->getCurrentPlayer()) {
        m_selected = clicked;
        m_validMoves.clear();
        auto all = clickedPiece->getValidMoves(m_game->getBoard());
        for (const auto& mv : all) {
            Game::Move m(m_selected, mv);
            ChessPiece* moved = m_game->getBoard().getPieceAt(m_selected);
            m.captured = m_game->getBoard().getPieceAt(mv);
            m.movedPiece = moved;
            if (!m_game->wouldKingBeInCheck(m))
                m_validMoves.push_back(mv);
        }
        update();
        return;
    }

    // ===== 情况3：点击合法走法目标 → 执行走棋 =====
    bool isLegal = false;
    for (const auto& mv : m_validMoves) {
        if (clicked == mv) { isLegal = true; break; }
    }
    if (isLegal) {
        m_game->makeMove(m_selected, clicked);   // 调用游戏逻辑走棋
        clearSelection();                        // 清除选中状态
        emit moveMade();                         // 发射信号：走了一步棋
        if (m_game->isGameOver())
            emit gameOverSignal();               // 发射信号：游戏结束（将死）
        return;
    }

    // ===== 情况4：其他（无效点击） → 取消选中 =====
    clearSelection();
}
