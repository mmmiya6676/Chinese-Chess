#include"../include/King.h"
#include"../include/Board.h"
King::King(Color color,const Position<int>&pos)
:ChessPiece(color,PieceType::KING,pos){
    m_value = 1000;
}

std::string King::getSymbol()const{
    return m_color == Color::RED ? "帅" : "将";
}

bool King::canMoveTo(const Position<int>&target,const Board&board)const{
    if(!board.isPositionValid(target)){
        return false;
    }
    if(board.isPositionOccupiedBy(target,m_color)){
        return false;
    }
    if(!isInPalace(target)){
        return false;
    }
    // 对将检测：将帅不能对面（同列且之间无任何棋子）
    ChessPiece* enemyKing = board.findKing(
        (m_color == Color::RED) ? Color::BLACK : Color::RED);
    if (enemyKing && target.getY() == enemyKing->getPosition().getY()) {
        int minRow = std::min(target.getX(), enemyKing->getPosition().getX());
        int maxRow = std::max(target.getX(), enemyKing->getPosition().getX());
        bool blocked = false;
        for (int r = minRow + 1; r < maxRow; ++r) {
            if (!board.isPositionEmpty(Position<int>(r, target.getY()))) {
                blocked = true;
                break;
            }
        }
        if (!blocked) return false;  // 同列且之间无子 = 对将，非法
    }
    int dx=abs(target.getX()-getPosition().getX());
    int dy=abs(target.getY()-getPosition().getY());
    return (dx==1&&dy==0)||(dy==1&&dx==0);
}

bool King::isInPalace(const Position<int>& pos) const {
    int x = pos.getX();
    int y = pos.getY();
    if (getColor() == Color::RED) {
        // 红帅在下方九宫格：行 7-9，列 3-5
        return (x >= 7 && x <= 9 && y >= 3 && y <= 5);
    } else {
        // 黑将在上方九宫格：行 0-2，列 3-5
        return (x >= 0 && x <= 2 && y >= 3 && y <= 5);
    }
}

std::vector<Position<int>> King::getValidMoves(const Board& board)const{
    std::vector<Position<int>>moves;
    int dx[]={-1,1,0,0};
    int dy[]={0,0,-1,1};
    for(int i=0;i<4;++i){
        Position<int>newPos(getPosition().getX()+dx[i],getPosition().getY()+dy[i]);
       if(isInPalace(newPos)){
        if(board.isPositionEmpty(newPos)||
        board.isPositionOccupiedBy(newPos,getColor()==Color::RED?Color::BLACK:Color::RED)){
            moves.push_back(newPos);
            }
       }
    }
    return moves;
}
