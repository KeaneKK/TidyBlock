#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <iomanip>

using namespace std;

// 定义矩形结构体
struct Block {
    double x1, y1, x2, y2;
    double weight;
};

// 定义扫描线事件
struct Event {
    double x;           // 当前扫描线的 X 坐标
    int type;           // 1 表示入边 (矩形开始), -1 表示出边 (矩形结束)
    int y_start_idx;    // 对应的 Y 轴离散化起始索引
    int y_end_idx;      // 对应的 Y 轴离散化结束索引
    double weight;      // 权重

    // 按 X 坐标从小到大排序
    bool operator<(const Event& other) const {
        if (x != other.x) return x < other.x;
        return type > other.type; // 如果 X 相同，先处理入边 (+1)，再处理出边 (-1)
    }
};

// 线段树节点
struct Node {
    double max_val;     // 当前区间的最大权重
    int max_idx;        // 最大权重对应的叶子节点索引 (代表具体的 Y 区间)
    double lazy;        // 懒标记
} tree[80005];          // 假设最多 20000 个矩形 -> 40000 个 Y 坐标 -> 树大小需 4 倍

int n; // 离散化后的区间数量

// 上推：父节点获取子节点的最大值信息
void push_up(int node) {
    if (tree[node * 2].max_val >= tree[node * 2 + 1].max_val) {
        tree[node].max_val = tree[node * 2].max_val;
        tree[node].max_idx = tree[node * 2].max_idx;
    } else {
        tree[node].max_val = tree[node * 2 + 1].max_val;
        tree[node].max_idx = tree[node * 2 + 1].max_idx;
    }
}

// 下推：懒标记下传
void push_down(int node) {
    if (tree[node].lazy != 0) {
        double lz = tree[node].lazy;
        
        tree[node * 2].max_val += lz;
        tree[node * 2].lazy += lz;
        
        tree[node * 2 + 1].max_val += lz;
        tree[node * 2 + 1].lazy += lz;
        
        tree[node].lazy = 0;
    }
}

// 建树
void build(int node, int start, int end) {
    tree[node].lazy = 0;
    if (start == end) {
        tree[node].max_val = 0;
        tree[node].max_idx = start; // 叶子节点记录自己的索引
        return;
    }
    int mid = (start + end) / 2;
    build(node * 2, start, mid);
    build(node * 2 + 1, mid + 1, end);
    push_up(node);
}

// 区间更新
void update(int node, int start, int end, int l, int r, double val) {
    if (l <= start && end <= r) {
        tree[node].max_val += val;
        tree[node].lazy += val;
        return;
    }
    push_down(node);
    int mid = (start + end) / 2;
    if (l <= mid) update(node * 2, start, mid, l, r, val);
    if (r > mid) update(node * 2 + 1, mid + 1, end, l, r, val);
    push_up(node);
}

// 核心算法函数
pair<double, double> solveBlockSelection(vector<Block>& blocks) {
    if (blocks.empty()) return {0.0, 0.0};

    vector<double> Y;
    // 1. 收集所有 Y 坐标用于离散化
    for (const auto& b : blocks) {
        Y.push_back(b.y1);
        Y.push_back(b.y2);
    }
    sort(Y.begin(), Y.end());
    Y.erase(unique(Y.begin(), Y.end()), Y.end());

    int y_cnt = Y.size();
    // 离散化后的有效区间是 y_cnt - 1 个
    // 每一个索引 i 代表区间 [Y[i], Y[i+1])
    n = y_cnt - 1; 

    // 2. 构建事件
    vector<Event> events;
    for (const auto& b : blocks) {
        // 使用 lower_bound 查找离散化后的索引
        int y_l = lower_bound(Y.begin(), Y.end(), b.y1) - Y.begin();
        int y_r = lower_bound(Y.begin(), Y.end(), b.y2) - Y.begin();
        
        // 注意：线段树维护的是区间索引。
        // 如果矩形覆盖 y1 到 y2，对应的离散区间索引是从 y_l 到 y_r - 1
        if (y_l < y_r) { 
            events.push_back({b.x1, 1, y_l, y_r - 1, b.weight});
            events.push_back({b.x2, -1, y_l, y_r - 1, b.weight});
        }
    }
    sort(events.begin(), events.end());

    // 3. 初始化线段树
    build(1, 0, n - 1);

    double max_weight = -1.0;
    double best_x1 = 0, best_x2 = 0;
    double best_y1 = 0, best_y2 = 0;

    // 4. 扫描过程
    for (int i = 0; i < events.size(); ++i) {
        // 更新线段树
        update(1, 0, n - 1, events[i].y_start_idx, events[i].y_end_idx, events[i].type * events[i].weight);

        // 如果下一个事件的 X 坐标不同，说明当前 X 位置的所有重叠情况已处理完毕
        // 或者是最后一个事件
        if (i == events.size() - 1 || events[i+1].x > events[i].x) {
            double current_max = tree[1].max_val;
            
            if (current_max > max_weight) {
                max_weight = current_max;
                
                // 记录 X 范围：当前事件 X 到 下一个事件 X
                best_x1 = events[i].x;
                // 防止越界读取下一个
                best_x2 = (i == events.size() - 1) ? best_x1 : events[i+1].x;
                
                // 获取 Y 范围：根据线段树最大值所在的索引 idx
                int best_idx = tree[1].max_idx;
                best_y1 = Y[best_idx];
                best_y2 = Y[best_idx + 1];
            }
        }
    }

    // 计算中心点 (原算法逻辑)
    double center_x = (best_x1 + best_x2) / 2.0;
    double center_y = (best_y1 + best_y2) / 2.0;

    cout << "最大权重: " << max_weight << endl;
    cout << "最佳区域 X: [" << best_x1 << ", " << best_x2 << "]" << endl;
    cout << "最佳区域 Y: [" << best_y1 << ", " << best_y2 << "]" << endl;

    return {center_x, center_y};
}

int main() {
    // 示例数据：生成一些矩形 (x1, y1, x2, y2, weight)
    // 这里的矩形可以理解为：以待验证 Block 为中心生成的区域
    vector<Block> blocks = {
        {10, 10, 20, 20, 5.0},
        {15, 15, 25, 25, 10.0}, // 这个与第一个有重叠
        {18, 12, 22, 18, 3.0},  // 这个也有重叠
        {40, 40, 50, 50, 8.0}   // 独立的
    };

    cout << "开始计算最佳区块位置..." << endl;
    pair<double, double> center = solveBlockSelection(blocks);

    cout << "推荐中心点坐标: (" << center.first << ", " << center.second << ")" << endl;

    return 0;
}