#ifndef ELEPHANT_H
#define ELEPHANT_H

#include "ChessPiece.h"

// 相 / 象
// 走法：走"田"字，不能过河，注意塞象眼
class Elephant : public ChessPiece {
public:
    Elephant(Color color, const Position<int>& pos);

    std::vector<Position<int>> getValidMoves(const Board& board) const override;
    bool canMoveTo(const Position<int>& target, const Board& board) const override;
    bool isInOwn(const Position<int>& pos) const ;
    std::string getSymbol() const override;        // "相" / "象"

    bool isPathBlocked(const Position<int>&target,const Board&board)const;
};

#endif // ELEPHANT_H
