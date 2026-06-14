#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <string>
#include <vector>
#include <map>

// 扫描 saves/ 目录下所有 .txt 文件，返回排序后的文件名列表
std::vector<std::string> listSaveFiles();

// 确保目录存在，不存在则创建
void ensureDir(const std::string& path);

// 玩家名 → 数字ID（解决中文文件名编码问题）
int  getOrCreatePlayerID(const std::string& name);

// 用数字 ID 生成存档文件名
std::string buildSaveFilename(int redID, int blackID, int playN);

// 计算同对玩家的下一个 play 编号（用ID）
int nextPlayNumber(int redID, int blackID);

// 记录对局结果：(红方名, 黑方名, 胜者名)
void recordGameResult(const std::string& red, const std::string& black,
                      const std::string& winner);

// Qt 可用的排行榜数据结构
struct LeaderboardEntry {
    std::string name;
    int wins = 0;
    int total = 0;
};
std::vector<LeaderboardEntry> loadLeaderboardData();

// 读玩家注册表（ID → 名字），loadgamedialog 用来翻译存档文件名的数字 ID
std::map<int, std::string> loadPlayerRegistry();

#endif // SAVEMANAGER_H
