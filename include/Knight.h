#ifndef KNIGHT_H
#define KNIGHT_H

#include "ChessPiece.h"

// 马
// 走法：走"日"字，注意蹩马脚
class Knight : public ChessPiece {
public:
    Knight(Color color, const Position<int>& pos);

    std::vector<Position<int>> getValidMoves(const Board& board) const override;
    bool canMoveTo(const Position<int>& target, const Board& board) const override;
    bool isLegBlocked(const Position<int>& target, const Board& board) const;
    std::string getSymbol() const override;        // "马"

};

#endif // KNIGHT_H
