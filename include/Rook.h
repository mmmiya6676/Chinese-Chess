#ifndef ROOK_H
#define ROOK_H

#include "ChessPiece.h"

// 车
// 走法：直线走任意格，中间不能有棋子阻挡
class Rook : public ChessPiece {
public:
    Rook(Color color, const Position<int>& pos);

    std::vector<Position<int>> getValidMoves(const Board& board) const override;
    bool canMoveTo(const Position<int>& target, const Board& board) const override;

    std::string getSymbol() const override;        // "车"

};

#endif // ROOK_H
