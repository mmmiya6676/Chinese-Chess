#include"../include/Cannon.h"
#include"../include/Board.h"
Cannon::Cannon(Color color,const Position<int>&pos)
:ChessPiece(color,PieceType::CANNON,pos){
    m_value = 6;
}

std::string Cannon::getSymbol()const{
    return "炮";
}

bool Cannon::canMoveTo(const Position<int>&target,const Board&board)const{
    if(!board.isPositionValid(target)){
        return false;
    }
    if(target.getX()!=getPosition().getX()&&target.getY()!=getPosition().getY()){
        return false;
    }
    if(target.getY()==getPosition().getY()){
        int min=std::min(target.getX(),getPosition().getX());
        int max=std::max(target.getX(),getPosition().getX());
        int count=0;
        for(int i=min+1;i<max;++i){
            if(!board.isPositionEmpty(Position<int>(i,target.getY()))){
                count++;
                }
            }
            if(count>1){
                return false;
            }
            if(count==1){
                if(board.isPositionOccupiedBy
                (Position<int>(target.getX(),target.getY())
                ,(m_color==Color::RED)?Color::RED:Color::BLACK)){
                    return false;
                }
                if(board.isPositionOccupiedBy
                (Position<int>(target.getX(),target.getY())
                ,(m_color==Color::RED)?Color::BLACK:Color::RED)){
                    return true;
                }
                return false;
            } 
            if(count==0){
                if(board.isPositionEmpty(target)){
                    return true;
                }
                return false;
            }
    } else if(target.getX()==getPosition().getX()){
        int min=std::min(target.getY(),getPosition().getY());
        int max=std::max(target.getY(),getPosition().getY());
        int count=0;
        for(int i=min+1;i<max;++i){
        if(!board.isPositionEmpty(Position<int>(target.getX(),i))){
            count++;
            }
        }
        if(count>1){
            return false;
        }
        if(count==1){
            if(board.isPositionOccupiedBy(
                Position<int>(target.getX(),target.getY())
                ,(m_color==Color::RED)?Color::RED:Color::BLACK)){
                return false;
            }
            if(board.isPositionOccupiedBy
                (Position<int>(target.getX(),target.getY())
                ,(m_color==Color::RED)?Color::BLACK:Color::RED)){
                return true;
            }
           
            return false;
        }
         if(count==0){
            if(board.isPositionEmpty(target)){
                return true;
            }
                return false;
        }
    }
    return false;
}

std::vector<Position<int>> Cannon::getValidMoves(const Board& board)const{
    std::vector<Position<int>>moves;
    int x = getPosition().getX();
    int y = getPosition().getY();
    // 四个方向扫描：上、下、左、右
    // 上（行减小）
    for(int i=x-1;i>=0;--i){
        Position<int>p(i,y);
        if(canMoveTo(p,board)){
            moves.push_back(p);
        }
    }
    for(int i=x+1;i<10;++i){
        Position<int>p(i,y);
        if(canMoveTo(p,board)){
            moves.push_back(p);
        }
    }
    for(int i=y-1;i>=0;--i){
        Position<int>p(x,i);
        if(canMoveTo(p,board)){
            moves.push_back(p);
        }
    }
    for(int i=y+1;i<9;++i){
        Position<int>p(x,i);
        if(canMoveTo(p,board)){
            moves.push_back(p);
        }
    }
    return moves;
}
