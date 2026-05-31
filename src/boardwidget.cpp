/*
 * boardwidget.cpp —— 棋盘控件（核心可视化组件）
 * =============================================
 *
 * 【QPainter —— Qt 的"画笔"】
 *   Qt 的程序绘制都是通过 QPainter 完成的。它是一个"绘图引擎"，
 *   提供画线、画圆、写文字、填色等所有绘图操作。
 *
 *   重要规则：QPainter 只能在 paintEvent() 里创建和使用。
 *   不能在其他地方保存 QPainter 对象然后复用。
 *
 *   常用方法：
 *     drawLine(x1,y1, x2,y2)     画直线
 *     drawRect(x,y, w,h)         画矩形
 *     drawEllipse(QPointF, rx,ry) 画椭圆（rx=ry=半径时就是圆）
 *     drawText(QRect, align, text) 写文字
 *     fillRect(QRect, color)      填充矩形
 *
 *   QPen（笔）：控制线条的颜色、粗细、虚实
 *   QBrush（刷）：控制填充的颜色、渐变、纹理
 *   两者配合使用：pen 画轮廓，brush 填内部。
 *
 * 【paintEvent —— "你来画吧"事件】
 *   Qt 绘制是"被动"的——不是你想画就画，而是等着 Qt 叫你画。
 *   以下情况会触发 paintEvent：
 *     1. 窗口第一次显示
 *     2. 调用了 update()（标记"需要重绘"，Qt 下次有空就调用 paintEvent）
 *     3. 调用了 repaint()（立即重绘，不等事件队列）
 *     4. 窗口被遮挡后重新露出
 *     5. 窗口从最小化恢复
 *
 *   update() 和 repaint() 的区别：
 *     update()  — 异步，把"需要重绘"标记放进事件队列，Qt 空闲时执行。
 *                 多次 update() 可能合并为一次 paintEvent，效率高。
 *     repaint() — 同步，立即绘制，不等事件队列。
 *                 会阻塞当前代码，一般不推荐，只在紧急需要立即刷新时用。
 *
 * 【QRadialGradient —— 径向渐变】
 *   从圆心向外辐射的颜色渐变，用来模拟棋子的立体感。
 *   setColorAt(0, color) — 圆心处的颜色（亮）
 *   setColorAt(1, color) — 边缘处的颜色（暗）
 *
 * 【QMouseEvent —— 鼠标事件】
 *   event->pos() 返回点击位置在控件内的像素坐标（从控件左上角算）。
 *   Qt::LeftButton 表示鼠标左键。
 *
 * 【坐标转换】
 *   棋盘逻辑坐标 (row, col)：row=0~9, col=0~8
 *   屏幕像素坐标 (x, y)：控件内的像素位置
 *   boardToPixel()：逻辑→像素
 *   pixelToBoard()：像素→逻辑（容错：点格子中心附近就算点到那格）
 *
 * 【信号发射（emit）】
 *   emit moveMade()     → MainWindow::onMoveMade 自动被调用
 *   emit gameOverSignal() → MainWindow::onGameOver 自动被调用
 */

#include "boardwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFont>
#include <QPen>

// =========================== 构造函数 ===========================

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(MARGIN * 2 + CELL * (COLS - 1) + 20,
                   MARGIN * 2 + CELL * (ROWS - 1) + 20);
    setMouseTracking(false);  // false=只在按下时追踪鼠标，省性能
}

// =========================== 坐标转换 ===========================

QPoint BoardWidget::boardToPixel(int row, int col) const {
    return QPoint(MARGIN + col * CELL, MARGIN + row * CELL);
}

Position<int> BoardWidget::pixelToBoard(const QPoint& pt) const {
    // + CELL/2 让格子中心附近都算点到那格，提高容错
    int col = (pt.x() - MARGIN + CELL / 2.0) / CELL;
    int row = (pt.y() - MARGIN + CELL / 2.0) / CELL;
    if (col < 0 || col >= COLS || row < 0 || row >= ROWS)
        return {-1, -1};   // 棋盘外的点击
    return {row, col};
}

void BoardWidget::clearSelection() {
    m_selected = {-1, -1};
    m_validMoves.clear();
    update();   // 触发重绘让高亮消失
}

// =========================== 绘制主流程 ===========================

void BoardWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);   // 创建画笔（必须在 paintEvent 内）

    // 开启抗锯齿：让圆和文字边缘平滑不锯齿
    p.setRenderHint(QPainter::Antialiasing, true);

    // 先铺底色
    p.fillRect(rect(), QColor(238, 207, 161));

    // 绘制顺序决定"谁在上面"——先画的被后画的覆盖
    drawBoard(p);        // 1. 棋盘网格线（最底层）
    drawHighlights(p);   // 2. 选中高亮 + 走法提示（中间层）
    drawPieces(p);       // 3. 棋子（最上层，盖在格线上）
}

// =========================== 棋盘网格 ===========================

void BoardWidget::drawBoard(QPainter& p) {
    QPen gridPen(QColor(80, 50, 20), 1.5);   // 网格线：深褐色 1.5 像素
    QPen borderPen(QColor(60, 30, 10), 3);    // 外框线：更深更粗

    // 外框
    p.setPen(borderPen);
    p.drawRect(MARGIN - 8, MARGIN - 8,
               CELL * (COLS - 1) + 16, CELL * (ROWS - 1) + 16);

    // 横线（10 条）
    p.setPen(gridPen);
    for (int r = 0; r < ROWS; r++) {
        QPoint left  = boardToPixel(r, 0);
        QPoint right = boardToPixel(r, COLS - 1);
        p.drawLine(left.x(), left.y(), right.x(), right.y());
    }

    // 竖线（9 条）—— 中间的在河界处断开
    for (int c = 0; c < COLS; c++) {
        if (c == 0 || c == COLS - 1) {
            // 左右边界：一整条线
            QPoint top = boardToPixel(0, c), bot = boardToPixel(ROWS - 1, c);
            p.drawLine(top.x(), top.y(), bot.x(), bot.y());
        } else {
            // 其余：上半段 + 下半段（第4-5行之间断开 = 河界）
            QPoint t1 = boardToPixel(0, c), t2 = boardToPixel(4, c);
            p.drawLine(t1.x(), t1.y(), t2.x(), t2.y());
            QPoint b1 = boardToPixel(5, c), b2 = boardToPixel(ROWS - 1, c);
            p.drawLine(b1.x(), b1.y(), b2.x(), b2.y());
        }
    }

    // 九宫格斜线（lambda 避免画上下九宫时重复代码）
    QPen dashPen(QColor(80, 50, 20), 1.2);
    p.setPen(dashPen);
    auto drawPalace = [&](int r1, int r2) {
        QPoint tl = boardToPixel(r1, 3), tr = boardToPixel(r1, 5);
        QPoint bl = boardToPixel(r2, 3), br = boardToPixel(r2, 5);
        p.drawLine(tl.x(), tl.y(), br.x(), br.y());
        p.drawLine(tr.x(), tr.y(), bl.x(), bl.y());
    };
    drawPalace(0, 2);   // 黑方九宫（上方，第0-2行）
    drawPalace(7, 9);   // 红方九宫（下方，第7-9行）

    // 楚河 汉界
    p.setFont(QFont("楷体", 22, QFont::Bold));
    p.setPen(QColor(60, 30, 10));
    QPoint rL = boardToPixel(4, 0), rR = boardToPixel(5, COLS - 1);
    int ry = (rL.y() + rR.y()) / 2;
    int cx = MARGIN + CELL * (COLS - 1) / 2;
    p.drawText(cx - CELL * 2, ry - 4, "楚  河");
    p.drawText(cx + CELL * 1, ry - 4, "汉  界");
}

// =========================== 高亮提示 ===========================

void BoardWidget::drawHighlights(QPainter& p) {
    if (m_selected.getX() < 0) return;   // 没选中棋子，不画

    // 选中棋子 — 黄色半透明光圈
    QPoint sel = boardToPixel(m_selected.getX(), m_selected.getY());
    p.setPen(QPen(QColor(255, 215, 0), 3));     // 金色边框（RGB 255,215,0）
    p.setBrush(QColor(255, 255, 0, 60));         // 黄色半透明（alpha=60）
    p.drawEllipse(QPointF(sel), PIECE_R + 3, PIECE_R + 3);

    // 合法走法提示
    for (const auto& mv : m_validMoves) {
        QPoint pt = boardToPixel(mv.getX(), mv.getY());
        ChessPiece* piece = m_game->getBoard().getPieceAt(mv);

        if (piece) {
            // 可吃子 → 红色圈
            p.setPen(QPen(QColor(220, 50, 50), 3));
            p.setBrush(QColor(255, 100, 100, 60));
        } else {
            // 空位 → 绿色小圆点（无边框）
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 180, 0, 120));
        }
        p.drawEllipse(QPointF(pt), piece ? PIECE_R + 3 : 8, piece ? PIECE_R + 3 : 8);
    }
}

// =========================== 棋子绘制 ===========================

void BoardWidget::drawPieces(QPainter& p) {
    if (!m_game) return;
    const Board& board = m_game->getBoard();
    p.setFont(QFont("楷体", 22, QFont::Bold));

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            ChessPiece* piece = board.getPieceAt({r, c});
            if (!piece) continue;

            QPoint center = boardToPixel(r, c);

            // 径向渐变——中心亮边缘暗，模拟木制棋子立体感
            QRadialGradient grad(center, PIECE_R);
            grad.setColorAt(0, QColor(255, 245, 220));  // 中心：浅米色
            grad.setColorAt(1, QColor(200, 160, 100));  // 边缘：深木色
            p.setBrush(grad);

            // 外圈边框
            p.setPen(QPen(QColor(80, 50, 20), 2));
            p.drawEllipse(QPointF(center), PIECE_R, PIECE_R);

            // 棋子文字
            QString text = QString::fromStdString(piece->getSymbol());
            // 红方暗红，黑方纯黑
            p.setPen(piece->getColor() == Color::RED
                     ? QColor(180, 30, 30) : QColor(20, 20, 20));

            // QRect 定义文字区域，Qt::AlignCenter 水平+垂直居中
            p.drawText(QRect(center.x() - PIECE_R, center.y() - PIECE_R,
                             PIECE_R * 2, PIECE_R * 2),
                       Qt::AlignCenter, text);
        }
    }
}

// =========================== 鼠标走棋 ===========================

/*
 * 鼠标点击处理逻辑（四种情况）：
 *
 *   [当前状态]        → [点击位置]        → [结果]
 *   ─────────────────────────────────────────────────
 *   未选中            己方棋子            选中它，显示合法走法
 *   未选中            空/敌方棋           无操作
 *   已选中棋子A       棋子A本身           取消选中
 *   已选中棋子A       己方棋子B           切换选中到 B
 *   已选中棋子A       合法目标(空位/敌)   执行走棋，清除选中
 *   已选中棋子A       非法位置            取消选中
 */
void BoardWidget::mousePressEvent(QMouseEvent *event) {
    if (!m_game || m_game->isGameOver()) {
        QWidget::mousePressEvent(event);  // 交给父类默认处理
        return;
    }

    Position<int> clicked = pixelToBoard(event->pos());
    if (clicked.getX() < 0) return;   // 点在棋盘外

    ChessPiece* clickedPiece = m_game->getBoard().getPieceAt(clicked);

    // 情况1：没有选中 → 选中己方棋子
    if (m_selected.getX() < 0) {
        if (clickedPiece && clickedPiece->getColor() == m_game->getCurrentPlayer()) {
            m_selected = clicked;
            m_validMoves.clear();
            // 获取该棋子理论上能走的所有位置
            auto all = clickedPiece->getValidMoves(m_game->getBoard());
            // 过滤掉走后被将军的违规走法
            for (const auto& mv : all) {
                Game::Move m{m_selected, mv};
                ChessPiece* moved = m_game->getBoard().getPieceAt(m_selected);
                m.captured = m_game->getBoard().getPieceAt(mv);
                m.movedPiece = moved;
                if (!m_game->wouldKingBeInCheck(m))
                    m_validMoves.push_back(mv);
            }
        }
        update();
        return;
    }

    // 情况2：点同一棋子 → 取消选中
    if (clicked == m_selected) { clearSelection(); return; }

    // 点己方另一棋子 → 切换选中
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

    // 情况3：点在合法目标上 → 走棋
    bool isLegal = false;
    for (const auto& mv : m_validMoves) {
        if (clicked == mv) { isLegal = true; break; }
    }
    if (isLegal) {
        m_game->makeMove(m_selected, clicked);
        clearSelection();
        emit moveMade();         // 通知 MainWindow "走了一步棋"
        if (m_game->isGameOver())
            emit gameOverSignal();  // 通知 MainWindow "将死了"
        return;
    }

    // 情况4：其他 → 取消选中
    clearSelection();
}
