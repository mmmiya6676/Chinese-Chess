#ifndef TIMER_H
#define TIMER_H

#include <string>

// 每步限时（秒）
extern const int MOVE_TIME_LIMIT;

/*
 * 在限时内等待用户输入，同时用独立线程每秒刷新倒计时显示。
 *
 *  用标准 getline 读取输入（支持 IME 中文输入法）。
 *  另一个线程用控制台 API 在计时行刷新数字。
 *  timeout 输出参数：true 表示超时。
 *  返回值：用户输入的字符串；超时时返回空字符串。
 */
std::string getInputWithTimer(int timeLimitSec, bool& timeout);

#endif // TIMER_H
