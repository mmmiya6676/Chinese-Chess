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
#include <cstdlib>
#include <algorithm>
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
