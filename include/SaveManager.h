#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <string>
#include <vector>

// 扫描 saves/ 目录下所有 .txt 文件，返回排序后的文件名列表
std::vector<std::string> listSaveFiles();

// 确保目录存在，不存在则创建
void ensureDir(const std::string& path);

// 从文件名提取 play 编号（"张三 vs 李四 play3.txt" → 3）
int extractPlayNumber(const std::string& filename);

// 计算同对玩家的下一个 play 编号
int nextPlayNumber(const std::string& red, const std::string& black);

// 弹出存档选择菜单，返回选中的完整路径，用户取消返回空
std::string showLoadMenu();

// 记录对局结果：winner 胜，loser 负
void recordGameResult(const std::string& red, const std::string& black,
                      const std::string& winner);

// 记录和棋：双方各记一场，无胜者
void recordGameDraw(const std::string& red, const std::string& black);

// 显示排行榜
void showLeaderboard();

#endif // SAVEMANAGER_H
