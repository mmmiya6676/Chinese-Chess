#ifndef CANNON_H
#define CANNON_H

#include "ChessPiece.h"
#include "Board.h"

// 炮
// 走法：直线走任意格（无炮架），吃子时必须翻过一个棋子（炮架）
class Cannon : public ChessPiece {
public:
    Cannon(Color color, const Position<int>& pos);

    std::vector<Position<int>> getValidMoves(const Board& board) const override;
    bool canMoveTo(const Position<int>& target, const Board& board) const override;

    std::string getSymbol() const override;        // "炮"

};

#endif // CANNON_H
