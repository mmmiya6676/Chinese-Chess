#ifndef KING_H
#define KING_H

#include "ChessPiece.h"

// 帅 / 将
// 走法：九宫格内直线一格，不能与对方将帅对面
class King : public ChessPiece {
public:
    King(Color color, const Position<int>& pos);
    bool isInPalace(const Position<int>& pos) const;
    std::vector<Position<int>> getValidMoves(const Board& board) const override;
    bool canMoveTo(const Position<int>& target, const Board& board) const override;

    std::string getSymbol() const override;        // "帅" / "将"

};

#endif // KING_H
