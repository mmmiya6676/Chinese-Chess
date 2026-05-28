#ifndef ADVISOR_H
#define ADVISOR_H

#include "ChessPiece.h"
#include "Board.h"

// 仕 / 士
// 走法：九宫格内斜走一格
class Advisor : public ChessPiece {
public:
 
    Advisor(Color color, const Position<int>& pos);
    bool isInPalace(const Position<int>& pos) const;

    std::vector<Position<int>> getValidMoves(const Board& board) const override;
    bool canMoveTo(const Position<int>& target, const Board& board) const override;

    std::string getSymbol() const override;        // "仕" / "士"

};

#endif // ADVISOR_H
