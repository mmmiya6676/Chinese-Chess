#ifndef GAME_H
#define GAME_H

#include "Board.h"
#include "Position.h"
#include "ChessPiece.h"
#include <stack>
#include <string>
// 游戏主逻辑类
class Game {
public:
    // 走法记录
    struct Move {
        Position<int> from;
        Position<int> to;
        ChessPiece* captured;      // 被吃的棋子 (用于悔棋恢复)
        ChessPiece* movedPiece;    // 移动的棋子
        Move(const Position<int>& from, const Position<int>& to);
    };

    Game();
    ~Game() = default;

    // -------- 游戏流程 --------

    // 开始新游戏
    void start();

    // 重新开始
    void restart();

    // -------- 走棋操作 --------

    // 尝试走棋（坐标），成功返回 true
    bool makeMove(const Position<int>& from, const Position<int>& to);

    // 尝试走棋（中文记谱法，如"炮二平五""马三进四"）
    bool makeMove(const std::string& notation);

    // 悔棋 (U 键)
    bool undo();

    // 重做 (R 键)
    bool redo();

    // -------- 状态查询 --------

    // 当前走棋方
    Color getCurrentPlayer() const;

    // 切换走棋方
    void switchPlayer();

    // 判断游戏是否结束
    bool isGameOver() const;

    // 获取胜方 (游戏未结束时无意义)
    Color getWinner() const;

    // 当前步数
    int getMoveCount() const;

    // 是否可悔棋
    bool canUndo() const;

    // 是否可重做
    bool canRedo() const;

    // -------- 棋盘访问 --------

    Board& getBoard();
    const Board& getBoard() const;

    // 获取某位置棋子
    ChessPiece* getPieceAt(const Position<int>& pos) const;

    // -------- 存档 / 读档 --------

    // 保存游戏到文件
    bool saveGame(const std::string& filename) const;

    // 从文件加载游戏
    bool loadGame(const std::string& filename);

    // 当前存档文件名
    void setSaveFilename(const std::string& name);
    std::string getSaveFilename() const;

    // -------- 运算符重载 --------

    // + 执行走棋
    Game& operator+(const Move& move);

    // - 悔棋
    Game& operator-(int steps);

    // == 比较两局棋是否相同 (用于检测循环)
    bool operator==(const Game& other) const;

    // << 输出游戏状态
    friend std::ostream& operator<<(std::ostream& os, const Game& game);

    // >> 输入走法
    friend std::istream& operator>>(std::istream& is, Game& game);

private:
    Board              m_board;
    Color              m_currentPlayer;
    int                m_moveCount;
    bool               m_gameOver;
    Color              m_winner;

    std::stack<Move>   m_moveHistory;   // 用于悔棋
    std::stack<Move>   m_redoStack;     // 用于重做
    std::string        m_saveFilename;

    // 检查走棋是否合法 (不含将军检测，由 makeMove 综合判断)
    bool isMoveLegal(const Position<int>& from, const Position<int>& to) const;

    // 走棋后己方是否被将军 (走棋违规检测)
    bool wouldKingBeInCheck(const Move& move) const;

    // 执行实际移动 (不检查合法性)
    void executeMove(Move& move);

    // 清空重做栈
    void clearRedoStack();
};

#endif // GAME_H
