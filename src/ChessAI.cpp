#include "ChessAI.h"
#include <algorithm>
#include <limits>

ChessAI::ChessAI(Color aiColor)
    : m_aiColor(aiColor) {}

// =========================================================================
//  第一步：局面评估函数 evaluateBoard()
// =========================================================================
//
// 这是 AI 的"眼睛"——让 AI 能判断当前局面谁优谁劣。
//
// 核心思路：给每种棋子赋一个价值分数，然后把棋盘上所有存活棋子的
// 分数加起来。假设：
//
//   棋盘总分 = 红方棋子价值总和 - 黑方棋子价值总和
//
//   总分 > 0 → 红方优势
//   总分 < 0 → 黑方优势
//   总分 = 0 → 均势
//
// AI 执黑 → AI 希望总分越小越好（越负数越有利于黑方）。
// AI 执红 → AI 希望总分越大越好（越正数越有利于红方）。
//
// 棋子价值设定原则：
//   将/帅 = 10000  因为"将被吃=游戏结束"，必须给极高权重
//   车   = 500     最强攻击棋子，控制整个横纵线
//   马   = 300     机动性强但有"蹩脚"限制
//   炮   = 300     需要有"炮架"才能吃子
//   士/仕 = 100    只能在九宫内斜走，纯防守
//   象/相 = 100    只能在本方半场斜走，纯防守
//   兵/卒 = 70     过河前攻击力弱，过河后增强
//
// =========================================================================

int ChessAI::pieceValue(PieceType type) {
    switch (type) {
        case PieceType::KING:    return 10000;   // 将/帅——无价之宝
        case PieceType::ROOK:    return 500;      // 车——横冲直撞
        case PieceType::KNIGHT:  return 300;      // 马——灵活机敏
        case PieceType::CANNON:  return 300;      // 炮——隔山打牛
        case PieceType::ADVISOR: return 100;      // 士/仕——贴身护卫
        case PieceType::ELEPHANT: return 100;     // 象/相——防守屏障
        case PieceType::PAWN:    return 70;       // 兵/卒——勇往直前
        default:                 return 0;
    }
}

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
    return score;   // 正=红优，负=黑优
}

// =========================================================================
//  第二步：生成合法走法列表 getLegalMoves()
// =========================================================================
//
// 搜索的第一步，就是列出当前局面下某个颜色的所有合法走法。
//
// 一个"合法走法"满足两个条件：
//   1. 棋子本身的走法规则允许（getValidMoves）
//   2. 走完后己方不会被将军（wouldKingBeInCheck 过滤）
//
// 为什么第2条需要单独过滤？因为有些走法虽然符合棋子规则，
// 但走完后会让自己的将帅暴露在对方攻击范围内（"送将"），
// 这是违规的，必须排除。
//
// =========================================================================

std::vector<ChessAI::ScoredMove> ChessAI::getLegalMoves(Game& game, Color color) {
    std::vector<ScoredMove> moves;
    const Board& board = game.getBoard();

    // 遍历棋盘的 10×9 每个格子
    for (int r = 0; r < Board::ROWS; r++) {
        for (int c = 0; c < Board::COLS; c++) {
            ChessPiece* piece = board.getPieceAt({r, c});

            // 不是该颜色的棋子，跳过
            if (!piece || piece->getColor() != color) continue;

            Position<int> from(r, c);

            // 调用棋子的 getValidMoves 获取它"理论"上能走的位置
            auto candidates = piece->getValidMoves(board);

            // 对每个候选位置，检查走完后己方是否被将军
            for (const auto& to : candidates) {
                // 构造一个 Move 对象用于模拟
                Game::Move m(from, to);
                ChessPiece* moved = board.getPieceAt(from);
                m.captured = board.getPieceAt(to);   // 被吃的棋子（可能为 null）
                m.movedPiece = moved;

                // 最后一步检查：走完后是否被将军？
                if (!game.wouldKingBeInCheck(m))
                    moves.push_back({from, to});
            }
        }
    }
    return moves;
}

// =========================================================================
//  第三步：Minimax 搜索 + Alpha-Beta 剪枝
// =========================================================================
//
// 【3.1 博弈树是什么？】
//
//   把游戏想象成一棵树：
//
//            ┌─── 当前局面 ───┐          ← 根节点（轮到 AI 走）
//           /       |         \
//      走法A     走法B      走法C        ← AI 的三种选择
//       /         |           \
//   对手应对1  对手应对1    对手应对1      ← 对手的应对
//    /  \      /  \        /  \
//    ...（继续往下展开）...
//
//   树的每一层交替代表"AI 走棋"和"对手走棋"。
//   最底层的叶子节点 = 不再展开的局面，用 evaluateBoard() 打分。
//
//
// 【3.2 Minimax 原理】
//
//   - Max 层（AI 走棋）：AI 会选择"得分最高"的走法
//     → 因为 AI 想最大化自己的优势
//
//   - Min 层（对手走棋）：对手会选择"得分最低"的走法
//     → 因为对手也想最大化他自己的优势（即最小化 AI 的优势）
//
//   AI 执黑时：AI = Min层，对手(红方) = Max层。
//   AI 执红时：AI = Max层，对手(黑方) = Min层。
//
//   举例（AI 执黑，depth=2）：
//
//            当前局面（AI=黑方走）            ← Min 层
//           /        \
//       走法A        走法B                    ← 黑方的两种选择
//       /              \
//    红方应对A1        红方应对B1             ← Max 层
//   得分 = -350       得分 = +200
//
//   Min 层选择：min(-350, +200) = -350 → 选走法 A（对黑方最有利）
//
//
// 【3.3 Alpha-Beta 剪枝——如何砍掉无用分支？】
//
//   想象你是 AI（黑方），面前有 3 步可选走法。你开始逐一分析：
//
//   走法1 → 对手反击后得分 = -500（对你很有优势！）
//   走法2 → 对手反击后得分 = +100（对手有优势，你不想要）
//   走法3 → 还没分析…
//
//   但你已经知道走法1得分 -500，走法2得分 +100。
//   你是黑方，当然选最低分，所以当前最好 = min(-500, +100) = -500。
//   你的目标是让最终得分 ≤ -500。
//
//   现在分析走法3的第一步，对手有 N 种回应：
//     对手回应3-1 → 得分 = -200（还没到 -500，继续看）
//     对手回应3-2 → 得分 = +300 > your_best!
//
//   等等——得分 +300 已经 > -500 了！
//   既然对手（Max层）想选最大，他肯定会选 ≥ +300 的走法，
//   那你（Min层）选走法3的最终得分必定 ≥ +300 > -500。
//   既然不可能打破 -500 的记录，走法3剩下的对手回应 3-3, 3-4…
//   就完全没有必要再算了——直接跳过！
//
//   这就是 Alpha-Beta 剪枝的核心思想：
//     如果某个分支已经不可能产生比当前已知最优解更好的结果，
//     就立即停止搜索这个分支，节省计算量。
//
//   参数解读：
//     alpha = "当前 Max 层已经保证能拿到的下界"（分数不会低于这个值）
//     beta  = "当前 Min 层已经保证能拿到的上界"（分数不会高于这个值）
//     剪枝条件：alpha >= beta
//
//
// 【3.4 伪代码对应】
//
//   int minimax(depth, isMaxPlayer, alpha, beta):
//       if (depth == 0) return evaluateBoard()
//
//       if (isMaxPlayer):   // 红方——选最大
//           maxScore = -∞
//           for each move:
//               makeMove()
//               score = minimax(depth-1, false, alpha, beta)
//               undoMove()
//               maxScore = max(maxScore, score)
//               alpha    = max(alpha, maxScore)    // 刷新下界
//               if (alpha >= beta) break           // 剪枝！
//           return maxScore
//
//       else:               // 黑方——选最小
//           minScore = +∞
//           for each move:
//               makeMove()
//               score = minimax(depth-1, true, alpha, beta)
//               undoMove()
//               minScore = min(minScore, score)
//               beta     = min(beta, minScore)     // 刷新上界
//               if (alpha >= beta) break           // 剪枝！
//           return minScore
//
// =========================================================================

int ChessAI::minimax(Game& game, int depth, bool isMaxPlayer,
                     int alpha, int beta) {

    // ---- 终止条件：到达叶子节点 ----
    // depth=0 表示不再往下搜索，直接评估当前局面
    if (depth == 0)
        return evaluateBoard(game.getBoard());

    // 当前由谁走棋
    Color currentColor = isMaxPlayer ? Color::RED : Color::BLACK;

    // 生成当前方的所有合法走法
    auto moves = getLegalMoves(game, currentColor);

    // ---- 将死/困毙检测 ----
    // 无合法走法 → 当前方被将死或困毙，返回极大/极小值
    if (moves.empty()) {
        // Max层无步可走 = 红方被将死 → 返回极小值（对红方最不利）
        // Min层无步可走 = 黑方被将死 → 返回极大值（对黑方最不利）
        return isMaxPlayer ? -99999 : 99999;
    }

    // ---- Max 层（红方 / AI 执红时的走棋方） ----
    if (isMaxPlayer) {
        int maxScore = std::numeric_limits<int>::min();  // 初始化为 -∞

        for (const auto& mv : moves) {
            // 1. 模拟走这一步
            game.makeMove(mv.from, mv.to);

            // 2. 递归搜索：轮到 Min 层（黑方），深度-1
            int score = minimax(game, depth - 1, false, alpha, beta);

            // 3. 撤销走棋，还原棋盘
            game.undo();

            // 4. 更新当前最优得分
            if (score > maxScore) maxScore = score;

            // 5. Alpha-Beta 剪枝
            //    用当前最大值刷新 alpha（下界）
            if (maxScore > alpha) alpha = maxScore;
            //    如果下界已经 >= 上界，剩下的分支不用看了
            if (alpha >= beta) break;
        }
        return maxScore;
    }

    // ---- Min 层（黑方 / AI 执黑时的走棋方） ----
    else {
        int minScore = std::numeric_limits<int>::max();  // 初始化为 +∞

        for (const auto& mv : moves) {
            // 1. 模拟走这一步
            game.makeMove(mv.from, mv.to);

            // 2. 递归搜索：轮到 Max 层（红方），深度-1
            int score = minimax(game, depth - 1, true, alpha, beta);

            // 3. 撤销走棋
            game.undo();

            // 4. 更新当前最优得分
            if (score < minScore) minScore = score;

            // 5. Alpha-Beta 剪枝
            //    用当前最小值刷新 beta（上界）
            if (minScore < beta) beta = minScore;
            //    如果下界已经 >= 上界，剩下的分支不用看了
            if (alpha >= beta) break;
        }
        return minScore;
    }
}

// =========================================================================
//  第四步：主入口 findBestMove() —— 选出最优走法
// =========================================================================
//
// 流程：
//   1. 根据难度确定搜索深度
//   2. 列出当前的所有合法走法
//   3. 对每个走法，调用 minimax 计算走完后的局面得分
//   4. 选得分最高（Max层）或最低（Min层）的走法返回
//
// 时间复杂度分析（近似）：
//   假设每个局面平均有 N 个合法走法（= 分支因子）：
//     depth=1：检查约 N 个走法
//     depth=2：约 N + N² 个节点（Alpha-Beta 剪枝后约 N + N√N）
//     depth=3：约 N + N² + N³ 个节点（剪枝后约 N + √N·N）
//
//   中国象棋分支因子 N 通常在 30~50 之间。
//   depth=2 时一般几十毫秒，depth=3 时一般几百毫秒。
//
// =========================================================================

ChessAI::AIMove ChessAI::findBestMove(Game& game) {

    // 第一步：确定搜索深度
    int depth;
    switch (m_difficulty) {
        case AIDifficulty::Easy:   depth = 1; break;  // 只算一步，只看眼前
        case AIDifficulty::Medium: depth = 2; break;  // 算两步，考虑对手应对
        case AIDifficulty::Hard:   depth = 3; break;  // 算三步，更深前瞻
        default:                   depth = 2; break;
    }

    // 第二步：判断当前是 Max 层还是 Min 层
    // AI 执红 → isMax=true（选最大分）
    // AI 执黑 → isMax=false（选最小分）
    Color currentColor = game.getCurrentPlayer();
    bool isMax = (currentColor == Color::RED);

    // 第三步：列出当前所有合法走法
    auto moves = getLegalMoves(game, currentColor);

    // 第四步：初始化最优解
    AIMove best;
    best.from = {-1, -1};  // 无效值——如果没有合法走法则返回这个
    best.to   = {-1, -1};

    int bestScore = isMax ? std::numeric_limits<int>::min()
                          : std::numeric_limits<int>::max();
    // Max层初始化为 -∞（等找到更大的就替换）
    // Min层初始化为 +∞（等找到更小的就替换）

    // 第五步：遍历每个候选走法，用 minimax 评估
    for (const auto& mv : moves) {
        // 模拟走棋
        game.makeMove(mv.from, mv.to);

        // 递归搜索评估得分
        // 注意：深度 -1，下一层切换角色
        int score = minimax(game, depth - 1, !isMax,
                            std::numeric_limits<int>::min(),  // 初始 alpha = -∞
                            std::numeric_limits<int>::max()); // 初始 beta  = +∞

        // 撤销模拟
        game.undo();

        // 更新最优解
        if (isMax) {
            // AI 执红：选最高分
            if (score > bestScore) {
                bestScore = score;
                best.from = mv.from;
                best.to   = mv.to;
            }
        } else {
            // AI 执黑：选最低分
            if (score < bestScore) {
                bestScore = score;
                best.from = mv.from;
                best.to   = mv.to;
            }
        }
    }

    return best;
}
