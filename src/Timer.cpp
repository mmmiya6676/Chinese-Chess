/*
 * Timer.cpp — 倒计时输入功能
 *
 * 核心思路：
 *   主线程用 getline 等用户打字（支持中文输入法 IME）
 *   另一个线程每秒在屏幕的计时行刷新倒计时数字
 *   两线程通过 atomic 变量安全通信，互不干扰
 */

#include "../include/Timer.h"
#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
/*
 * <windows.h> —— 只用于控制台光标操控（C++ 标准库没有替代品）。
 *   本文件只用：HANDLE / GetStdHandle / STD_OUTPUT_HANDLE /
 *             GetConsoleScreenBufferInfo / CONSOLE_SCREEN_BUFFER_INFO /
 *             COORD / SetConsoleCursorPosition /
 *             WaitForSingleObject / INFINITE / CloseHandle
 */
#include <windows.h>
using namespace std;

const int MOVE_TIME_LIMIT = 90;

struct TimerData {
    atomic<bool> stop{false};      // 主线程说"停！"，定时器线程看到后就退出
    atomic<int>  remaining{0};     // 当前剩余秒数
    atomic<bool> timeUp{false};    // 倒计时到 0 后设为 true

    SHORT timerLineY = 0;  // 倒计时显示在第几行
    SHORT inputLineY = 0;  // 用户输入在第几行
};

/*
 * 定时器线程：每秒倒数，同时更新屏幕上的倒计时数字。
 *
 *   控制台屏幕布局：
 *   ┌────────────────────────────┐
 *   │ ...                        │
 *   │ 剩余 85 秒                 │  ← timerLineY（本线程刷新）
 *   │ > 炮二_                    │  ← inputLineY（getline 在这里等用户打字）
 *   └────────────────────────────┘
 */
static void timerThreadFunc(TimerData* data) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    while (!data->stop && data->remaining > 0) {
        // 等 1 秒（C++ 标准库 this_thread::sleep_for）
        this_thread::sleep_for(chrono::seconds(1));

        if (data->stop) break;

        // 倒计时减 1
        data->remaining--;

        // ---- 更新计时行 ----
        CONSOLE_SCREEN_BUFFER_INFO curInfo;
        GetConsoleScreenBufferInfo(hOut, &curInfo);

        // 跳到计时行，刷新数字
        COORD timerPos = {0, data->timerLineY};
        SetConsoleCursorPosition(hOut, timerPos);
        cout << "\r剩余 " << data->remaining << " 秒    " << flush;

        // 跳回输入行
        COORD inputPos = {0, data->inputLineY};
        SetConsoleCursorPosition(hOut, inputPos);
    }

    if (data->remaining <= 0) data->timeUp = true;
}

/*
 * 在限时内等待用户输入，同时用独立线程每秒刷新倒计时显示。
 *
 * 为什么用 getline + 线程，而不是 _kbhit()/_getch()？
 *  ┌─────────────────────────────────────────────┐                   
 *  │   getline 正常读输入（IME 工作正常）          │
 *  │   + 另一个线程每秒刷新倒计时                  │
 *  │   = 两不耽误                                 │
 *  └─────────────────────────────────────────────┘
 */
string getInputWithTimer(int timeLimitSec, bool& timeout) {
    // 打印初始计时行
    cout << "剩余 " << timeLimitSec << " 秒" << endl;

    // 准备共享数据
    TimerData data;
    data.remaining = timeLimitSec;

    // 记录计时行和输入行的 Y 坐标（行号）
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    data.timerLineY = csbi.dwCursorPosition.Y - 1;  // 上一行 = 计时行

    cout << "> " << flush;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    data.inputLineY = csbi.dwCursorPosition.Y;       // 当前行 = 输入行

    // 启动定时器线程（std::thread，传 TimerData 指针）
    thread timer(timerThreadFunc, &data);

    // 等用户输入（IME 中文输入法正常工作）
    string input;
    getline(cin, input);

    // 用户按了回车 → 通知定时器线程停止 → 等它退出
    data.stop = true;
    timer.join();

    // 判断是否超时
    if (data.timeUp) {
        timeout = true;
        cout << endl << "超时! 90秒未走棋。" << endl;
        return "";
    }

    timeout = false;
    return input;
}
