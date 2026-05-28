#ifndef PAWN_H
#define PAWN_H

#include "ChessPiece.h"

// 兵 / 卒
// 走法：未过河只能向前一步，过河后可以向前/左/右一步，不能后退
class Pawn : public ChessPiece {
public:
    Pawn(Color color, const Position<int>& pos);

    std::vector<Position<int>> getValidMoves(const Board& board) const override;
    bool canMoveTo(const Position<int>& target, const Board& board) const override;
    bool inOwn(const Position<int>& pos) const;
    std::string getSymbol() const override;        // "兵" / "卒"

};

#endif // PAWN_H
