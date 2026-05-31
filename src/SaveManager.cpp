/*
 * SaveManager.cpp — 存档文件管理
 *
 * 负责：
 *  - 扫描 saves/ 目录下的存档文件
 *  - 自动递增 play 编号
 *  - 存档选择菜单
 */

#include "../include/SaveManager.h"
#include <fstream>
#include <algorithm>
#include <map>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

/*
 * 扫描 saves/ 目录下的所有 .txt 文件，返回排序后的文件名列表。
 * 用 C++17 std::filesystem 遍历目录。
 *
 * directory_iterator：遍历目录下的每个条目。
 * 每个条目是 directory_entry，path() 是完整路径，
 * filename() 只取文件名部分。
 */
vector<string> listSaveFiles() {
    vector<string> result;
    // 目录不存在直接返回空
    if (!fs::exists("saves")) return result;

    for (const auto& entry : fs::directory_iterator("saves")) {
        if (entry.path().extension() == ".txt")
            result.push_back(entry.path().filename().string());
    }
    sort(result.begin(), result.end());
    return result;
}

/*
 * 确保目录存在，不存在则创建。
 * create_directory：如果目录已存在什么也不做（不会报错）。
 */
void ensureDir(const string& path) {
    fs::create_directory(path);
}

/*
 * 从文件名中提取 play 后面的数字。
 * 例："张三 vs 李四 play3.txt"
 *   → 找 "play" → 跳过4字节 → 取 "3" → 返回 3
 */
int extractPlayNumber(const string& filename) {
    size_t p = filename.rfind("play");
    if (p == string::npos) return -1;
    p += 4;
    size_t dot = filename.find('.', p);
    string numStr = filename.substr(p, dot - p);
    try { return stoi(numStr); }
    catch (...) { return -1; }
}

// ============================================================
//  玩家注册表（名字 → 数字ID，解决中文文件名编码问题）
// ============================================================

static const string PLAYERS_FILE = "saves/players.dat";

static map<int, string> loadPlayerRegistry() {
    map<int, string> result;
    ifstream file(PLAYERS_FILE);
    if (!file) return result;
    string line;
    while (getline(file, line)) {
        size_t colon = line.find(':');
        if (colon != string::npos) {
            int id = stoi(line.substr(0, colon));
            result[id] = line.substr(colon + 1);
        }
    }
    return result;
}
// 写玩家注册表，格式：每行 "ID:Name"
static void savePlayerRegistry(const map<int, string>& reg) {
    ofstream file(PLAYERS_FILE);
    for (auto& [id, name] : reg)
        file << id << ":" << name << "\n";
}

int getOrCreatePlayerID(const string& name) {
    ensureDir("saves");
    auto reg = loadPlayerRegistry();
    //原排行榜有记录
    for (auto& [id, n] : reg)
        if (n == name) return id;
    //新玩家，分配新ID
    int newID = 1;
    while (reg.count(newID)) newID++;
    reg[newID] = name;
    savePlayerRegistry(reg);
    return newID;
}

string buildSaveFilename(int redID, int blackID, int playN) {
    return "saves/" + to_string(redID) + "_vs_" + to_string(blackID)
           + "_play" + to_string(playN) + ".txt";
}

int nextPlayNumber(int redID, int blackID) {
    string prefix = to_string(redID) + "_vs_" + to_string(blackID) + "_play";
    int maxN = -1;
    for (const auto& f : listSaveFiles()) {
        if (f.find(prefix) == 0) {
            int n = extractPlayNumber(f);
            if (n > maxN) maxN = n;
        }
    }
    return maxN + 1;
}

// ============================================================
//  排行榜
// ============================================================

// 排行榜文件路径
static const string LEADERBOARD_FILE = "saves/leaderboard.txt";

// 单个玩家的排行榜记录
struct PlayerRecord {
    string name;
    int    wins  = 0;
    int    total = 0;
};

// 读排行榜（按行读取，从行尾反向解析数字，名字含空格/中文都不影响）
static vector<PlayerRecord> loadLeaderboard() {
    vector<PlayerRecord> result;
    ifstream file(LEADERBOARD_FILE);
    if (!file) return result;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        // 从行尾找最后一个空格 → total
        size_t sp2 = line.rfind(' ');
        if (sp2 == string::npos) continue;
        // 从 total 前面找倒数第二个空格 → wins
        size_t sp1 = line.rfind(' ', sp2 - 1);
        if (sp1 == string::npos) continue;
        string name  = line.substr(0, sp1);
        int wins     = stoi(line.substr(sp1 + 1, sp2 - sp1 - 1));
        int total    = stoi(line.substr(sp2 + 1));
        if (!name.empty())
            result.push_back({name, wins, total});
    }
    return result;
}

// 写排行榜
static void saveLeaderboard(const vector<PlayerRecord>& data) {
    ofstream file(LEADERBOARD_FILE);
    for (const auto& r : data)
        file << r.name << " " << r.wins << " " << r.total << "\n";
}

// 更新或添加某个玩家的记录
static void updatePlayer(vector<PlayerRecord>& data,
                          const string& name, bool isWin) {
    for (auto& r : data) {
        if (r.name == name) {
            r.total += 1;
            if (isWin) r.wins += 1;
            return;
        }
    }
    // 新玩家
    data.push_back({name, isWin ? 1 : 0, 1});
}

void recordGameResult(const string& red, const string& black,
                      const string& winner) {
    ensureDir("saves");
    auto data = loadLeaderboard();
    bool redWin = (winner == "红" || winner == "RED");
    updatePlayer(data, red,   redWin);
    updatePlayer(data, black, !redWin);
    saveLeaderboard(data);
}

// Qt 可用的排行榜数据
vector<LeaderboardEntry> loadLeaderboardData() {
    auto raw = loadLeaderboard();
    // 按胜率 → 胜场 → 总场排序
    sort(raw.begin(), raw.end(),
         [](const PlayerRecord& a, const PlayerRecord& b) {
            double rateA = (a.total > 0) ? (double)a.wins / a.total : 0;
            double rateB = (b.total > 0) ? (double)b.wins / b.total : 0;
            if (rateA != rateB) return rateA > rateB;
            if (a.wins != b.wins) return a.wins > b.wins;
            return a.total > b.total;
         });
    vector<LeaderboardEntry> result;
    for (const auto& r : raw)
        result.push_back({r.name, r.wins, r.total});
    return result;
}
