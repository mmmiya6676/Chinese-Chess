#ifndef CHESSPIECE_H
#define CHESSPIECE_H

#include "Position.h"
#include <vector>
#include <string>

// 棋子颜色
enum class Color { RED, BLACK };

// 棋子类型
enum class PieceType { KING, ADVISOR, ELEPHANT, KNIGHT, ROOK, CANNON, PAWN };

class Board;  // 前置声明

// 棋子抽象基类
class ChessPiece {
public:
    ChessPiece(Color color, PieceType type, const Position<int>& pos);
    virtual ~ChessPiece() = default;

    // -------- 纯虚函数：子类必须实现 --------

    // 返回当前棋子的所有合法走法
    virtual std::vector<Position<int>> getValidMoves(const Board& board) const = 0;

    // 判断能否移动到目标位置
    virtual bool canMoveTo(const Position<int>& target, const Board& board) const = 0;

    // 获取棋子显示符号 (中文字符)
    virtual std::string getSymbol() const = 0;

    // -------- 虚函数：子类可选覆盖 --------

    // 获取棋子价值
    virtual int getValue() const;

    // -------- 非虚成员函数 --------

    Position<int> getPosition() const;
    Color getColor() const;
    PieceType getType() const;
    bool isAlive() const;

    void setPosition(const Position<int>& pos);
    void setAlive(bool alive);

    // -------- 运算符重载 --------

    bool operator==(const ChessPiece& other) const;
    bool operator!=(const ChessPiece& other) const;
    bool operator<(const ChessPiece& other) const;   // 按价值比较
    bool operator>(const ChessPiece& other) const;

protected:
    Position<int> m_position;
    Color         m_color;
    PieceType     m_type;
    int           m_value;   // 棋子价值，用于排序
    bool          m_alive;
};

#endif // CHESSPIECE_H
