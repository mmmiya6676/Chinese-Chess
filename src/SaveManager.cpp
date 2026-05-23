/*
 * SaveManager.cpp — 存档文件管理
 *
 * 负责：
 *  - 扫描 saves/ 目录下的存档文件
 *  - 自动递增 play 编号
 *  - 存档选择菜单
 */

#include "../include/SaveManager.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>
#include <tuple>
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

/*
 * 扫描已有存档，找出 {红方} vs {黑方} 的最大 play 编号，+1 返回。
 * 如果没有旧存档，返回 0。
 */
int nextPlayNumber(const string& red, const string& black) {
    string prefix = red + " vs " + black + " play";
    int maxN = -1;
    for (const auto& f : listSaveFiles()) {
        if (f.find(prefix) == 0) {
            int n = extractPlayNumber(f);
            if (n > maxN) maxN = n;
        }
    }
    return maxN + 1;
}

/*
 * 存档选择菜单：列出所有存档让用户选一个。
 * 返回完整路径；用户选0返回空。
 */
string showLoadMenu() {
    system("cls");
    auto files = listSaveFiles();

    if (files.empty()) {
        cout << "===== 存档列表 =====" << endl;
        cout << "（暂无存档文件）" << endl;
        cout << "请按任意键返回..." << endl;
        system("pause > nul");
        return "";
    }

    cout << "===== 存档列表 =====" << endl;
    for (size_t i = 0; i < files.size(); ++i)
        cout << (i + 1) << ". " << files[i] << endl;
    cout << "0. 返回" << endl;
    cout << "请选择: ";

    int choice;
    cin >> choice;
    cin.ignore();
    if (choice > 0 && choice <= (int)files.size())
        return "saves/" + files[choice - 1];
    return "";
}

// ============================================================
//  排行榜
// ============================================================

// 排行榜文件路径
static const string LEADERBOARD_FILE = "saves/leaderboard.txt";

// 读排行榜，返回每行 [名字, 胜场, 总场]
static vector<tuple<string, int, int>> loadLeaderboard() {
    vector<tuple<string, int, int>> result;
    ifstream file(LEADERBOARD_FILE);
    if (!file) return result;
    string name;
    int wins = 0, total = 0;
    while (file >> name >> wins >> total) {
        if (!name.empty())
            result.emplace_back(name, wins, total);
    }
    return result;
}

// 写排行榜
static void saveLeaderboard(const vector<tuple<string, int, int>>& data) {
    ofstream file(LEADERBOARD_FILE);
    for (const auto& [name, wins, total] : data)
        file << name << " " << wins << " " << total << "\n";
}

// 更新或添加某个玩家的记录
static void updatePlayer(vector<tuple<string, int, int>>& data,
                          const string& name, bool isWin) {
    for (auto& [n, wins, total] : data) {
        if (n == name) {
            total += 1;
            if (isWin) wins += 1;
            return;
        }
    }
    // 新玩家
    data.emplace_back(name, isWin ? 1 : 0, 1);
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

void recordGameDraw(const string& red, const string& black) {
    ensureDir("saves");
    auto data = loadLeaderboard();
    // 和棋：双方各记一场，无胜者
    updatePlayer(data, red,   false);
    updatePlayer(data, black, false);
    saveLeaderboard(data);
}

void showLeaderboard() {
    auto data = loadLeaderboard();
    if (data.empty()) {
        cout << "===== 排行榜 =====" << endl;
        cout << "（暂无对局记录）" << endl;
        return;
    }

    // 按 胜率 → 胜场 → 总场 排序（降序）
    sort(data.begin(), data.end(),
         [](const auto& a, const auto& b) {
            double rateA = (get<2>(a) > 0) ? (double)get<1>(a) / get<2>(a) : 0;
            double rateB = (get<2>(b) > 0) ? (double)get<1>(b) / get<2>(b) : 0;
            if (rateA != rateB) return rateA > rateB;
            if (get<1>(a) != get<1>(b)) return get<1>(a) > get<1>(b);
            return get<2>(a) > get<2>(b);
         });

    system("cls");
    cout << "===== 排行榜 =====" << endl;
    cout << left  << setw(6)  << "排名"
         << left  << setw(16) << "玩家"
         << right << setw(6)  << "胜场"
         << right << setw(6)  << "总场"
         << right << setw(8)  << "胜率" << endl;
    cout << "----------------------------------------" << endl;
    int rank = 1;
    for (const auto& [name, wins, total] : data) {
        double rate = (total > 0) ? 100.0 * wins / total : 0;
        cout << left  << setw(6)  << to_string(rank) + "."
             << left  << setw(16) << name
             << right << setw(6)  << wins
             << right << setw(6)  << total
             << right << setw(7)  << (int)(rate + 0.5) << "%" << endl;
        rank++;
    }
    cout << "========================================" << endl;
}
