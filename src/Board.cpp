#include"../include/Board.h"
#include"../include/King.h"
#include"../include/Rook.h"
#include"../include/Advisor.h"
#include"../include/Knight.h"
#include"../include/Pawn.h"
#include"../include/Elephant.h"
#include"../include/Cannon.h"
#include<iostream>
#include<string>
#include<algorithm>
using namespace std;
//构造棋盘
Board::Board(){
    for(int i=0;i<10;++i){
        for(int j=0;j<9;++j){
            m_grid[i][j]=NULL;
        }
    }
}
//析构棋盘，删除m_pieces中的棋子，m_grid每个位置地址指向NULL,
Board::~Board(){
    for(ChessPiece* piece:m_pieces){
        delete piece;
    }
    m_pieces.clear();
    for(int i=0;i<10;++i){
        for(int j=0;j<9;++j){
            m_grid[i][j]=NULL;
        }
    }
}

//判断位置是否在棋盘范围内,0<=x<10,0<=y<9.
bool Board::isPositionValid(const Position<int>&pos)const{
   int x=pos.getX();
   int y=pos.getY();
   return(x>=0&&x<10&&y>=0&&y<9);
}

//获取指定位置的棋子//无子返回 nullptr

ChessPiece* Board::getPieceAt(const Position<int>&pos)const{
    if(!isPositionValid(pos))return NULL;
    return m_grid[pos.getX()][pos.getY()];
}

//判断指定位置是否为空
bool Board::isPositionEmpty(const Position<int>&pos)const{
    return getPieceAt(pos)==NULL;
}

//判断指定位置是否有某方棋子
bool Board::isPositionOccupiedBy(const Position<int>&pos,Color color)const{
    ChessPiece* piece=getPieceAt(pos);
    return piece!=NULL&&piece->getColor()==color;
}

//在指定位置放置棋子
void Board::placePiece(ChessPiece* piece,const Position<int>&pos){
    if(piece==NULL)return;
    piece->setPosition(pos);
    m_grid[pos.getX()][pos.getY()]=piece;
    m_pieces.push_back(piece);
}

//释放所有棋子,与析构函数同理
void Board::clear(){
    for(ChessPiece* piece:m_pieces){
        delete piece;
    }
    m_pieces.clear();
    for(int i=0;i<10;++i){
        for(int j=0;j<9;++j){
            m_grid[i][j]=NULL;
        }
    }
}

bool Board::operator==(const Board& other)const{
    if(m_pieces.size()!=other.m_pieces.size()){
        return false;
    }
    for(int i=0;i<m_pieces.size();++i){
        if(*m_pieces[i]!=*other.m_pieces[i]){
            return false;
        }
    }
    return true;
}

//初始化棋盘
void Board::initialize(){
    clear();
    //黑方棋子
    placePiece(new King(Color::BLACK,Position<int>(0,4)),Position<int>(0,4));
    placePiece(new Advisor(Color::BLACK,Position<int>(0,3)),Position<int>(0,3));
    placePiece(new Advisor(Color::BLACK,Position<int>(0,5)),Position<int>(0,5));
    placePiece(new Elephant(Color::BLACK,Position<int>(0,2)),Position<int>(0,2));
    placePiece(new Elephant(Color::BLACK,Position<int>(0,6)),Position<int>(0,6));
    placePiece(new Knight(Color::BLACK,Position<int>(0,1)),Position<int>(0,1));
    placePiece(new Knight(Color::BLACK,Position<int>(0,7)),Position<int>(0,7));
    placePiece(new Rook(Color::BLACK,Position<int>(0,0)),Position<int>(0,0));
    placePiece(new Rook(Color::BLACK,Position<int>(0,8)),Position<int>(0,8));
    placePiece(new Cannon(Color::BLACK,Position<int>(2,1)),Position<int>(2,1));
    placePiece(new Cannon(Color::BLACK,Position<int>(2,7)),Position<int>(2,7));
    for(int i=0;i<9;i+=2){
        placePiece(new Pawn(Color::BLACK,Position<int>(3,i)),Position<int>(3,i));
    }
    //红方棋子
    placePiece(new King(Color::RED,Position<int>(9,4)),Position<int>(9,4));
    placePiece(new Advisor(Color::RED,Position<int>(9,3)),Position<int>(9,3));
    placePiece(new Advisor(Color::RED,Position<int>(9,5)),Position<int>(9,5));
    placePiece(new Elephant(Color::RED,Position<int>(9,2)),Position<int>(9,2));
    placePiece(new Elephant(Color::RED,Position<int>(9,6)),Position<int>(9,6));
    placePiece(new Knight(Color::RED,Position<int>(9,1)),Position<int>(9,1));
    placePiece(new Knight(Color::RED,Position<int>(9,7)),Position<int>(9,7));
    placePiece(new Rook(Color::RED,Position<int>(9,0)),Position<int>(9,0));
    placePiece(new Rook(Color::RED,Position<int>(9,8)),Position<int>(9,8));
    placePiece(new Cannon(Color::RED,Position<int>(7,1)),Position<int>(7,1));
    placePiece(new Cannon(Color::RED,Position<int>(7,7)),Position<int>(7,7));
    for(int i=0;i<9;i+=2){
        placePiece(new Pawn(Color::RED,Position<int>(6,i)),Position<int>(6,i));
    }
}

//深拷贝构造函数，
Board::Board(const Board& other){
    clear();
    for(ChessPiece* piece:other.m_pieces){
        Position<int>pos=piece->getPosition();
        switch(piece->getType()){
            case PieceType::KING:
                placePiece(new King(piece->getColor(),pos),pos);
                break;
            case PieceType::ADVISOR:
                placePiece(new Advisor(piece->getColor(),pos),pos);
                break;
            case PieceType::ELEPHANT:
                placePiece(new Elephant(piece->getColor(),pos),pos);
                break;
            case PieceType::KNIGHT:
                placePiece(new Knight(piece->getColor(),pos),pos);
                break;
            case PieceType::ROOK:
                placePiece(new Rook(piece->getColor(),pos),pos);
                break;
            case PieceType::CANNON:
                placePiece(new Cannon(piece->getColor(),pos),pos);
                break;
            case PieceType::PAWN:
                placePiece(new Pawn(piece->getColor(),pos),pos);
                break;
        }
    }
}

//赋值运算符重载，与深拷贝同理
Board& Board::operator=(const Board& other){
    clear();
    for(ChessPiece* piece:other.m_pieces){
        Position<int>pos=piece->getPosition();
        switch(piece->getType()){
            case PieceType::KING:
                placePiece(new King(piece->getColor(),pos),pos);
                break;
            case PieceType::ADVISOR:
                placePiece(new Advisor(piece->getColor(),pos),pos);
                break;
            case PieceType::ELEPHANT:
                placePiece(new Elephant(piece->getColor(),pos),pos);
                break;
            case PieceType::KNIGHT:
                placePiece(new Knight(piece->getColor(),pos),pos);
                break;
            case PieceType::ROOK:
                placePiece(new Rook(piece->getColor(),pos),pos);
                break;
            case PieceType::CANNON:
                placePiece(new Cannon(piece->getColor(),pos),pos);
                break;
            case PieceType::PAWN:
                placePiece(new Pawn(piece->getColor(),pos),pos);
                break;
        }
    }
    return *this;
}

//移动棋子
ChessPiece*Board::movePiece(const Position<int>&from,const Position<int>&to){
    ChessPiece* piece=getPieceAt(from);
    if(piece==NULL){
        return NULL;
    }
    ChessPiece* temp=getPieceAt(to);
    m_grid[to.getX()][to.getY()]=piece;
    piece->setPosition(to);
    m_grid[from.getX()][from.getY()]=NULL;
    if(temp!=NULL){
        temp->setAlive(false);
        auto it=std::find(m_pieces.begin(),m_pieces.end(),temp);
        if(it!=m_pieces.end())m_pieces.erase(it);
    }
    return temp;
}

//悔棋功能
void Board::undoMove(const Position<int>&from,const Position<int>&to,ChessPiece* temp){
    ChessPiece* piece=getPieceAt(to);
    if(piece==nullptr)return;
    m_grid[from.getX()][from.getY()]=piece;
    m_grid[to.getX()][to.getY()]=nullptr;
    piece->setPosition(from);
    if(temp!=nullptr){
        temp->setAlive(true);
        temp->setPosition(to);
        m_grid[to.getX()][to.getY()]=temp;
        m_pieces.push_back(temp);
    }
}

//获取某方所有存活棋子
vector<ChessPiece*>Board::getPieces(Color color)const{
    vector<ChessPiece*>pieces;
    for(ChessPiece* piece:m_pieces){
        if(piece->getColor()==color&&piece->isAlive()){
            pieces.push_back(piece);
        }
    }
    return pieces;
}

//查找某方的将/帅
ChessPiece*Board::findKing(Color color)const{
    for(ChessPiece* piece:m_pieces){
        if(piece->getType()==PieceType::KING&&piece->getColor()==color&&piece->isAlive()){
            return piece;
        }
    }
    return nullptr;
}

//是否被将军
bool Board::isKingInCheck(Color color)const{
    ChessPiece* king=findKing(color);
    if(king==nullptr)return false;
    Position<int>kingPos=king->getPosition();
    Color enemyColor=(color==Color::RED)?Color::BLACK:Color::RED;
    vector<ChessPiece*>enemyPieces=getPieces(enemyColor);
    for(ChessPiece* piece:enemyPieces){
        if(piece->canMoveTo(kingPos,*this)){
            return true;
            //如果被将军，就返回 true
        }
    }
    return false;
    //如果没有被将军，就返回 false
}


bool Board::isCheckmate(Color color)const{
    if(!isKingInCheck(color))return false;
    //如果没有被将军，就返回 false  
    bool canEscape=false;
    vector<ChessPiece*>mypieces=getPieces(color);
    for(ChessPiece*piece:mypieces){
        vector<Position<int>>moves=piece->getValidMoves(*this);
        //遍历所有有效移动（包含挡将军以及将帅的移动躲避）
        for(const Position<int>&tagert:moves){
            Position<int> originalPos = piece->getPosition();  // 保存原位（movePiece 后会变）
            ChessPiece* temp=const_cast<Board*>(this)->movePiece(originalPos,tagert);
            //尝试移动棋子
            bool stillInCheck=isKingInCheck(color);
            //如果移动后被将军，就返回 false
            const_cast<Board*>(this)->undoMove(originalPos,tagert,temp);
            //撤销继续遍历其他有效移动
            if(!stillInCheck){
                canEscape=true;
                break;
            }
        }
    }
    if(canEscape)return false;
    //根据canEscape的维护，返回 true 或 false，判断是否将死
    return true;
}
ostream& operator<<(ostream& os, const Board& board) {
    os << " ======================" << endl;
    os << "   ";
    for (int j = 0; j < Board::COLS; ++j) {
        os << " " << j << " ";
    }
    os << endl;

    for (int i = 0; i < Board::ROWS; ++i) {
        os << " " << i << " ";
        for (int j = 0; j < Board::COLS; ++j) {
            ChessPiece* p = board.m_grid[i][j];
            if (p == nullptr) {
                os << " . ";
            } else {
                os << " " << p->getSymbol();
            }
        }
        os << endl;
    }
    os << " ======================";
    return os;
}
istream& operator>>(istream& is,Board& board){
    int fx, fy, tx, ty;
    char c1, c2;
// 1. 检查读取是否成功
    if (!(is >> fx >> c1 >> fy >> tx >> c2 >> ty)) {
        return is; // 读取失败，直接返回流
    }
// 2. 构造位置对象并执行移动
    Position<int> from(fx, fy);
    Position<int> to(tx, ty);
    ChessPiece* captured = board.movePiece(from, to);
    if (captured == nullptr && board.getPieceAt(from) != nullptr) {
// 如果移动失败且起点仍有棋子，说明操作无效，标记流为错误状态
        is.setstate(ios::failbit);
    }
    return is;
}