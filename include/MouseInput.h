#ifndef MOUSEINPUT_H
#define MOUSEINPUT_H

#include "Game.h"

// 鼠标模式的返回结果
enum class InputResult {
    Move,  // 正常走了一步棋
    Quit   // 用户退出
};

InputResult mouseInputLoop(Game& game);

#endif // MOUSEINPUT_H
