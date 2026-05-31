#ifndef CHESSAI_H
#define CHESSAI_H

#include "Game.h"
#include "Position.h"

/*
 * ChessAI —— 传统博弈树搜索 AI
 * ------------------------------------------------
 * 算法：Minimax + Alpha-Beta 剪枝
 * 搜索深度：初级=1, 中级=2, 高级=3
 *
 * 局面评估：
 *   将/帅 = 10000, 车 = 500, 马 = 300, 炮 = 300,
 *   士/仕 = 100, 象/相 = 100, 兵/卒 = 70
 *   score = 红方棋子总价值 - 黑方棋子总价值
 *   AI 执黑 → 选得分最低的走法（Min 层）
 */

// 难度级别
enum class AIDifficulty { Easy, Medium, Hard };

class ChessAI {
public:
    ChessAI(Color aiColor = Color::BLACK);

    void setDifficulty(AIDifficulty d) { m_difficulty = d; }
    AIDifficulty difficulty() const { return m_difficulty; }

    // 计算最佳走法，返回 (from, to)
    // 如果无合法走法（将死/困毙）返回空 optional
    struct AIMove {
        Position<int> from;
        Position<int> to;
    };
    AIMove findBestMove(Game& game);

    // 静态评估函数（公开，方便调试）
    static int evaluateBoard(const Board& board);

private:
    // 棋子价值表
    static int pieceValue(PieceType type);

    // 获取某方的所有合法走法（已过滤被将军的）
    struct ScoredMove {
        Position<int> from;
        Position<int> to;
    };
    std::vector<ScoredMove> getLegalMoves(Game& game, Color color);

    // Minimax + Alpha-Beta
    int minimax(Game& game, int depth, bool isMaxPlayer,
                int alpha, int beta);

    Color m_aiColor;                          // AI 执哪一方
    AIDifficulty m_difficulty = AIDifficulty::Medium;  // 默认中等
};

#endif // CHESSAI_H
