#ifndef POSITION_H
#define POSITION_H

#include <iostream>

// 位置模板类，T 为坐标类型 (int 或自定义 Coord)
template <typename T>
class Position {
public:
    Position();
    Position(T x, T y);
    Position(const Position& other);
    ~Position() = default;

    // Getter / Setter
    T getX() const;
    T getY() const;
    void setX(T x);
    void setY(T y);

    // 运算符重载
    Position<T>& operator=(const Position<T>& other);
    bool operator==(const Position<T>& other) const;
    bool operator!=(const Position<T>& other) const;
    Position<T> operator+(const Position<T>& other) const;
    Position<T> operator-(const Position<T>& other) const;

    // 流输出/输入
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const Position<U>& pos);

    template <typename U>
    friend std::istream& operator>>(std::istream& is, Position<U>& pos);

private:
    T x;  // 行 (0~9, 共10行)
    T y;  // 列 (0~8, 共9列)
};

// ========== 模板实现 ==========

template<typename T>
Position<T>::Position() {
    x = 0;
    y = 0;
}

template<typename T>
Position<T>::Position(T x, T y) {
    this->x = x;
    this->y = y;
}

template<typename T>
Position<T>::Position(const Position<T>& another) {
    this->x = another.x;
    this->y = another.y;
}

template<typename T>
T Position<T>::getX() const {
    return x;
}

template<typename T>
T Position<T>::getY() const {
    return y;
}

template<typename T>
void Position<T>::setX(T x) {
    this->x = x;
}

template<typename T>
void Position<T>::setY(T y) {
    this->y = y;
}

template<typename T>
Position<T>& Position<T>::operator=(const Position<T>& other) {
    this->x = other.x;
    this->y = other.y;
    return *this;
}

template<typename T>
bool Position<T>::operator==(const Position<T>& another) const {
    return this->x == another.x && this->y == another.y;
}

template<typename T>
bool Position<T>::operator!=(const Position<T>& another) const {
    return this->x != another.x || this->y != another.y;
}

template<typename T>
Position<T> Position<T>::operator+(const Position<T>& other) const {
    return Position<T>(this->x + other.x, this->y + other.y);
}

template<typename T>
Position<T> Position<T>::operator-(const Position<T>& other) const {
    return Position<T>(this->x - other.x, this->y - other.y);
}

template<typename U>
std::ostream& operator<<(std::ostream& os, const Position<U>& ps) {
    os << "(" << ps.x << "," << ps.y << ")";
    return os;
}

template<typename U>
std::istream& operator>>(std::istream& is, Position<U>& ps) {
    U x, y;
    is >> x >> y;
    ps = Position<U>(x, y);
    return is;
}

#endif // POSITION_H
