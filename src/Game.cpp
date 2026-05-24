#include"../include/Game.h"
#include <sstream>
#include"../include/King.h"
#include"../include/Advisor.h"
#include"../include/Elephant.h"
#include"../include/Knight.h"
#include"../include/Rook.h"
#include"../include/Cannon.h"
#include"../include/Pawn.h"
using namespace std;
//游戏初始化，执棋子方为红
Game::Game():m_currentPlayer(Color::RED),m_moveCount(0),m_gameOver(false),m_winner(Color::RED){
    m_board.initialize();
}

Game::Move::Move(const Position<int>& from, const Position<int>& to)
:from(from),to(to),captured(nullptr),movedPiece(nullptr){
}

void Game::start(){
    restart();
}

void Game::restart(){
    m_board.initialize();
    m_currentPlayer=Color::RED;
    m_moveCount=0;
    m_gameOver=false;
    m_winner=Color::RED;
    while(!m_moveHistory.empty()){
        m_moveHistory.pop();
    }
    while(!m_redoStack.empty()){
        m_redoStack.pop();
    }
    //清空两个栈
}

//以下是get函数
Color Game::getCurrentPlayer()const{
    return m_currentPlayer;
}
int Game::getMoveCount()const{
    return m_moveCount;
}
bool Game::isGameOver()const{
    return m_gameOver;
}
Color Game::getWinner()const{
    return m_winner;
}
bool Game::canUndo()const{
    return !m_moveHistory.empty();
}
bool Game::canRedo()const{
    return !m_redoStack.empty();
}
ChessPiece* Game::getPieceAt(const Position<int>& pos)const{
    return m_board.getPieceAt(pos);
}
Board& Game::getBoard(){
    return m_board;
}
const Board& Game::getBoard()const{
    return m_board;
}
void Game::setSaveFilename(const std::string& name){
    m_saveFilename = name;
}
std::string Game::getSaveFilename() const{
    return m_saveFilename;
}

//切换走棋方
void Game::switchPlayer(){
    m_currentPlayer = (m_currentPlayer == Color::RED) ? Color::BLACK : Color::RED;
}

//清空重做栈
void Game::clearRedoStack(){
    while(!m_redoStack.empty()){
        m_redoStack.pop();
    }
}

//执行移动
void Game::executeMove(Move& move){
    move.movedPiece=m_board.getPieceAt(move.from);
    move.captured=m_board.movePiece(move.from,move.to);
}

//检查移动是否合法
bool Game::isMoveLegal(const Position<int>&from,const Position<int>&to)const{
    if(m_board.getPieceAt(from)->getColor()!=m_currentPlayer){
        return false;
    }//棋子是己方的吗？
    if(!m_board.isPositionValid(from)){
        return false;
    }//位置合理吗
    if(!m_board.isPositionValid(to)){
        return false;
    }
    if(m_board.getPieceAt(from)==nullptr){
        return false;
    }//真有东西吗
    if(!m_board.getPieceAt(from)->canMoveTo(to,m_board)){
        return false;
    }//能合理移动到那里吗
    return true;
}

//检查移动是否会导致国王被吃
bool Game::wouldKingBeInCheck(const Move& move)const{
    //复制棋盘状态
    Board tempBoard(m_board);
    ChessPiece* movedPiece = tempBoard.getPieceAt(move.from);
    ChessPiece* capturedPiece = tempBoard.movePiece(move.from, move.to);
    return tempBoard.isKingInCheck(m_currentPlayer);
}
//执行移动
bool Game::makeMove(const Position<int>&from,const Position<int>&to){
    if(isGameOver())return false;//游戏结束了没
    if(!isMoveLegal(from,to))return false;//移动合法吗
    Move temp(from,to);
    if(wouldKingBeInCheck(temp)){
        return false;
    }//是否送将
    executeMove(temp);
    m_moveHistory.push(temp);
    //检查是否游戏结束
    if(m_board.isCheckmate(m_currentPlayer==Color::RED?Color::BLACK:Color::RED)){
        m_gameOver=true;
        m_winner=m_currentPlayer;
    }
    clearRedoStack();
    switchPlayer();
    m_moveCount++;
    return true;
}

// ========== 中文记谱法辅助函数 ==========
/*
 * 判断一个字节属于几字节的 UTF-8 字符。
 *  UTF-8 编码规则（看首字节的高位bit）：
 *    0xxxxxxx          → 1字节（ASCII，如 'A' = 65）
 *    110xxxxx          → 2字节（罕见符号）
 *    1110xxxx          → 3字节（中文汉字全在这里，如 "马"）
 *    11110xxx          → 4字节（emoji）
 *  本函数只处理到3字节（中文够用），其余都返回3。
 *  用法：拿到字符串首字节 → 知道要切几个字节 → 切出完整中文字。
 */
static int utf8ByteLen(unsigned char c) {
    // 0x80 = 10000000，检查最高位是否为0（即ASCII）
    if ((c & 0x80) == 0) return 1;
    // 0xE0 = 11100000，0xC0 = 11000000，检查高三位是否为110
    if ((c & 0xE0) == 0xC0) return 2;
    // 剩下就是1110开头 = 3字节中文
    return 3;
}

/*
 * 从字符串 s 的第 pos 个字节开始，提取一个完整的 UTF-8 字符。
 * 提取后 pos 自动跳到下一个字符的开头位置。
 * 例："马三进四" → 第1次调用取"马"，pos从0跳到3；第2次取"三"，pos从3跳到6...
 */
static std::string nextChar(const std::string& s, size_t& pos) {
    if (pos >= s.size()) return "";
    // 看首字节，判断这个字符占几个字节
    int len = utf8ByteLen(static_cast<unsigned char>(s[pos]));
    // 切出完整字符（中文3字节、数字1字节）
    std::string ch = s.substr(pos, len);
    pos += len;  // 下标前进到下一个字符
    return ch;
}

/*
 * 记谱法的列号 → 棋盘上的列号。
 *  记谱法中列号从己方右侧数起（1到9）。
 *  但棋盘列号0~8是固定从左到右的。
 *  红方：右侧=棋盘列8 → 公式 9-N（如"二"→9-2=7）
 *  黑方：右侧=棋盘列0 → 公式 N-1（如"2"→2-1=1）
 */
static int notationColToBoard(int notationCol, Color player) {
    return (player == Color::RED) ? (9 - notationCol) : (notationCol - 1);
}

/*
 * 把单个数字字符转成整数1~9。
 *  支持中文数字："一"→1、"二"→2 ... "九"→9
 *  也支持阿拉伯数字："1"→1、"2"→2 ... "9"→9
 *  因为红方用中文数字，黑方用阿拉伯数字。
 */
static int numToInt(const std::string& s) {
    static const std::string table[] = {
        "", "一", "二", "三", "四", "五",
        "六", "七", "八", "九"
    };
    for (int i = 1; i <= 9; ++i) {
        // 同时匹配中文数字和阿拉伯数字
        if (s == table[i] || s == std::to_string(i)) return i;
    }
    return -1;  // 不是1~9
}

/*
 * 棋子中文字符 → PieceType 枚举。
 *  红方用"帅""仕""相""兵"，黑方用"将""士""象""卒"。
 *  "马""车""炮"双方通用。
 */
static PieceType symbolToType(const std::string& s) {
    if (s == "帅" || s == "将") return PieceType::KING;
    if (s == "仕" || s == "士") return PieceType::ADVISOR;
    if (s == "相" || s == "象") return PieceType::ELEPHANT;
    if (s == "马")                  return PieceType::KNIGHT;
    if (s == "车")                  return PieceType::ROOK;
    if (s == "炮")                  return PieceType::CANNON;
    if (s == "兵" || s == "卒") return PieceType::PAWN;
    return PieceType::KING; // 理论上不会执行到这里
}

/*
 * 中文记谱法走棋。输入如 "炮二平五"、"马三进四"、"前兵进一"。
 * 记谱法格式：[棋子名][当前列号][方向][步数或目标列号]
 *
 * 解析步骤：
 *  1. 把输入切成4个字符：棋子、列号、方向、参数
 *  2. 确定棋子类型和移动方向
 *  3. 在当前方找到对应列号上的棋子（支持"前"/"后"区分同列棋子）
 *  4. 根据方向计算目标坐标：
 *     - 平：换列不换行
 *     - 进/退 + 直线棋子（车炮帅兵）：沿列前进/后退N步
 *     - 进/退 + 斜线棋子（马相仕）：到目标列号，自动匹配合法走法
 *  5. 调用坐标版 makeMove 执行
 */
bool Game::makeMove(const std::string& notation) {
    if (notation.empty()) return false;

    // ===== 第1步：把输入切成4个字符 =====
    // 例："炮二平五" → pieceStr="炮" colStr="二" dirStr="平" amountStr="五"
    size_t pos = 0;
    std::string pieceStr  = nextChar(notation, pos);  // 棋子名
    std::string colStr    = nextChar(notation, pos);  // 列号（或"前"/"后"）
    std::string dirStr    = nextChar(notation, pos);  // 方向："进""退""平"
    std::string amountStr = nextChar(notation, pos);  // 目标列号或步数
    if (pieceStr.empty() || colStr.empty() || dirStr.empty() || amountStr.empty())
        return false;

    // ===== 第2步：确定棋子类型 =====
    PieceType pieceType = symbolToType(pieceStr);
    bool useFront = (pieceStr == "前");
    bool useBack  = (pieceStr == "后");
    if(useFront || useBack)pieceType = symbolToType(colStr); 
    // ===== 第3步：确定方向 =====
    bool isAdvance = (dirStr == "进");
    bool isRetreat = (dirStr == "退");
    bool isHorizontal = (dirStr == "平");
    if (!isAdvance && !isRetreat && !isHorizontal) return false;

    // ===== 第4步：解析目标参数（整数1~9） =====
    int amount = numToInt(amountStr);
    if (amount < 1 || amount > 9) return false;

    // ===== 第5步：找到要移动的棋子 =====
    // 列号可能是数字（1~9）或"前""后"（同列有多个同种棋子时）
    
    int notationCol = useFront ? -1 : numToInt(colStr);
    if (!useFront && !useBack && (notationCol < 1 || notationCol > 9))
        return false;

    // 收集当前方所有该类型的存活棋子
    std::vector<ChessPiece*> candidates;
    for (auto* p : m_board.getPieces(m_currentPlayer)) {
        if (p->getType() == pieceType && p->isAlive())
            candidates.push_back(p);
    }
    if (candidates.empty()) return false;

    ChessPiece* selected = nullptr;

    if (useFront || useBack) {
        // ===== 处理"前"/"后"：在同列的多个同种棋子中选出前或后 =====
        if (candidates.size() < 2) return false;

        // 先找哪一列上有2个以上同类型棋子
        int col = -1;
        for (size_t i = 0; i < candidates.size(); ++i) {
            for (size_t j = i + 1; j < candidates.size(); ++j) {
                if (candidates[i]->getPosition().getY() ==
                    candidates[j]->getPosition().getY()) {
                    col = candidates[i]->getPosition().getY();
                    break;
                }
            }
            if (col >= 0) break;
        }
        if (col < 0) return false;  // 没有同列的，不能用"前"/"后"

        // 在该列中找出最前和最后的棋子
        // "前"=更靠近对方半场：红方x越小越靠前，黑方x越大越靠前
        ChessPiece *frontPiece = nullptr, *backPiece = nullptr;
        int bestFront = (m_currentPlayer == Color::RED) ? 999 : -1;
        int bestBack  = (m_currentPlayer == Color::RED) ? -1 : 999;
        for (auto* p : candidates) {
            if (p->getPosition().getY() != col) continue;
            int x = p->getPosition().getX();
            if (m_currentPlayer == Color::RED) {
                // 红方朝上，x越小越靠前（更靠近对方）
                if (x < bestFront) { bestFront = x; frontPiece = p; }
                if (x > bestBack)  { bestBack  = x; backPiece  = p; }
            } else {
                // 黑方朝下，x越大越靠前（更靠近对方）
                if (x > bestFront) { bestFront = x; frontPiece = p; }
                if (x < bestBack)  { bestBack  = x; backPiece  = p; }
            }
        }
        selected = useFront ? frontPiece : backPiece;
    } else {
        // ===== 正常列号：在该列找到对应棋子 =====
        int boardCol = notationColToBoard(notationCol, m_currentPlayer);
        for (auto* p : candidates) {
            if (p->getPosition().getY() == boardCol) {
                selected = p;
                break;
            }
        }
    }
    if (selected == nullptr) return false;

    Position<int> from = selected->getPosition();

    // ===== 第6步：计算目标坐标 =====
    Position<int> to;

    if (isHorizontal) {
        // "平"：同行，换到目标列
        int targetCol = notationColToBoard(amount, m_currentPlayer);
        to = Position<int>(from.getX(), targetCol);
    } else {
        // "进"或"退"：先区分直线棋子还是斜线棋子
        bool isStraight = (pieceType == PieceType::ROOK  ||
                           pieceType == PieceType::CANNON ||
                           pieceType == PieceType::KING  ||
                           pieceType == PieceType::PAWN);
        if (isStraight) {
            // 直线棋子（车炮帅兵）：进/退的参数 = 步数
            // 红方向前 = x-1，黑方向前 = x+1
            int stepDir;
            if (m_currentPlayer == Color::RED)
                stepDir = isAdvance ? -1 : 1;
            else
                stepDir = isAdvance ? 1 : -1;
            to = Position<int>(from.getX() + stepDir * amount, from.getY());
        } else {
            // 斜线棋子（马相仕）：进/退的参数 = 目标列号
            // 在合法走法中找一个：列号匹配且方向（进/退）正确的
            int targetBoardCol = notationColToBoard(amount, m_currentPlayer);
            auto moves = selected->getValidMoves(m_board);
            bool found = false;
            for (const auto& m : moves) {
                if (m.getY() != targetBoardCol) continue;  // 列不对，跳过
                int rowDiff = m.getX() - from.getX();
                // 判断这个走法是"进"（向前）还是"退"（向后）
                bool goesForward = (m_currentPlayer == Color::RED)
                                     ? (rowDiff < 0) : (rowDiff > 0);
                if ((isAdvance && goesForward) || (isRetreat && !goesForward)) {
                    to = m;
                    found = true;
                    break;
                }
            }
            if (!found) return false;  // 没找到匹配的走法
        }
    }

    // ===== 第7步：执行走棋 =====
    return makeMove(from, to);
}

//悔棋功能实现

bool Game::undo(){
    if(!canUndo())return false;
    Move last=m_moveHistory.top();
    m_moveHistory.pop();
    m_board.undoMove(last.from,last.to,last.captured);
    m_redoStack.push(last);
    m_moveCount--;
    switchPlayer();
    m_gameOver=false;
    return true;
}
//重做移动

bool Game::redo(){
    if(!canRedo())return false;
    Move last=m_redoStack.top();
    m_redoStack.pop();
    executeMove(last);
    m_moveCount++;
    m_moveHistory.push(last);
    if(m_board.isCheckmate(m_currentPlayer==Color::RED?Color::BLACK:Color::RED)){
        m_gameOver=true;
        m_winner=m_currentPlayer;
        return true;
    }
    switchPlayer();
    m_gameOver=false;
    return true;
}

//保存游戏状态
bool Game::saveGame(const std::string& filename)const{
    std::ofstream file(filename);
    file<<"==============当前执棋方:"<<
    (m_currentPlayer==Color::RED?"红===============":"黑==============")<<std::endl;
    file<<m_moveCount<<std::endl;
    file<<"当前棋子状态:"<<std::endl;
    file<<"红方:"<<std::endl;
    for(auto it:m_board.getPieces(Color::RED)){
        file<<it->getSymbol()<<" "<<it->getPosition().getX()<<" "<<it->getPosition().getY();
        file<<std::endl;
    }
    file<<"黑方:"<<std::endl;
    for(auto it:m_board.getPieces(Color::BLACK)){
        file<<it->getSymbol()<<" "<<it->getPosition().getX()<<" "<<it->getPosition().getY();
        file<<std::endl;
    }
    // 步法记录
    if(!m_moveHistory.empty()){
        file<<"步法记录:"<<std::endl;
        std::stack<Move> temp = m_moveHistory;
        std::vector<Move> moves;
        while(!temp.empty()){
            moves.push_back(temp.top());
            temp.pop();
        }
        for(auto it = moves.rbegin(); it != moves.rend(); ++it){
            file<<it->from.getX()<<" "<<it->from.getY()<<" "
                <<it->to.getX()<<" "<<it->to.getY()<<std::endl;
        }
    }
    file.close();
    return true;
}
//读取游戏状态
bool Game::loadGame(const std::string& filename){
    m_board.clear();
    std::ifstream file(filename);
    if(!file) return false;

    std::string line;
    std::getline(file,line);
    if(     line=="==============当前执棋方:红==============="){
        m_currentPlayer=Color::RED;
    }
    else if(line=="==============当前执棋方:黑==============="){
        m_currentPlayer=Color::BLACK;
    }
    else{
        return false;
    }
    std::getline(file,line);
    m_moveCount=std::stoi(line);
    std::getline(file,line);
    if(line!="当前棋子状态:"){
        return false;
    }
    std::getline(file,line);
    if(line.find("红方:") != 0){
        return false;
    }

    // 先扫描整个文件，判断是否包含"步法记录:"
    // 如果有步法记录就从初始局面回放，跳过棋子列表解析
    bool hasMoveHistory = false;
    {
        std::streampos savedPos = file.tellg();  // 记住当前位置（红方第一行的开头）
        std::string scanLine;
        while(std::getline(file, scanLine)){
            if(scanLine.rfind("步", 0) == 0){
                hasMoveHistory = true;
                break;
            }
        }
        file.clear();                // 清除 EOF 标志
        file.seekg(savedPos);        // 回到红方第一行
    }

    if(hasMoveHistory){
        // 跳过所有棋子列表行（红方+黑方），直接读步法记录
        while(std::getline(file, line)){
            if(line.rfind("步", 0) == 0) break;
        }

        // 解析步法记录
        std::vector<Move> loadedMoves;
        while(std::getline(file, line)){
            if(line.empty()) break;
            std::istringstream iss(line);
            int fx, fy, tx, ty;
            if(!(iss >> fx >> fy >> tx >> ty)){
                return false;
            }
            loadedMoves.emplace_back(Position<int>(fx, fy), Position<int>(tx, ty));
        }

        // 从初始局面回放所有走法
        m_board.initialize();
        m_currentPlayer = Color::RED;
        m_moveCount = 0;
        m_gameOver = false;
        while(!m_moveHistory.empty()) m_moveHistory.pop();
        while(!m_redoStack.empty()) m_redoStack.pop();
        for(const auto& move : loadedMoves){
            if(!makeMove(move.from, move.to)){
                return false;
            }
        }
    } else {
        // 没有步法记录，按棋子列表解析（兼容旧版存档）
        while(std::getline(file,line)){
            if(line.empty()){
                return false;
            }
            if(line.rfind("黑", 0) == 0){
                break;
            }
            std::istringstream iss(line);
            std::string symbol;
            int x=0, y=0;
            iss >> symbol >> x >> y;
            if(!m_board.isPositionValid(Position<int>(x,y))){
                return false;
            }
            if(m_board.getPieceAt(Position<int>(x,y))!=nullptr){
                return false;
            }
            if(symbol=="炮"){
                m_board.placePiece(new Cannon(Color::RED,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="帅"){
                m_board.placePiece(new King(Color::RED,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="仕"){
                m_board.placePiece(new Advisor(Color::RED,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="马"){
                m_board.placePiece(new Knight(Color::RED,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="相"){
                m_board.placePiece(new Elephant(Color::RED,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="兵"){
                m_board.placePiece(new Pawn(Color::RED,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="车"){
                m_board.placePiece(new Rook(Color::RED,Position<int>(x,y)),Position<int>(x,y));
            }
            else{
                return false;
            }
        }
        while(std::getline(file,line)){
            if(line.empty()){
                return false;
            }
            std::istringstream iss(line);
            std::string symbol;
            int x=0, y=0;
            iss >> symbol >> x >> y;
            if(!m_board.isPositionValid(Position<int>(x,y))){
                return false;
            }
            if(m_board.getPieceAt(Position<int>(x,y))!=nullptr){
                return false;
            }
            if(symbol=="炮"){
                m_board.placePiece(new Cannon(Color::BLACK,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="将"){
                m_board.placePiece(new King(Color::BLACK,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="车"){
                m_board.placePiece(new Rook(Color::BLACK,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="马"){
                m_board.placePiece(new Knight(Color::BLACK,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="卒"){
                m_board.placePiece(new Pawn(Color::BLACK,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="士"){
                m_board.placePiece(new Advisor(Color::BLACK,Position<int>(x,y)),Position<int>(x,y));
            }
            else if(symbol=="象"){
                m_board.placePiece(new Elephant(Color::BLACK,Position<int>(x,y)),Position<int>(x,y));
            }
            else{
                return false;
            }
        }
    }
    file.close();
    return true;
}
//运算符重载
Game& Game::operator+(const Move& move){
    makeMove(move.from,move.to);
    return *this;
}
Game& Game::operator-(int steps){
    for(int i=0;i<steps;i++){
        if(m_moveCount<=0||m_moveHistory.empty()){
            break;
        }
        undo();
    }
    return *this;
}
bool Game::operator==(const Game& other)const{
    if(m_currentPlayer!=other.m_currentPlayer){
        return false;
    }
    if(m_moveCount!=other.m_moveCount){
        return false;
    }
    if(m_gameOver!=other.m_gameOver){
        return false;
    }
    if(m_winner!=other.m_winner){
        return false;
    }
    if(!(m_board==other.m_board)){
        return false;
    }
    return true;
}
std::ostream& operator<<(std::ostream& os, const Game& game){
    os<<"当前执棋方:"<<(game.m_currentPlayer==Color::RED?"红":"黑")<<std::endl;
    os<<"当前移动次数:"<<game.m_moveCount<<std::endl;
    os<<game.m_board;
    return os;
}
std::istream& operator>>(std::istream& is, Game& game){
    int x1,y1,x2,y2;
    if(!(is>>x1>>y1>>x2>>y2))return is;
    if(!game.makeMove(Position<int>(x1,y1),Position<int>(x2,y2)))
        is.setstate(std::ios::failbit);
    return is;
}
