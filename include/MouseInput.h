#ifndef MOUSEINPUT_H
#define MOUSEINPUT_H

#include "Game.h"

// 鼠标模式的返回结果
enum class InputResult {
    Move,             // 正常走了一步棋
    SwitchToKeyboard, // 用户要求切到键盘输入模式
    Quit              // 用户退出（Q键）
};

InputResult mouseInputLoop(Game& game);

#endif // MOUSEINPUT_H
