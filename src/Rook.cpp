#include"../include/Rook.h"
#include"../include/Board.h"
Rook::Rook(Color color,const Position<int>&pos)
:ChessPiece(color,PieceType::ROOK,pos){
    m_value = 5;
}
std::string Rook::getSymbol()const{
    return "车";
}

bool Rook::canMoveTo(const Position<int>&target,const Board&board)const{
    if(!board.isPositionValid(target)){
        return false;
    }
    if(target.getX()!=getPosition().getX()&&target.getY()!=getPosition().getY()){
        return false;
    }
    if(target.getY()==getPosition().getY()){
        int min=std::min(target.getX(),getPosition().getX());
        int max=std::max(target.getX(),getPosition().getX());
        for(int i=min+1;i<max;++i){
            if(!board.isPositionEmpty(Position<int>(i,target.getY()))){
            return false;
            }
        }
    }
    if(target.getX()==getPosition().getX()){
        int min=std::min(target.getY(),getPosition().getY());
        int max=std::max(target.getY(),getPosition().getY());
        for(int i=min+1;i<max;++i){
        if(!board.isPositionEmpty(Position<int>(target.getX(),i))){
            return false;
             }
        }
    }
    if(board.isPositionOccupiedBy(target,m_color)){
        return false;
    }
    return true;
}
std::vector<Position<int>> Rook::getValidMoves(const Board& board)const{
    std::vector<Position<int>>moves;
    int x = getPosition().getX();
    int y = getPosition().getY();
    // 四个方向扫描：上、下、左、右
    // 上（行减小）
    for(int i=x-1;i>=0;--i){
        Position<int>p(i,y);
        if(board.isPositionEmpty(p)){
            moves.push_back(p);
        }else{
            if(!board.isPositionOccupiedBy(p,m_color)) moves.push_back(p);
            break;
        }
    }
    // 下（行增大）
    for(int i=x+1;i<Board::ROWS;++i){
        Position<int>p(i,y);
        if(board.isPositionEmpty(p)){
            moves.push_back(p);
        }else{
            if(!board.isPositionOccupiedBy(p,m_color)) moves.push_back(p);
            break;
        }
    }
    // 左（列减小）
    for(int i=y-1;i>=0;--i){
        Position<int>p(x,i);
        if(board.isPositionEmpty(p)){
            moves.push_back(p);
        }else{
            if(!board.isPositionOccupiedBy(p,m_color)) moves.push_back(p);
            break;
        }
    }
    // 右（列增大）
    for(int i=y+1;i<Board::COLS;++i){
        Position<int>p(x,i);
        if(board.isPositionEmpty(p)){
            moves.push_back(p);
        }else{
            if(!board.isPositionOccupiedBy(p,m_color)) moves.push_back(p);
            break;
        }
    }
    return moves;
}