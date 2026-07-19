# Spectral-EDEN V4.1：增量谱骨架与区域商图规划

> **方案定位**：推荐主方案。保留 Fiedler 向量与 Normalized Cut 作为论文核心，但将求谱对象从持续增长的完整 DTG 改为固定规模、增量维护的拓扑骨架；同时删除 EOHDT 完整访问序列和谱完整候选序列的双重计算。
>
> **代码基线**：`island-ran/Spectral-EDEN`，提交 `e97d65be6d347d20d7d2387d0328c9454bb85855`。

---

## 1. 当前 V3 的真正问题

V3 的计算崩溃不是单一的 `Eigen::SelfAdjointEigenSolver` 导致，而是同一规划周期重复做了多套全局工作：

1. `BuildActiveDistanceMatrix()` 构造活动锚点的全对距离矩阵，并补齐缺失的节点对距离；
2. `RunEohdtFallback()` 对全部活动锚点执行 Prim，并遍历整棵树生成完整路线；
3. `BuildSparseSupportSpectralGraph()` 再对活动锚点建立 kNN，并补一棵活动节点 MST；
4. `CompressDegreeTwoChains()` 每次重新扫描并压缩支撑图；
5. 可选 `ComputeEdgeBetweenness()` 从每个谱节点执行一次最短路统计；
6. `BuildRegionAwareRoute()` 再生成一条“距离 + 区域切换惩罚”的完整贪心路线；
7. 同时计算 baseline/candidate 两条路线的完整代价，并维护 soft mode、lock debt、recovery 和 cooldown。

因此 V3 是：

```text
全对距离 + EOHDT MST + 谱 kNN/MST + 全图压缩 + 特征求解
+ 完整谱路线 + 双路线比较 + 复杂状态机
```

V4.1 的目标不是把其中某一项再优化 20%，而是删除重复层次。

---

## 2. 文献启发与理论降维

### 2.1 动态谱维护：维护增量，而不是反复重建

动态图谱聚类近年的核心思想是维护随边和节点更新而变化的图摘要，并将更新与查询解耦。V4.1 不直接照搬理论算法，而采用其工程原则：

- DTG 变化表示为 `GraphDelta`；
- 谱线程持有独立、持久的图副本；
- 只更新受影响节点、边、degree 和 corridor chain；
- 规划线程读取最近一份合法区域状态，从不等待本次求解。

### 2.2 只求 `u2`、`u3`

在线二分只需要：

- `u2`：主弱连接方向；
- `u3`：二分可靠性与下一谱方向。

不需要完整特征分解。使用块 LOBPCG、块 Lanczos，或保留当前两向量 Rayleigh–Ritz 框架，但必须直接对稀疏拉普拉斯执行矩阵向量乘法。

当前代码在迭代路径中接收 `Eigen::MatrixXd normalized_laplacian`，随后再次扫描所有元素转换成 `SparseMatrix`。V4.1 应从一开始就构造 `Eigen::SparseMatrix<double>`，禁止“稠密构造 → 再稀疏化”。

### 2.3 持久化 corridor contraction

当前链压缩是每次全量执行。V4.1 维护持久化骨架：

```text
真实 DTG
  ├─ 分叉节点：永久保留
  ├─ 活跃 frontier anchor：保留
  ├─ 低净空瓶颈端点：保留
  ├─ UAV 当前投影节点：保留
  └─ 度为 2 的普通走廊节点：折叠为 chain edge
```

局部更新规则：

- 新增普通度 2 节点：扩展一条 chain；
- 新增分支：只拆分所在 chain；
- frontier 状态改变：只更新相关 anchor 标志；
- 边失效：只重建所在连通局部；
- chain 长度为边长之和；
- chain clearance 为沿途最小值。

固定骨架预算：

```yaml
skeleton_max_nodes: 64
```

超过预算时，优先删除无 frontier、非分叉、非瓶颈、距当前活动区域最远的中间节点，而不是进入随机 cooldown。

---

## 3. EOHDT 剪枝：必须删除什么

### 3.1 删除完整活动节点全对距离矩阵

停止在每周期调用完整 `BuildActiveDistanceMatrix()`。

替代为三类按需距离：

1. UAV 到本周期 TOP-K frontier/入口：一次多目标 Dijkstra；
2. 区域商图相邻边：缓存真实瓶颈两端距离；
3. 当前区域内部：只计算 TOP-K 候选的精确路径。

不再维护 `O(n^2)` 的活动 anchor 对距离。

### 3.2 删除每周期 EOHDT 完整 Prim 路线

停止使用 `RunEohdtFallback()` 生成“访问所有 frontier 的完整静态路线”。未知地图每个周期都变化，长路线很快失效。

保留 EOHDT 中真正有价值的底层能力：

- DTG 建图；
- Dijkstra/A*；
- 到单个目标的路径构造；
- EDEN 局部视点评分；
- ASEO 连续轨迹和安全轨迹。

删除其全局 tour 层，不删除其路径搜索层。

### 3.3 删除谱支撑图中的第二套 MST

当前谱图采用 kNN union，再增加活动节点 MST 保连通。V4.1 的谱骨架直接来自真实 DTG 边，不再从全对活动距离构造人工 kNN/MST。

### 3.4 删除在线边介数

停止在线调用 `ComputeEdgeBetweenness()`。它对每个谱节点执行最短路，既昂贵，又与 Fiedler/Ncut 再次重复表达瓶颈。

在线边权只使用局部量：

\[
w_{ij}=
\exp\!\left(-\frac{\ell_{ij}}{\sigma_\ell}\right)
\left(\frac{c_{ij}+\epsilon}{c_{ref}+\epsilon}\right)^p
\]

其中：

- `ℓ_ij`：chain 路径长度；
- `c_ij`：最小净空；
- `σ_ℓ`：骨架边长稳健尺度。

### 3.5 删除完整区域感知贪心路线

停止调用 `BuildRegionAwareRoute()` 对所有 anchor 重新排序。谱层只输出一个宏观动作：

```text
KEEP_CURRENT_TARGET
NEXT_FRONTIER
NEXT_REGION_ENTRY
```

### 3.6 删除 soft-lock、lock debt 和随机 cooldown

删除在线决策中的：

- `ComputeDynamicSwitchPenalty()`；
- `UpdateLockDebt()`；
- `recovery_until_` 时间冷却；
- `OBSERVING → ACTIVE_SOFT → RECOVERY` 复杂状态转换；
- 五项 confidence 对规划模式的联动。

新谱状态只保留：

```cpp
enum class SpectralStateValidity {
  VALID,
  STALE,
  INVALID
};
```

---

## 4. 新的深度融合架构

```text
Layer 1：Sparse DTG
  只表达真实可达性和路径

Layer 2：Persistent Spectral Skeleton
  固定规模、增量维护、识别瓶颈

Layer 3：Region Quotient Graph Q
  顶点=稳定区域，边=真实区域入口

Layer 4：Single-Step Macro Planner
  只决定下一个 frontier 或区域入口

Layer 5：EDEN Local Viewpoint + ASEO
  负责视点、连续性、安全与轨迹
```

### 4.1 区域商图

```cpp
struct RegionNode {
  int id = -1;
  double total_gain = 0.0;
  std::vector<uint32_t> frontier_ids;
  std::vector<uint32_t> entry_h_ids;
  uint64_t last_changed_version = 0;
};

struct RegionEdge {
  int from = -1;
  int to = -1;
  uint32_t entry_from = 0;
  uint32_t entry_to = 0;
  double traversal_cost = 0.0;
};
```

商图通常只有 2–8 个节点，只在这个小图上做 Dijkstra/A*。

### 4.2 单步宏观目标

候选区域代价：

\[
J(r)=T_{entry}(r)-\beta\log(1+G_r)+\gamma R_r
\]

- `T_entry`：精确到入口的预计飞行时间；
- `G_r`：区域中 raw reachable frontier 的总增益；
- `R_r`：真实执行后低收益的历史统计。

`R_r` 不是锁定项，不禁止跨区。

### 4.3 目标承诺替代 soft-lock

已发布目标保持不变，直到满足明确事件：

- 当前 frontier 失效；
- 当前路径失效；
- 当前轨迹不安全；
- 实际信息增益连续两次过低；
- 新目标预计时间显著更优。

切换条件：

\[
T_{new}+\Delta_T<T_{current}
\]

`ΔT` 是固定迟滞裕量，所有运行完全确定；平局按 frontier ID 排序。

建议：

```yaml
target_switch_margin_sec: 0.7
minimum_target_commit_sec: 1.0
```

---

## 5. 真正的 C++ 异步架构

### 5.1 当前异步为什么仍有 P99 尖峰

当前 `SubmitSpectralJobAsync()` 在主线程中先执行：

```cpp
BuildSpectralGraphSnapshot(...)
```

完成支撑路径、kNN/MST、clearance、compression 后，才调用：

```cpp
std::async(std::launch::async, ... router.Solve(...))
```

因此当前不是“完全没有异步”，而是**只有最终求解异步，重预处理仍在主线程**。此外，每任务一次 `std::async` 会带来线程生命周期抖动。

### 5.2 永久 Worker + latest-only mailbox

```cpp
class SpectralWorker {
 public:
  void Start();
  void Stop();
  void Submit(std::shared_ptr<const SkeletonDelta> delta);
  std::shared_ptr<const SpectralState> TryConsume();

 private:
  void Run();

  std::thread worker_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::shared_ptr<const SkeletonDelta> latest_delta_;
  std::shared_ptr<const SpectralState> latest_result_;
  bool running_ = false;
};
```

提交时只交换指针：

```cpp
void SpectralWorker::Submit(
    std::shared_ptr<const SkeletonDelta> delta) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    latest_delta_ = std::move(delta);  // 覆盖旧的未处理增量
  }
  cv_.notify_one();
}
```

Worker 取走对象后立即释放锁，所有图处理在锁外执行。

### 5.3 主线程只构造轻量 delta

```cpp
struct SkeletonDelta {
  uint64_t graph_version = 0;
  std::vector<NodeInsert> node_inserts;
  std::vector<NodeErase> node_erases;
  std::vector<EdgeUpsert> edge_upserts;
  std::vector<EdgeErase> edge_erases;
  std::vector<FrontierAnchorChange> anchor_changes;
};
```

主线程禁止执行：

- corridor compression；
- Laplacian 构造；
- eigensolve；
- Ncut sweep；
- 区域匹配；
- 商图构造。

### 5.4 Worker 结果

```cpp
struct SpectralState {
  uint64_t graph_version = 0;
  double residual = INFINITY;
  double ncut = INFINITY;
  std::unordered_map<uint32_t, int> region_of_h;
  RegionQuotientGraph quotient;
};
```

主线程仅当以下条件满足时替换稳定状态：

- `result.version <= current_version`；
- 版本落后不超过 2；
- residual 有限且达标；
- Ncut 有限；
- 区域映射完整。

旧结果不会触发 cooldown，只是不参与新决策。

### 5.5 为什么短锁 mailbox 优于盲目无锁

这里只有一个 producer 和一个 consumer，锁只保护两个 `shared_ptr` 的交换，临界区极短。相比自写 lock-free 队列，它更容易通过 TSan 验证，且不会成为毫秒级瓶颈。

若必须无锁，使用容量 2 的 SPSC 环形队列，元素只能是对象池索引或 immutable handle，不能在队列中放置并发修改的 `std::vector`。

---

## 6. 顶层伪代码

```cpp
PlanCycleResult PlanV41(const RobotState& robot) {
  UpdateMapAndFrontiers();

  SkeletonDelta delta = BuildLightweightDelta();
  if (!delta.empty()) {
    spectral_worker_.Submit(
        std::make_shared<const SkeletonDelta>(std::move(delta)));
  }

  if (auto state = spectral_worker_.TryConsume()) {
    if (ValidateSpectralState(*state, dtg_version_)) {
      stable_spectral_state_ = std::move(state);
    }
  }

  if (CurrentTargetStillCommitted(robot, current_target_)) {
    return ContinueCurrentTrajectory();
  }

  CandidateSet candidates = CollectLocalTopKFrontiers(robot, 8);

  if (stable_spectral_state_ &&
      VersionLag(*stable_spectral_state_) <= 2) {
    AddAdjacentRegionEntries(
        stable_spectral_state_->quotient, candidates);
  }

  Candidate next = SelectSingleMacroTarget(candidates);
  Path path = SearchToSingleTarget(robot.position, next);
  return BuildEdenViewpointAndAseoTrajectory(path, next);
}
```

---

## 7. 文件级改造

### 停止在线调用

| 当前函数 | V4.1 处理 |
|---|---|
| `BuildActiveDistanceMatrix()` | 删除全对矩阵，改为局部 TOP-K 与商图邻边 |
| `RunEohdtFallback()` | 不再生成完整 MST tour |
| `BuildSparseSupportSpectralGraph()` | 由持久化真实边骨架替代 |
| `ComputeEdgeBetweenness()` | 在线删除 |
| `CompressDegreeTwoChains()` | 改为增量持久化 chain |
| `BuildRegionAwareRoute()` | 删除完整候选路线 |
| `ComputeRouteMetrics()` | 不再比较两条完整 tour |
| `UpdateLockDebt()` | 删除 |
| `UpdateSpectralMode()` | 替换为 VALID/STALE/INVALID |

### 新增文件

```text
src/incremental_spectral_skeleton.cpp
src/spectral_worker.cpp
src/region_quotient_planner.cpp
src/single_macro_target_planner.cpp

include/mr_dtg_plus/incremental_spectral_skeleton.h
include/mr_dtg_plus/spectral_worker.h
include/mr_dtg_plus/region_quotient_planner.h
```

---

## 8. 参数起点

```yaml
SpectralV41:
  skeleton_max_nodes: 64
  dense_refresh_max_nodes: 48
  iterative_vector_count: 2
  iterative_max_iterations: 12
  residual_threshold: 1.0e-4
  exact_refresh_residual: 5.0e-4
  maximum_version_lag: 2

  quotient_max_regions: 8
  local_frontier_top_k: 8
  target_switch_margin_sec: 0.7
  minimum_target_commit_sec: 1.0
```

---

## 9. 实施顺序

1. 为 DTG 增删节点和边生成 delta；
2. 建立永久 worker，但先仅记录结果；
3. 将 corridor chain 改为持久化结构；
4. 改成直接稀疏 Laplacian；
5. 删除在线 betweenness；
6. 删除完整 EOHDT tour 和完整谱 tour；
7. 建立区域商图；
8. 上线单步 macro target；
9. 删除 soft-lock、debt、cooldown 状态机；
10. City/Maze 重复 20 次做延迟和方差验收。

---

## 10. 验收标准

| 指标 | 最低门槛 |
|---|---:|
| 主线程谱相关 P99 | ≤ 5 ms |
| Worker 谱计算 P99 | ≤ 30 ms，且不阻塞 UAV |
| 完整规划 P99 | 不超过 EDEN 基线 15% |
| UAV 等待谱结果次数 | 0 |
| 路径相对 EDEN 增长 | ≤ 2% |
| 同地图探索时间 Std | 不高于 EDEN 的 1.2 倍 |
| cooldown/lock debt | 已删除 |
| 活动 anchor 全对矩阵 | 已删除 |
| 每周期完整 MST tour | 已删除 |

---

## 11. 风险

- 增量 chain 的边删除与分支拆分实现复杂；
- 自研迭代特征求解必须做残差、正交性和稠密小图对照测试；
- 商图入口代价需要真实路径而非欧氏距离；
- 若骨架压缩策略错误，可能删除关键瓶颈。

---

## 12. 推荐结论

V4.1 最适合当前研究：保留 Fiedler 向量的理论贡献，同时把计算对象压缩到固定骨架，并从根本上删除 EOHDT tour、谱 tour、双路线比较和随机恢复状态机。

---

## 参考文献

1. Steinar Laenen, He Sun. *Dynamic Spectral Clustering with Provable Approximation Guarantee*. ICML 2024.
2. Peter Macgregor, He Sun. *A Tighter Analysis of Spectral Clustering, and Beyond*. ICML 2022.
3. Peter Macgregor, He Sun. *Local Algorithms for Finding Densely Connected Clusters*. ICML 2021.
