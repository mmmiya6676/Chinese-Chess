#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "Position.h"
#include "ChessPiece.h"
#include <vector>

// 棋盘模板类：支持不同规则变体 (如国际象棋、中国象棋等)
// RuleType 必须提供: initPieces(), validateMove() 等接口
template <typename RuleType>
class ChessBoard {
public:
    ChessBoard();
    ~ChessBoard();

    // 拷贝控制
    ChessBoard(const ChessBoard& other);
    ChessBoard& operator=(const ChessBoard& other);

    // 初始化棋盘
    void initialize();

    // 重置
    void clear();

    // 获取指定位置棋子
    ChessPiece* getPieceAt(const Position<int>& pos) const;

    // 移动棋子
    ChessPiece* movePiece(const Position<int>& from, const Position<int>& to);

    // 验证走法合法性 (委托给 RuleType)
    bool isValidMove(const Position<int>& from, const Position<int>& to) const;

    // 运算符 << 输出棋盘
    template <typename RT>
    friend std::ostream& operator<<(std::ostream& os,
                                    const ChessBoard<RT>& board);

private:
    static const int ROWS = 10;
    static const int COLS = 9;
    ChessPiece* m_grid[10][9];
    std::vector<ChessPiece*> m_pieces;
    RuleType m_rule;
};

#endif // CHESSBOARD_H
