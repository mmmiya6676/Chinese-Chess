#include "ChessAI.h"
#include <algorithm>
#include <limits>

ChessAI::ChessAI(Color aiColor)
    : m_aiColor(aiColor) {}

// =========================== 棋子估值 ===========================

/*
 * 棋子基础价值表。
 * 将/帅给极高值，确保 AI 绝不会忽略被将军的威胁。
 */
int ChessAI::pieceValue(PieceType type) {
    switch (type) {
        case PieceType::KING:    return 10000;
        case PieceType::ROOK:    return 500;
        case PieceType::KNIGHT:  return 300;
        case PieceType::CANNON:  return 300;
        case PieceType::ADVISOR: return 100;
        case PieceType::ELEPHANT: return 100;
        case PieceType::PAWN:    return 70;
        default:                 return 0;
    }
}

/*
 * 局面静态评估函数
 *
 *   遍历棋盘上所有存活棋子，累加红方价值 - 黑方价值。
 *   正数 = 红方优势，负数 = 黑方优势。
 *
 *   AI 执黑时作为 Min 层，选择得分最低的走法（ = 最有利于黑方）。
 *   如果 AI 执红则作为 Max 层。
 */
int ChessAI::evaluateBoard(const Board& board) {
    int score = 0;
    for (int r = 0; r < Board::ROWS; r++) {
        for (int c = 0; c < Board::COLS; c++) {
            ChessPiece* p = board.getPieceAt({r, c});
            if (!p) continue;
            int val = pieceValue(p->getType());
            score += (p->getColor() == Color::RED) ? val : -val;
        }
    }
    return score;
}

// =========================== 合法走法生成 ===========================

/*
 * 获取某方的所有合法走法（已过滤走后被将军的）
 *
 * 步骤：
 *   1. 遍历棋盘所有位置
 *   2. 找到该颜色的棋子 → 调用 getValidMoves()
 *   3. 对每个候选走法，用 wouldKingBeInCheck 过滤掉"送将"的违规走法
 *
 * Game 的 makeMove/undo 机制在这里不够高效（每次走棋都会拷贝棋盘），
 * 但深度 3 的搜索量可接受（最多 ~40^3 ≈ 64000 节点，剪枝后更少）。
 */
std::vector<ChessAI::ScoredMove> ChessAI::getLegalMoves(Game& game, Color color) {
    std::vector<ScoredMove> moves;
    const Board& board = game.getBoard();

    for (int r = 0; r < Board::ROWS; r++) {
        for (int c = 0; c < Board::COLS; c++) {
            ChessPiece* piece = board.getPieceAt({r, c});
            if (!piece || piece->getColor() != color) continue;

            Position<int> from(r, c);
            auto candidates = piece->getValidMoves(board);
            for (const auto& to : candidates) {
                // 构造 Move 结构，调用 wouldKingBeInCheck 过滤
                Game::Move m(from, to);
                ChessPiece* moved = board.getPieceAt(from);
                m.captured = board.getPieceAt(to);
                m.movedPiece = moved;
                if (!game.wouldKingBeInCheck(m))
                    moves.push_back({from, to});
            }
        }
    }
    return moves;
}

// =========================== Minimax + Alpha-Beta ===========================

/*
 * Alpha-Beta 剪枝版 Minimax 搜索
 *
 * 参数：
 *   game       — 当前游戏状态（会被临时修改，离开函数时还原）
 *   depth      — 剩余搜索深度（0 = 直接返回静态评估）
 *   isMaxPlayer — true = Max 层（选最大得分），false = Min 层（选最小得分）
 *   alpha      — 当前已知最好下界（-inf = 尚无）
 *   beta       — 当前已知最好上界（+inf = 尚无）
 *
 * 剪枝条件：当 alpha >= beta 时立即返回，不再搜索兄弟节点。
 *
 *   例：Min 层发现一个走法得分 = 3，更新 beta = 3。
 *       Max 层祖父节点已知 alpha = 5。
 *       因为 3 < 5，Min 父节点不可能选出 >5 的走法，
 *       继续搜索已无意义 → 剪枝。
 *
 * AI 执黑 → AI 是 Min 层（选最低分=最有利于黑方）
 * AI 执红 → AI 是 Max 层（选最高分=最有利于红方）
 */
int ChessAI::minimax(Game& game, int depth, bool isMaxPlayer,
                     int alpha, int beta) {
    // 叶子节点：直接返回静态评估
    if (depth == 0)
        return evaluateBoard(game.getBoard());

    Color currentColor = isMaxPlayer ? Color::RED : Color::BLACK;
    auto moves = getLegalMoves(game, currentColor);

    // 无合法走法 = 将死或困毙，返回极值
    if (moves.empty()) {
        // 当前玩家无子可走 → 对手获胜
        return isMaxPlayer ? -99999 : 99999;
    }

    if (isMaxPlayer) {
        // Max 层（红方）：选择最大得分
        int maxScore = std::numeric_limits<int>::min();
        for (const auto& mv : moves) {
            game.makeMove(mv.from, mv.to);      // 执行走棋
            int score = minimax(game, depth - 1, false, alpha, beta);
            game.undo();                         // 撤销走棋

            if (score > maxScore) maxScore = score;

            // Alpha-Beta 剪枝
            if (maxScore > alpha) alpha = maxScore;
            if (alpha >= beta) break;            // 剪枝！
        }
        return maxScore;
    } else {
        // Min 层（黑方）：选择最小得分
        int minScore = std::numeric_limits<int>::max();
        for (const auto& mv : moves) {
            game.makeMove(mv.from, mv.to);
            int score = minimax(game, depth - 1, true, alpha, beta);
            game.undo();

            if (score < minScore) minScore = score;

            // Alpha-Beta 剪枝
            if (minScore < beta) beta = minScore;
            if (alpha >= beta) break;            // 剪枝！
        }
        return minScore;
    }
}

// =========================== 主入口 ===========================

/*
 * findBestMove —— AI 计算最佳走法
 *
 * 根据难度级别确定搜索深度：
 *   初级 (Easy)   : depth = 1（只算一步，选当前最优）
 *   中级 (Medium) : depth = 2（预报对手应对）
 *   高级 (Hard)   : depth = 3（三步前瞻）
 *
 * 返回最佳走法的起止坐标。
 * 如果有多个相同最优得分的走法，取第一个（可改为随机选择增加趣味性）。
 */
ChessAI::AIMove ChessAI::findBestMove(Game& game) {
    int depth;
    switch (m_difficulty) {
        case AIDifficulty::Easy:   depth = 1; break;
        case AIDifficulty::Medium: depth = 2; break;
        case AIDifficulty::Hard:   depth = 3; break;
        default:                   depth = 2; break;
    }

    Color currentColor = game.getCurrentPlayer();
    bool isMax = (currentColor == Color::RED);  // AI 执红 = Max 层
    auto moves = getLegalMoves(game, currentColor);

    AIMove best;
    best.from = {-1, -1};  // 无合法走法时返回无效值
    best.to   = {-1, -1};

    int bestScore = isMax ? std::numeric_limits<int>::min()
                          : std::numeric_limits<int>::max();

    for (const auto& mv : moves) {
        game.makeMove(mv.from, mv.to);
        int score = minimax(game, depth - 1, !isMax,
                            std::numeric_limits<int>::min(),
                            std::numeric_limits<int>::max());
        game.undo();

        if (isMax) {
            if (score > bestScore) {
                bestScore = score;
                best.from = mv.from;
                best.to   = mv.to;
            }
        } else {
            if (score < bestScore) {
                bestScore = score;
                best.from = mv.from;
                best.to   = mv.to;
            }
        }
    }

    return best;
}
