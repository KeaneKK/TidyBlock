    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <math.h>

    typedef struct {
        int x, y;
        int id;
    } Point;

    typedef struct {
        Point p1, p2;
    } Edge;

    // 比较函数：按横坐标递增排序
    int compare_points(const void *a, const void *b) {
        Point *p1 = (Point *)a;
        Point *p2 = (Point *)b;
        if (p1->x != p2->x) return p1->x - p2->x;
        return p1->y - p2->y;
    }

    // 计算叉积
    int cross_product(Point p1, Point p2, Point p3) {
        return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
    }
    /*例如P3: (6, 3)
        P2: (6, 5)
        P1: (7, 1)
        叉积 = (6 - 7) * (3 - 1) - (5 - 1) * (6 - 7)
            = (-1) * 2 - 4 * (-1)
            = -2 + 4
            = 2 > 0 （左转）
        */
        
    // 计算两点距离的平方
    double distance_sq(Point a, Point b) {
        return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
    }

    // 判断点是否在线段上（包括端点）
    bool point_on_segment_include_endpoints(Point p, Point a, Point b) {
        if (cross_product(a, b, p) != 0) return false;
        return (p.x >= fmin(a.x, b.x) && p.x <= fmax(a.x, b.x) &&
                p.y >= fmin(a.y, b.y) && p.y <= fmax(a.y, b.y));
    }

    // 判断边是否在边集合中
    bool is_edge_in_set(Edge edge, Edge *edge_set, int count) {
        for (int i = 0; i < count; i++) {
            if ((edge.p1.id == edge_set[i].p1.id && edge.p2.id == edge_set[i].p2.id) ||
                (edge.p1.id == edge_set[i].p2.id && edge.p2.id == edge_set[i].p1.id)) {
                return true;
            }
        }
        return false;
    }

    // 添加边到集合
    void add_edge_to_set(Edge edge, Edge **edge_set, int *count, int *capacity) {
        if (is_edge_in_set(edge, *edge_set, *count)) return;
        if (*count >= *capacity) {
            *capacity = (*capacity == 0) ? 20 : *capacity * 2;
            *edge_set = realloc(*edge_set, *capacity * sizeof(Edge));
        }
        (*edge_set)[(*count)++] = edge;
    }

    // 凸包计算 - 保留所有共线点
    void compute_convex_hull(Point *points, int n, Point **hull_points, int *hull_count) {
        printf("=== 开始凸包计算 ===\n");
        printf("输入点 (%d个): ", n);
        for (int i = 0; i < n; i++) {
            printf("P%d(%d,%d) ", points[i].id, points[i].x, points[i].y);
        }
        printf("\n");
        
        if (n <= 3) {
            *hull_count = n;
            *hull_points = malloc(n * sizeof(Point));
            for (int i = 0; i < n; i++) (*hull_points)[i] = points[i];
            printf("点数≤3，直接返回所有点\n");
            printf("凸包点: ");
            for (int i = 0; i < n; i++) printf("P%d ", (*hull_points)[i].id);
            printf("\n=== 结束凸包计算 ===\n\n");
            return;
        }
        
        // 复制点集，避免修改原数据
        Point *copy = malloc(n * sizeof(Point));
        for (int i = 0; i < n; i++) copy[i] = points[i];
        
        // 找到最左下角的点
        int start = 0;
        for (int i = 1; i < n; i++) {
            if (copy[i].y < copy[start].y || 
                (copy[i].y == copy[start].y && copy[i].x < copy[start].x)) {
                start = i;
            }
        }
        printf("最左下角点: P%d(%d,%d)\n", copy[start].id, copy[start].x, copy[start].y);
        
        // 将起始点交换到第一个位置
        Point temp = copy[0];
        copy[0] = copy[start];
        copy[start] = temp;
        
        // 极角排序
        printf("极角排序过程:\n");
        for (int i = 1; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                int orient = cross_product(copy[0], copy[i], copy[j]);
                printf("  P%d-P%d-P%d 叉积=%d ", copy[0].id, copy[i].id, copy[j].id, orient);
                
                if (orient < 0) {
                    printf("→ 逆时针，交换P%d和P%d\n", copy[i].id, copy[j].id);
                    Point temp = copy[i];
                    copy[i] = copy[j];
                    copy[j] = temp;
                } else if (orient == 0) {
                    double dist_i = distance_sq(copy[0], copy[i]);
                    double dist_j = distance_sq(copy[0], copy[j]);
                    printf("→ 共线，距离P%d=%.1f P%d=%.1f ", copy[i].id, dist_i, copy[j].id, dist_j);
                    if (dist_i < dist_j) {
                        printf("→ 交换(远的在前)\n");
                        Point temp = copy[i];
                        copy[i] = copy[j];
                        copy[j] = temp;
                    } else {
                        printf("→ 不交换\n");
                    }
                } else {
                    printf("→ 顺时针，不交换\n");
                }
            }
        }
        
        printf("排序后点顺序: ");
        for (int i = 0; i < n; i++) {
            printf("P%d ", copy[i].id);
        }
        printf("\n");
        
        // Graham Scan - 保留所有共线点
        Point *stack = malloc(n * sizeof(Point));
        int stack_size = 0;
        
        stack[stack_size++] = copy[0];
        stack[stack_size++] = copy[1];
        
        printf("Graham Scan过程:\n");
        printf("  初始栈: ");
        for (int i = 0; i < stack_size; i++) printf("P%d ", stack[i].id);
        printf("\n");
        
        for (int i = 2; i < n; i++) {
            printf("  处理点 P%d: ", copy[i].id);
            
            while (stack_size >= 2) {
                Point p1 = stack[stack_size - 2];
                Point p2 = stack[stack_size - 1];
                Point p3 = copy[i];
                
                int orient = cross_product(p1, p2, p3);
                printf("转向(P%d-P%d-P%d)=%d ", p1.id, p2.id, p3.id, orient);
                
                if (orient < 0) {
                    printf("→ 右转，弹出P%d ", stack[stack_size-1].id);
                    stack_size--;
                } else {
                    printf("→ 左转/共线，保留 ");
                    break;
                }
            }
            
            stack[stack_size++] = copy[i];
            printf("→ 压入P%d\n", copy[i].id);
            printf("    当前栈: ");
            for (int j = 0; j < stack_size; j++) printf("P%d ", stack[j].id);
            printf("\n");
        }
        
        *hull_count = stack_size;
        *hull_points = malloc(stack_size * sizeof(Point));
        for (int i = 0; i < stack_size; i++) {
            (*hull_points)[i] = stack[i];
        }
        
        printf("最终凸包点 (%d个): ", stack_size);
        for (int i = 0; i < stack_size; i++) printf("P%d ", (*hull_points)[i].id);
        printf("\n=== 结束凸包计算 ===\n\n");
        
        free(stack);
        free(copy);
    }

    // 构建凸包边集
    void build_hull_edges(Point *hull_points, int hull_count, Edge **hull_edges, int *edge_count) {
        *edge_count = hull_count;
        *hull_edges = malloc(hull_count * sizeof(Edge));
        for (int i = 0; i < hull_count; i++) {
            (*hull_edges)[i] = (Edge){hull_points[i], hull_points[(i + 1) % hull_count]};
        }
    }

    // 可见性检查
    bool is_visible(Point Pi, Point Pk, Point *hull_points, int hull_count, Edge *hull_edges, int edge_count, Edge *existing_edges, int existing_count) {
        // 1. 检查是否与凸包边在非端点处相交
        for (int j = 0; j < edge_count; j++) {
            Edge e = hull_edges[j];
            if (Pk.id == e.p1.id || Pk.id == e.p2.id) continue;
            
            Point a1 = Pi, a2 = Pk;
            Point b1 = e.p1, b2 = e.p2;
            
            int d1 = cross_product(b1, b2, a1);
            int d2 = cross_product(b1, b2, a2);
            int d3 = cross_product(a1, a2, b1);
            int d4 = cross_product(a1, a2, b2);
            
            if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
                ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) {
                return false;
            }
        }
        
        // 2. 检查是否与已存在边在非端点处相交
        for (int j = 0; j < existing_count; j++) {
            Edge e = existing_edges[j];
            if (Pi.id == e.p1.id || Pi.id == e.p2.id || 
                Pk.id == e.p1.id || Pk.id == e.p2.id) continue;
            
            Point a1 = Pi, a2 = Pk;
            Point b1 = e.p1, b2 = e.p2;
            
            int d1 = cross_product(b1, b2, a1);
            int d2 = cross_product(b1, b2, a2);
            int d3 = cross_product(a1, a2, b1);
            int d4 = cross_product(a1, a2, b2);
            
            if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
                ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) {
                return false;
            }
        }
        
        // 3. 检查是否有其他凸包点在Pi-Pk线段上（阻挡视线）
        for (int m = 0; m < hull_count; m++) {
            Point Pm = hull_points[m];
            if (Pm.id == Pi.id || Pm.id == Pk.id) continue;
            
            if (point_on_segment_include_endpoints(Pm, Pi, Pk)) {
                double dist_PiPm = distance_sq(Pi, Pm);
                double dist_PiPk = distance_sq(Pi, Pk);
                
                if (dist_PiPm < dist_PiPk) {
                    return false;
                }
            }
        }
        
        return true;
    }

    // 主算法
    Edge* build_visible_network(Point *points, int n, int *total_edge_count) {
        qsort(points, n, sizeof(Point), compare_points);
        for (int i = 0; i < n; i++) points[i].id = i + 1;
        
        int E_capacity = 50;
        Edge *E = malloc(E_capacity * sizeof(Edge));
        int E_count = 0;
        
        Point *V = malloc(n * sizeof(Point));
        int V_count = 0;
        
        Point *CP = NULL;
        int CP_count = 0;
        Edge *CE = NULL;
        int CE_count = 0;
        
        // 初始化最后三个点
        if (n >= 3) {
            V[V_count++] = points[n-1];
            V[V_count++] = points[n-2];
            V[V_count++] = points[n-3];
            
            printf("初始化最后三个点: P%d, P%d, P%d\n", 
                points[n-1].id, points[n-2].id, points[n-3].id);
            
            compute_convex_hull(V, V_count, &CP, &CP_count);
            build_hull_edges(CP, CP_count, &CE, &CE_count);
            
            for (int i = 0; i < CE_count; i++) {
                add_edge_to_set(CE[i], &E, &E_count, &E_capacity);
            }
        }
        
        // 从右向左扫描
        for (int i = n-4; i >= 0; i--) {
            Point Pi = points[i];
            
            printf(">>> 处理点 P%d\n", Pi.id);
            printf("当前V中的点 (%d个): ", V_count);
            for (int j = 0; j < V_count; j++) printf("P%d ", V[j].id);
            printf("\n");
            
            printf("可见点检查: ");
            for (int k = 0; k < CP_count; k++) {
                Point Pk = CP[k];
                if (is_visible(Pi, Pk, CP, CP_count, CE, CE_count, E, E_count)) {
                    Edge new_edge = {Pi, Pk};
                    add_edge_to_set(new_edge, &E, &E_count, &E_capacity);
                    printf("P%d ", Pk.id);
                }
            }
            printf("\n");
            
            V[V_count++] = Pi;
            
            if (CP != NULL) free(CP);
            if (CE != NULL) free(CE);
            compute_convex_hull(V, V_count, &CP, &CP_count);
            build_hull_edges(CP, CP_count, &CE, &CE_count);
        }
        
        *total_edge_count = E_count;
        free(V);
        if (CP != NULL) free(CP);
        if (CE != NULL) free(CE);
        return E;
    }

    // 检查是否包含特定边
    bool contains_edge(Edge *edges, int count, int id1, int id2) {
        for (int i = 0; i < count; i++) {
            if ((edges[i].p1.id == id1 && edges[i].p2.id == id2) ||
                (edges[i].p1.id == id2 && edges[i].p2.id == id1)) {
                return true;
            }
        }
        return false;
    }

    // 测试函数
    int main() {
        Point points[] = {
            {1, 4}, {2, 2}, {3, 3}, {3, 4}, {4, 2}, 
            {5, 4}, {6, 2}, {6, 3}, {6, 5}, {7, 1}
        };
        int n = sizeof(points) / sizeof(points[0]);
        
        printf("输入点集 (%d 个点):\n", n);
        for (int i = 0; i < n; i++) {
            printf("P%d: (%d, %d)\n", i + 1, points[i].x, points[i].y);
        }
        printf("\n");
        
        int total_edges;
        Edge *network_edges = build_visible_network(points, n, &total_edges);
        
        printf("\n最终可见网络的所有边 (%d 条):\n", total_edges);
        for (int i = 0; i < total_edges; i++) {
            printf("边 %2d: P%d-P%d\n", i + 1, network_edges[i].p1.id, network_edges[i].p2.id);
        }
        
        // 检查预期的23条边
        printf("\n检查预期边:\n");
        int expected_edges[23][2] = {
            {8,9}, {9,10}, {8,10}, {7,8}, {7,10}, {6,7}, {6,8}, {6,9},
            {5,6}, {5,7}, {5,10}, {4,5}, {4,6}, {4,9}, {3,4}, {3,5},
            {2,3}, {2,4}, {2,5}, {2,10}, {1,2}, {1,4}, {1,9}
        };
        
        int missing_count = 0;
        int extra_count = 0;
        
        for (int i = 0; i < 23; i++) {
            int id1 = expected_edges[i][0];
            int id2 = expected_edges[i][1];
            if (!contains_edge(network_edges, total_edges, id1, id2)) {
                printf("缺少边: P%d-P%d\n", id1, id2);
                missing_count++;
            }
        }
        
        // 检查多余的边
        printf("\n检查多余边:\n");
        for (int i = 0; i < total_edges; i++) {
            int id1 = network_edges[i].p1.id;
            int id2 = network_edges[i].p2.id;
            bool found = false;
            for (int j = 0; j < 23; j++) {
                if ((id1 == expected_edges[j][0] && id2 == expected_edges[j][1]) ||
                    (id1 == expected_edges[j][1] && id2 == expected_edges[j][0])) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("多余边: P%d-P%d\n", id1, id2);
                extra_count++;
            }
        }
        
        if (missing_count == 0 && extra_count == 0) {
            printf("✓ 所有预期边都存在，没有多余边！\n");
        } else {
            printf("✗ 缺少 %d 条边，多出 %d 条边\n", missing_count, extra_count);
        }
        
        free(network_edges);
        return 0;
    }