#ifndef BOARD_H
#define BOARD_H

#include "Position.h"
#include "ChessPiece.h"
#include <vector>
#include <iostream>

// 棋盘类：管理所有棋子
class Board {
public:
    static const int ROWS = 10;   // 10 行
    static const int COLS = 9;    // 9 列

    Board();
    ~Board();
    bool operator==(const Board& other)const;
    // 拷贝控制 (深拷贝)
    Board(const Board& other);
    Board& operator=(const Board& other);

    // 移动构造 / 移动赋值
    //Board(Board&& other) noexcept;
    //Board& operator=(Board&& other) noexcept;

    // -------- 初始化 --------

    // 重置为初始局面
    void initialize();
    // 清空棋盘
    void clear();

    // -------- 查询 --------

    // 获取指定位置的棋子 (无子返回 nullptr)
    ChessPiece* getPieceAt(const Position<int>& pos) const;

    // 判断位置是否在棋盘范围内
    bool isPositionValid(const Position<int>& pos) const;

    // 判断指定位置是否为空
    bool isPositionEmpty(const Position<int>& pos) const;

    // 判断指定位置是否有某方棋子
    bool isPositionOccupiedBy(const Position<int>& pos, Color color) const;

    // 获取某方所有存活棋子
    std::vector<ChessPiece*> getPieces(Color color) const;

    // 查找某方的将/帅
    ChessPiece* findKing(Color color) const;

    // -------- 走棋与回退 --------

    // 移动棋子，返回被吃的棋子 (无吃子返回 nullptr)
    ChessPiece* movePiece(const Position<int>& from, const Position<int>& to);

    // 撤销一步移动
    void undoMove(const Position<int>& from, const Position<int>& to,
                  ChessPiece* captured);

    // -------- 将军检测 --------

    // 判断 color 方是否被将军
    bool isKingInCheck(Color color) const;

    // 判断 color 方是否无子可走 (将死)
    bool isCheckmate(Color color) const;

    // -------- 运算符重载 --------

    // << 输出棋盘
    friend std::ostream& operator<<(std::ostream& os, const Board& board);

    // >> 读取走法字符串 (如 "a2 a4")
    friend std::istream& operator>>(std::istream& is, Board& board);
    // 在指定位置放置棋子
    void placePiece(ChessPiece* piece, const Position<int>& pos);
private:
    // 棋盘网格，nullptr 表示空
    ChessPiece* m_grid[ROWS][COLS];

    // 持有所有棋子的所有权，方便析构和深拷贝
    std::vector<ChessPiece*> m_pieces;

};

#endif // BOARD_H
