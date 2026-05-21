#include"../include/Pawn.h"   
#include"../include/Board.h"
Pawn::Pawn(Color color, const Position<int>& pos)
:ChessPiece(color,PieceType::PAWN,pos){
    m_value = 1;
}

bool Pawn::inOwn(const Position<int>& pos) const{
    int x = pos.getX();
    int y = pos.getY();
    if(m_color==Color::RED){
        return x>=5&&x<=6;
    }
    else{
        return x>=3&&x<=4;
    }
}

std::string Pawn::getSymbol()const{
    return m_color == Color::RED ? "兵" : "卒";
}

bool Pawn::canMoveTo(const Position<int>&target,const Board&board)const{
    if(!board.isPositionValid(target)){
        return false;
    }
    if(board.isPositionOccupiedBy(target,m_color)){
        return false;
    }
    int dx=target.getX()-getPosition().getX();
    int dy=target.getY()-getPosition().getY();
    if(inOwn(getPosition())){
        if(dy!=0){
            return false;
        }
        else{
            if(m_color==Color::RED){
                if(dx!=-1)return false;
            }
            else{
                if(dx!=1)return false;
            }
        }
    }
   else{  // 过河后
    if(abs(dx)+abs(dy) != 1) return false;   // 必须走一步
    // 单步了，再判方向
    if(m_color==Color::RED && dx==1) return false;    // 红不能退
    if(m_color==Color::BLACK && dx==-1) return false; // 黑不能退
}

    return true;
}

std::vector<Position<int>> Pawn::getValidMoves(const Board& board)const{
    std::vector<Position<int>>moves;
    if(inOwn(getPosition())){
        if(m_color==Color::RED){
            // 红兵只能前进一步
            if(canMoveTo(Position<int>(getPosition().getX()-1,getPosition().getY()),board)){
                moves.push_back(Position<int>(getPosition().getX()-1,getPosition().getY()));
            }
        }
        else{
            // 黑兵只能前进一步
            if(canMoveTo(Position<int>(getPosition().getX()+1,getPosition().getY()),board)){
                moves.push_back(Position<int>(getPosition().getX()+1,getPosition().getY()));
            }
        }
    }
    else{
        if(m_color==Color::RED){
            int dx[]={0,0,-1};
            int dy[]={-1,1,0};
            for(int i=0;i<3;++i){
                Position<int>newPos(getPosition().getX()+dx[i],getPosition().getY()+dy[i]);
                if(canMoveTo(newPos,board)){
                    moves.push_back(newPos);
                }
            }
        }
        else{
            int dx[]={0,0,1};
            int dy[]={-1,1,0};
            for(int i=0;i<3;++i){
                Position<int>newPos(getPosition().getX()+dx[i],getPosition().getY()+dy[i]);
                if(canMoveTo(newPos,board)){
                    moves.push_back(newPos);
                }
            }
        }
    }
    return moves;
}
