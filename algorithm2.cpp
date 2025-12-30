#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <algorithm> 
#include <queue>     

// --- 辅助函数 ---

/**
 * @brief 打印一个集合 (set) 的内容，用于中间过程
 */
void print_set(const std::set<int>& s) {
    std::cout << "{";
    bool first = true;
    for (int id : s) {
        if (!first) {
            std::cout << ", ";
        }
        std::cout << "P" << id;
        first = false;
    }
    std::cout << "}";
}

/**
 * @brief 检查 C_t 是否是 L_i^l 的子集
 */
bool is_ct_subset_li(const std::set<int>& Ct, const std::set<int>& Li) {
    // 检查 Ct 中的每个元素是否都存在于 Li 中
    return std::all_of(Ct.begin(), Ct.end(), [&](int Pk_id) {
        return Li.count(Pk_id) > 0;
    });
}

/**
 * @brief 计算 L_i^l (P_i 的 l-层 邻居)
 * * 根据伪代码和PPT的扫描逻辑 (从右到左), L_i^l 似乎是指
 * 所有在 P_i "右侧" (即 ID > i) 且在 l 跳 (hops) 内可达的事务。
 *
 * @param Pi_id   当前扫描的事务 ID
 * @param l_val   l-层 (l=1 表示直接邻居, l=2 表示邻居的邻居, 等)
 * @param n       总事务数 (未使用, 但可用于边界)
 * @param adj     完整的邻接表 (图 G_vis)
 * @return std::set<int> L_i^l 集合
 */
std::set<int> calculate_Li_l(int Pi_id, int l_val, int n, const std::map<int, std::set<int>>& adj) {
    std::set<int> Li_l_set;
    if (l_val <= 0) {
        return Li_l_set;
    }

    std::queue<std::pair<int, int>> q; // {node_id, distance}
    q.push({Pi_id, 0});
    
    std::map<int, int> distance; // 存储从 Pi_id 出发的距离
    distance[Pi_id] = 0;

    while (!q.empty()) {
        std::pair<int, int> current = q.front();
        q.pop();
        int current_id = current.first;
        int current_dist = current.second;

        // 如果距离超过 l_val，停止这条路径的搜索
        if (current_dist >= l_val) {
            continue;
        }

        // 遍历所有邻居
        if (adj.count(current_id)) {
            for (int neighbor_id : adj.at(current_id)) {
                // 如果这个邻居还没有被访问过
                if (distance.find(neighbor_id) == distance.end()) {
                    int new_dist = current_dist + 1;
                    distance[neighbor_id] = new_dist;
                    
                    // 关键约束：只添加 ID > Pi_id 的事务
                    if (neighbor_id > Pi_id) {
                        Li_l_set.insert(neighbor_id);
                    }
                    
                    // 继续搜索 (即使 neighbor_id <= Pi_id，它仍然可以作为桥梁)
                    q.push({neighbor_id, new_dist});
                }
            }
        }
    }

    return Li_l_set;
}


// --- 算法输入数据 ---

/**
 * @brief 步骤 0: 设置算法1的输出 (来自 image_67435b.png 和 algorithm1.c)
 * @return 包含23条边的邻接表
 */
std::map<int, std::set<int>> setupAdjacencyList_Edges() {
    std::map<int, std::set<int>> adj;
    
    // 初始化所有10个点
    for (int i = 1; i <= 10; ++i) {
        adj[i] = {};
    }

    // 23条边的列表 
    std::vector<std::pair<int, int>> edges = {
        {8, 9}, {9, 10}, {8, 10},
        {7, 8}, {7, 10},
        {6, 7}, {6, 8}, {6, 9},
        {5, 6}, {5, 7}, {5, 10},
        {3, 4}, {3, 5},
        {4, 5}, {4, 6}, {4, 9},
        {2, 3}, {2, 4}, {2, 5}, {2, 10},
        {1, 2}, {1, 4}, {1, 9}
    };

    // 构建邻接表
    for (const auto& edge : edges) {
        adj[edge.first].insert(edge.second);
        adj[edge.second].insert(edge.first);
    }
    
    return adj;
}

// --- 主函数 ---

int main() {
    // --- 步骤 0: 初始化参数 (l, k 可修改) ---
    // =============================================
    // =          在这里修改 l 和 k 的值          =
    // =============================================
    int k = 3;  // 目标簇大小 (例如: 3)
    int l = 1;  // 邻接层数 (例如: 1)
    // =============================================

    const int n = 10; // 总事务数

    std::cout << "--- 算法2: 交易打包选择算法  ---" << std::endl;
    std::cout << "参数: K = " << k << ", L = " << l << std::endl;
    std::cout << "输入: 算法1提供的 23 条可见边 " << std::endl;

    // 获取算法1的输出 (G_vis)
    std::map<int, std::set<int>> adj = setupAdjacencyList_Edges();

    // H 是一个数组，索引代表簇的大小 j
    // H[j] 是一个 vector，包含所有大小为 j 的簇 (set)
    std::vector<std::set<int>> H[k + 1];

    // --- 算法2 伪代码 第1行: 初始化 H_1 ---
    std::cout << "\n--- 1. 初始化 H_1 (j=1) ---" << std::endl;
    for (int i = 1; i <= n; ++i) {
        H[1].push_back({i});
    }
    std::cout << "H_1 (共 " << H[1].size() << " 个簇): {";
    for (size_t i = 0; i < H[1].size(); ++i) {
        print_set(H[1][i]);
        if (i < H[1].size() - 1) std::cout << ", ";
    }
    std::cout << "}" << std::endl;

    // --- 算法2 伪代码 第3-11行: 迭代构建 H_j ---
    for (int j = 2; j <= k; ++j) {
        std::cout << "\n--- 2. 开始构建 H_" << j << " (j=" << j << ") ---" << std::endl;
        
        // 第4行: for i = n-j+1; i >= 1; i--
        for (int i = n - j + 1; i >= 1; --i) {
            int Pi_id = i;
            
            // 计算 L_i^l (P_i 的 l-层 "右侧" 邻居)
            const std::set<int> Li = calculate_Li_l(Pi_id, l, n, adj);
            
            std::cout << "\n   扫描 P_i = P" << Pi_id << ":" << std::endl;
            std::cout << "      L_" << Pi_id << "^" << l << " (l=" << l << " 跳可达且 ID > " << Pi_id << "): ";
            print_set(Li);
            std::cout << std::endl;

            // 第5行: for each C_t in H_{j-1}
            for (const auto& Ct : H[j - 1]) {
                
                // 优化：只检查那些在 P_i "右侧" (ID更大) 的簇
                bool all_gt_i = true;
                for (int id : Ct) {
                    if (id <= Pi_id) {
                        all_gt_i = false;
                        break;
                    }
                }
                
                if (!all_gt_i) {
                    continue; // 跳过 C_t = {P_m, ...} m <= i 的情况
                }

                std::cout << "      - 检查 H_" << (j - 1) << " 中的 C_t = ";
                print_set(Ct);
                std::cout << std::endl;

                // 第6行: if C_t subset L_i^l
                if (is_ct_subset_li(Ct, Li)) {
                    // 第7行: H_j <- H_j U {P_i, C_t}
                    std::set<int> newCluster = Ct;
                    newCluster.insert(Pi_id);
                    H[j].push_back(newCluster);
                    
                    std::cout << "         -> OK! C_t 是 L_" << Pi_id << "^" << l << " 的子集。" << std::endl;
                    std::cout << "         -> 创建新 H_" << j << " 簇: ";
                    print_set(newCluster);
                    std::cout << std::endl;
                } else {
                    std::cout << "         -> 失败! C_t 不是 L_" << Pi_id << "^" << l << " 的子集。" << std::endl;
                }
            }
        }
        
        std::cout << "\n--- H_" << j << " 构建完成 (共 " << H[j].size() << " 个簇) ---" << std::endl;
        // 打印所有 H_j 的内容
        for (size_t i = 0; i < H[j].size(); ++i) {
            print_set(H[j][i]);
            if ((i + 1) % 5 == 0) std::cout << std::endl; // 每5个换行
            else if (i < H[j].size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << "\n--- 算法2 执行完成 (j=" << k << ") ---" << std::endl;

    // --- 算法2 伪代码 第12-17行: 查找最佳 H_k ---
    // 伪代码的这一部分需要 R(C_t) (评分函数)，
    // 该函数需要 w_j (等待时间), e_j (事务优先级), a_j (设备优先级)
    // 这些数据在 algorithm1.c 中未定义。
    // 因此，我们仅打印出 H_k (即 H_3) 的所有候选簇，这与 PPT 第6页的演示一致。
    
    std::cout << "\n--- 最终 H_" << k << " 候选簇列表 (共 " << H[k].size() << " 个) ---" << std::endl;
    for (size_t i = 0; i < H[k].size(); ++i) {
        std::cout << "   候选 " << (i + 1) << ": ";
        print_set(H[k][i]);
        std::cout << std::endl;
    }

    return 0;
}