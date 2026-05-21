#include"../include/ChessPiece.h"
ChessPiece::ChessPiece(Color color,PieceType type,const Position<int>&pos)
:m_color(color),
m_type(type),
m_position(pos),
m_alive(true){}
int ChessPiece::getValue() const{
    return m_value;
}
Position<int>ChessPiece::getPosition() const{
    return m_position;
}
Color ChessPiece::getColor() const{
    return m_color;
}
PieceType ChessPiece::getType() const{
    return m_type;
}
bool ChessPiece::isAlive() const{
    return m_alive;
}
void ChessPiece::setPosition(const Position<int>& pos){
    m_position = pos;
}
void ChessPiece::setAlive(bool alive){
    m_alive = alive;
}
bool ChessPiece::operator==(const ChessPiece& other)const{
    return m_color == other.m_color && m_type == other.m_type;
}
bool ChessPiece::operator!=(const ChessPiece& other)const{
    return !(*this == other);
}
bool ChessPiece::operator<(const ChessPiece& other)const{
    return m_value < other.m_value;
}
bool ChessPiece::operator>(const ChessPiece& other)const{
    return m_value > other.m_value;
}