# Spectral-EDEN V4.2：无特征分解的局部 PageRank 区域探索

> **方案定位**：性能最激进、代码最精简的方案。彻底删除在线 Eigendecomposition，用局部 Personalized PageRank / APPR 与 conductance sweep 替代 Fiedler 全局二分。
>
> **代码基线**：`island-ran/Spectral-EDEN`，提交 `e97d65be6d347d20d7d2387d0328c9454bb85855`。

---

## 1. 核心思想

V3 试图每次回答：

> 整个已知 DTG 应当怎样全局分成两部分？

对于在线探索，更直接的问题是：

> 从 UAV 当前节点出发，当前局部紧密连通区域到哪里结束，哪些 frontier 位于该局部区域之外？

V4.2 使用：

```text
UAV 最近 H-node 作为 seed
    ↓
在真实稀疏 DTG 上执行有界 APPR push
    ↓
只访问非零 residual 附近节点
    ↓
按 p(v)/degree(v) 做 conductance sweep
    ↓
得到当前局部区域 C
    ↓
区域内 frontier + 边界相邻入口形成候选
```

该方案没有 `λ2`、`λ3`、eigengap，也没有任何 `O(n^3)` 特征分解。

---

## 2. 文献依据与数学策略

### 2.1 局部图聚类

近年的局部图聚类研究强调：算法运行量可以依赖输出簇大小，而不是完整输入图大小。这非常适合 City：完整 DTG 持续增长，但每次真正需要决策的只是 UAV 周围的一个区域。

### 2.2 Personalized PageRank

PPR 满足：

\[
p=\alpha s+(1-\alpha)pW
\]

- `s`：以 UAV seed 为中心的个性化分布；
- `W`：DTG 上的 lazy random walk；
- `α`：回到 seed 的概率。

不求精确解，使用 push 维护近似 `p` 和 residual `r`：

```text
while r(v)/degree(v) > epsilon:
    push(v)
```

### 2.3 Conductance sweep

按：

\[
\frac{p(v)}{d(v)}
\]

降序排列节点，枚举前缀集合并选择较小 conductance：

\[
\Phi(C)=
\frac{w(C,V\setminus C)}
{\min(\operatorname{vol}(C),\operatorname{vol}(V\setminus C))}
\]

它仍然优化低割边/高内部连通结构，与 Ncut 谱思想一致，但没有全局特征向量。

### 2.4 固定多 seed

最多使用三个确定性 seed：

1. UAV 最近 H-node；
2. 当前最高 raw gain frontier 的 anchor；
3. 与当前 cluster 边界拓扑距离最大的一个 anchor。

平局按 H-node ID，禁止随机 seed。

---

## 3. EOHDT 与 V3 模块剪枝

### 3.1 彻底删除在线路径

- `spectral_router.cpp` 的特征值求解；
- 稠密 normalized Laplacian；
- `lambda2/lambda3/eigengap`；
- `BuildActiveDistanceMatrix()` 全对距离；
- `RunEohdtFallback()` 完整 Prim/MST tour；
- kNN + 第二套 MST 谱支撑图；
- corridor compression 专用谱流水线；
- `ComputeEdgeBetweenness()`；
- 五项 partition confidence；
- `BuildRegionAwareRoute()` 完整候选路线；
- baseline/candidate 双 tour 比较；
- soft-lock、lock debt、deferred/cooldown 模式机。

### 3.2 保留

- 真实 DTG 邻接表；
- frontier 和 viewpoint 数据；
- 到单个目标的 Dijkstra/A*；
- EDEN 局部视点评分；
- ASEO continuous/safety trajectory；
- 实际信息增益统计。

---

## 4. 新的规划模型

每周期只做一个宏观决策：

```text
KEEP_CURRENT_TARGET
SELECT_FRONTIER_INSIDE_CURRENT_CLUSTER
SELECT_ADJACENT_CLUSTER_ENTRY
```

### 4.1 当前区域内目标

\[
J(f)=T(f)-\beta\log(1+G_f)
\]

### 4.2 离开局部区域

只考虑当前 cluster 割边直接相邻的区域入口，不允许直接跳到任意远端 frontier：

\[
J_{exit}(f)=T(f)-\beta\log(1+G_f)+\gamma\Phi(C)
\]

`Φ(C)` 是结构量，不是锁定。低 conductance 代表边界较强，但任何候选只要真实时间更优，仍然可以被选中。

### 4.3 确定性目标承诺

当前目标保持，直到：

- frontier 失效；
- 路径或轨迹失败；
- 实际信息增益连续低；
- 图拓扑发生与当前路径相关的变化；
- 新目标预计时间至少优于固定裕量。

\[
T_{new}+\Delta_T<T_{current}
\]

不使用随机 cooldown。

---

## 5. 有界 APPR 实现

```cpp
LocalClusterResult ApproximatePprCluster(
    const SparseGraph& graph,
    uint32_t seed,
    double alpha,
    double epsilon,
    std::size_t max_pushes) {

  SparseVector p;
  SparseVector residual;
  residual[seed] = 1.0;

  MaxQueue active;
  active.push(seed);

  std::size_t pushes = 0;
  while (!active.empty() && pushes < max_pushes) {
    const uint32_t v = active.pop();
    const double degree = graph.weightedDegree(v);
    if (degree <= 1.0e-12) continue;
    if (residual[v] / degree <= epsilon) continue;

    const double mass = residual[v];
    p[v] += alpha * mass;
    residual[v] = 0.5 * (1.0 - alpha) * mass;

    const double distributable =
        0.5 * (1.0 - alpha) * mass;

    for (const auto& edge : graph.neighbors(v)) {
      residual[edge.to] +=
          distributable * edge.weight / degree;
      if (residual[edge.to] /
          graph.weightedDegree(edge.to) > epsilon) {
        active.push(edge.to);
      }
    }
    ++pushes;
  }

  return ConductanceSweep(graph, p, seed, pushes == max_pushes);
}
```

硬预算使用**操作数上限**，不使用 wall-time 强杀：

```yaml
max_push_operations: 20000
max_support_nodes: 512
```

达到上限时返回当前最佳集合并标记 `truncated=true`。主线程可继续使用上一合法 cluster，不能等待。

---

## 6. 真正异步 Worker

### 6.1 Worker 持有独立稀疏图

```cpp
struct GraphDelta {
  uint64_t version = 0;
  std::vector<NodeUpdate> nodes;
  std::vector<EdgeUpdate> edges;
  std::vector<FrontierUpdate> frontiers;
  uint32_t robot_seed = 0;
  std::array<uint32_t, 2> optional_seeds{};
};
```

Worker 不读取主线程正在修改的 `h_ptr`、`H_list_` 或 `hf_edges_`。

### 6.2 永久 Worker 与覆盖式邮箱

```cpp
class LocalClusterWorker {
 public:
  void Start();
  void Stop();
  void Submit(std::shared_ptr<const GraphDelta> delta);
  std::shared_ptr<const LocalClusterState> TryConsume();

 private:
  void Run();

  std::thread worker_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::shared_ptr<const GraphDelta> latest_delta_;
  std::shared_ptr<const LocalClusterState> latest_result_;
  bool running_ = false;
};
```

```cpp
void LocalClusterWorker::Run() {
  SparseGraph graph;

  while (true) {
    std::shared_ptr<const GraphDelta> delta;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [&] {
        return !running_ || latest_delta_ != nullptr;
      });
      if (!running_) break;
      delta = std::move(latest_delta_);
    }

    graph.Apply(*delta);
    LocalClusterState result = RunBoundedClusters(graph, *delta);

    {
      std::lock_guard<std::mutex> lock(mutex_);
      latest_result_ =
          std::make_shared<const LocalClusterState>(std::move(result));
    }
  }
}
```

### 6.3 是否需要无锁队列

短锁 mailbox 只交换两个 `shared_ptr`，更容易验证，优先使用。

若必须无锁：

- 使用容量 2 的 SPSC ring buffer；
- 队列元素是预分配对象池索引；
- producer 构造完成后再发布；
- consumer 独占读取；
- 满时覆盖最旧未消费任务；
- 不在队列中移动并发修改的 `std::vector`。

---

## 7. 顶层伪代码

```cpp
PlanCycleResult PlanV42(const RobotState& robot) {
  UpdateMapAndFrontiers();

  ppr_worker_.Submit(BuildGraphDeltaAndSeeds(robot));

  if (auto result = ppr_worker_.TryConsume()) {
    if (result->version + 1 >= dtg_version_ && result->valid) {
      stable_local_clusters_ = std::move(result);
    }
  }

  if (CurrentTargetValidAndCommitted()) {
    return ContinueCurrentTrajectory();
  }

  CandidateSet candidates;
  AddFrontiersInsideCurrentCluster(candidates);
  AddAdjacentClusterBoundaryEntries(candidates);

  if (candidates.empty()) {
    AddRawReachableFrontierFailOpen(candidates);
  }

  Candidate next = SelectOneTarget(candidates);
  Path path = SearchToSingleTarget(next);
  return EdenLocalViewpointAndAseo(path, next);
}
```

---

## 8. 文件级重构

### Release 路径停止编译/调用

```text
spectral_router.cpp
spectral_v2_policy.cpp
mr_dtg_plus_spectral.cpp 中原 Fiedler pipeline
BuildActiveDistanceMatrix
RunEohdtFallback 的完整 tour
BuildRegionAwareRoute
UpdateLockDebt
UpdateSpectralMode
```

旧代码可保留在 `legacy_v3/` 作为对照，不进入 Release target。

### 新增

```text
src/local_ppr_clusterer.cpp
src/local_cluster_worker.cpp
src/single_macro_target_planner.cpp

include/mr_dtg_plus/local_ppr_clusterer.h
include/mr_dtg_plus/local_cluster_worker.h
include/mr_dtg_plus/single_macro_target_planner.h
```

---

## 9. 参数起点

```yaml
SpectralV42:
  alpha: 0.15
  epsilon: 1.0e-4
  max_push_operations: 20000
  max_support_nodes: 512
  max_seeds: 3
  cluster_min_nodes: 4

  local_candidate_top_k: 8
  target_switch_margin_sec: 0.7
  minimum_target_commit_sec: 1.0
  maximum_result_version_lag: 1
```

---

## 10. 实施顺序

1. 建立 worker 独立 DTG 副本；
2. 实现单 seed APPR 和 sweep 单元测试；
3. 加入固定操作数预算；
4. 在 shadow 模式与 V3 Fiedler 分区比较；
5. 删除 V3 eigensolve 在线路径；
6. 删除全对距离和完整 MST tour；
7. 上线单目标 macro planner；
8. 删除 soft-lock/debt/cooldown；
9. City、Maze、Tunnel 各运行 20 次。

---

## 11. 验收标准

| 指标 | 最低门槛 |
|---|---:|
| Eigendecomposition 调用次数 | 0 |
| 主线程 P99 新增延迟 | ≤ 3 ms |
| Worker P99 | ≤ 20 ms |
| 计算量随完整 DTG 节点数 | 不呈立方趋势 |
| UAV 等待 Worker | 0 |
| 路径相对 EDEN 增长 | ≤ 2% |
| 同地图探索时间 Std | ≤ EDEN 的 1.15 倍 |
| cooldown/lock debt | 已删除 |

---

## 12. 风险

- 论文贡献从“Fiedler向量全局切割”转为“局部谱随机游走”；
- seed 选择会影响局部区域；
- 无法同时得到完整全局区域视图；
- 对相隔很远但战略价值高的孤立区域，需通过第三个 seed 或历史任务队列补充。

---

## 13. 推荐结论

V4.2 是实时性、低方差和代码减法最彻底的方案。若首要目标是消灭 P99 与运行随机性，而不是必须保留 Fiedler 向量，这是最佳选择。

---

## 参考文献

1. Wooseok Ha, Kimon Fountoulakis, Michael W. Mahoney. *Statistical Guarantees for Local Graph Clustering*. JMLR, 2021.
2. Alden Green, Sivaraman Balakrishnan, Ryan J. Tibshirani. *Statistical Guarantees for Local Spectral Clustering on Random Neighborhood Graphs*. JMLR, 2021.
3. Peter Macgregor, He Sun. *Local Algorithms for Finding Densely Connected Clusters*. ICML, 2021.
