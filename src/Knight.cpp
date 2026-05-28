#include"../include/Knight.h"
#include"../include/Board.h"
Knight::Knight(Color color,const Position<int>&pos)
:ChessPiece(color,PieceType::KNIGHT,pos){
    m_value = 3;
}

std::string Knight::getSymbol()const{
    return "马";
}

bool Knight::isLegBlocked(const Position<int>& target, const Board& board)const{
    if(!board.isPositionValid(target)){
        return false;
    }
    int dx=target.getX()-getPosition().getX();
    int dy=target.getY()-getPosition().getY();
    int legx=getPosition().getX();
    int legy=getPosition().getY();
    if(abs(dx)==2&&abs(dy)==1){
        legx+=dx/2;
    }
    else if(abs(dx)==1&&abs(dy)==2){
        legy+=dy/2;
    }
    Position<int>legPos(legx,legy);
    return !board.isPositionEmpty(legPos);
}

bool Knight::canMoveTo(const Position<int>& target, const Board& board) const{
    if(!board.isPositionValid(target)){
        return false;
    }
    if(isLegBlocked(target,board)){
        return false;
    }
    if(board.isPositionOccupiedBy(target,m_color)){
        return false;
    }
    int dx = abs(target.getX() - getPosition().getX());
    int dy = abs(target.getY() - getPosition().getY());
    if (!((dx == 2 && dy == 1) || (dx == 1 && dy == 2))) {
        return false;
    }
    return true;
}

std::vector<Position<int>> Knight::getValidMoves(const Board& board)const{
    std::vector<Position<int>>moves;
    int dx[]={-2, -2, -1, -1, 1, 1, 2, 2};
    int dy[]={-1,  1, -2,  2,-2, 2,-1, 1};
    for(int i=0;i<8;++i){
        Position<int>newPos(getPosition().getX()+dx[i],getPosition().getY()+dy[i]);
        if(canMoveTo(newPos,board)){
            moves.push_back(newPos);
        }
    }
    return moves;
}