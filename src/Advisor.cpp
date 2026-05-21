#include "../include/Advisor.h"
Advisor::Advisor(Color color,const Position<int>&pos)
:ChessPiece(color,PieceType::ADVISOR,pos){
    m_value = 2;
}

std::string Advisor::getSymbol()const{
    return m_color == Color::RED ? "仕" : "士";
}

bool Advisor::isInPalace(const Position<int>&pos)const{
    int x = pos.getX();
    int y = pos.getY();
    if (m_color == Color::RED) {
        // 红仕在下方九宫格：行 7-9，列 3-5
        return (x >= 7 && x <= 9 && y >= 3 && y <= 5);
    } else {
        // 黑仕在上方九宫格：行 0-2，列 3-5
        return (x >= 0 && x <= 2 && y >= 3 && y <= 5);
    }
}

bool Advisor::canMoveTo(const Position<int>&target,const Board&board)const{
    if(!isInPalace(target)){
        return false;
    }
    if(board.isPositionOccupiedBy(target,m_color)){
        return false;
    }
    int dx=abs(target.getX()-getPosition().getX());
    int dy=abs(target.getY()-getPosition().getY());
    if(dx!=1||dy!=1){
        return false;
    }
    return true;
}

std::vector<Position<int>> Advisor::getValidMoves(const Board& board)const{
    std::vector<Position<int>>moves;
    int dx[]={-1,1,-1,1};
    int dy[]={-1,1,1,-1};
    for(int i=0;i<4;++i){
        Position<int>newPos(getPosition().getX()+dx[i],getPosition().getY()+dy[i]);
        if(isInPalace(newPos)){
            if(canMoveTo(newPos,board)){
                moves.push_back(newPos);
            }
        }
    }
    return moves;
}
