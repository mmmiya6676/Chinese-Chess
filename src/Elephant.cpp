#include"../include/Elephant.h"
#include"../include/Board.h"
Elephant::Elephant(Color color,const Position<int>&pos)
:ChessPiece(color,PieceType::ELEPHANT,pos){
    m_value = 2;
}

std::string Elephant::getSymbol()const{
    return m_color == Color::RED ? "相" : "象";
}

bool Elephant::isInOwn(const Position<int>&pos)const{
    int x = pos.getX();
    int y = pos.getY();
    if (m_color == Color::RED) {
        // 红相在下方
        return (x>=5);
    }
    // 黑相在上方
    return (x<=4);
}

bool Elephant::isPathBlocked(const Position<int>&target,const Board&board)const{
    int midX = (target.getX() + m_position.getX()) / 2;
    int midY = (target.getY() + m_position.getY()) / 2;
    return !board.isPositionEmpty(Position<int>(midX,midY));
}

bool Elephant::canMoveTo(const Position<int>& target, const Board& board) const{
    if(!board.isPositionValid(target)) return false;
    if(!isInOwn(target)){
        return false;
    }
    if(board.isPositionOccupiedBy(target,getColor())){
        return false;
    }
    int dx = abs(target.getX() - m_position.getX());
    int dy = abs(target.getY() - m_position.getY());
    if(dx != 2 || dy != 2){
        return false;
    }
    if(isPathBlocked(target,board)){
        return false;
    }
    return true;
}

std::vector<Position<int>> Elephant::getValidMoves(const Board& board) const{
    std::vector<Position<int>> moves;
    int dx[]={-2,-2,2,2};
    int dy[]={-2,2,-2,2};
    for(int i=0;i<4;i++){
        int x = m_position.getX() + dx[i];
        int y = m_position.getY() + dy[i];
        if(canMoveTo(Position<int>(x,y),board)){
            moves.push_back(Position<int>(x,y));
        }
    }
    return moves;
}
