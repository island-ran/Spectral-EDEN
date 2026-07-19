# Spectral-EDEN V4.3：确定性 Nyström 地标谱区域图

> **方案定位**：全局信息与固定最坏计算量之间的折中。保留全局谱区域视图，但只对固定数量地标构成的小矩阵求解，再将标签扩展到完整 DTG。
>
> **代码基线**：`island-ran/Spectral-EDEN`，提交 `e97d65be6d347d20d7d2387d0328c9454bb85855`。

---

## 1. 方案摘要

设完整 DTG 有 `n` 个节点，只选择 `m` 个稳定地标：

\[
m\ll n,\qquad m\in[24,48]
\]

```text
完整稀疏 DTG
    ↓ 确定性地标选择
分叉点 + 活跃anchor + 瓶颈端点 + 拓扑远距节点
    ↓
构造稀疏节点-地标亲和 C 与地标子矩阵 W
    ↓
仅对 m×m 小矩阵求 u2、u3
    ↓ Nyström 扩展/标签传播
完整 DTG 区域标签
    ↓
区域商图
    ↓
只选择下一个区域入口或 frontier
```

当 `m` 固定时，特征求解从 `O(n^3)` 变为 `O(m^3)`；完整图相关工作主要是 `O(nm)` 或稀疏多源最短路，且全部放在 Worker。

---

## 2. 理论降维

### 2.1 Nyström 近似

把亲和矩阵按地标与非地标分块：

\[
K=
\begin{bmatrix}
W & B^T\\
B & C
\end{bmatrix}
\]

只求地标子问题的特征向量。简化扩展形式：

\[
\widetilde U=K_{n,m}U_m\Lambda_m^{-1}
\]

工程实现必须选择一种一致的 normalized affinity/Nyström 公式并完整推导，不能把不同论文中的普通 affinity、normalized Laplacian 与随机游走公式混用。

### 2.2 固定地标预算

```yaml
landmark_count: 32
landmark_max_count: 48
```

无论 DTG 有 200、1000 还是 5000 个节点，在线特征矩阵都不超过 `48×48`。

### 2.3 地标选择必须确定性

V3 的运行方差已经很高，因此禁止随机采样。优先级：

1. 真实 DTG 分叉节点，`degree != 2`；
2. 活跃 frontier anchor；
3. bridge/articulation 候选或低净空通道端点；
4. 当前区域入口；
5. 用拓扑距离执行确定性 farthest-point sampling 补足预算；
6. 所有平局按 H-node ID。

### 2.4 地标持久化

地标不在每周期全部重选。仅在：

- 地标节点删除；
- 与活动拓扑断开；
- 新关键分叉的重要度超过替换阈值；
- 活跃 frontier 集发生大幅结构变化；

时替换少量地标。

这样同一地图多次运行的地标序列和区域标签更稳定。

---

## 3. EOHDT 与 V3 剪枝

### 3.1 删除

- `BuildActiveDistanceMatrix()` 完整活动 anchor 全对距离；
- `RunEohdtFallback()` 每周期完整 Prim/double-tree tour；
- kNN + 第二套 MST 谱支撑图；
- 全量 corridor compression 作为主降维方法；
- `ComputeEdgeBetweenness()`；
- 完整 `BuildRegionAwareRoute()`；
- baseline 与 candidate 两条完整路线评估；
- soft-lock；
- lock debt；
- deferred/cooldown；
- 五项 confidence 模式机。

### 3.2 保留

- 稀疏真实 DTG；
- 多源 Dijkstra；
- 到一个目标的 A*/Dijkstra；
- frontier raw reachability；
- EDEN 局部视点；
- ASEO 连续和安全轨迹；
- actual gain。

---

## 4. 地标亲和构造

### 4.1 多源距离

从所有地标执行一次多源或分批 Dijkstra。每个节点只保留最近 `q` 个地标：

```yaml
nearest_landmarks_per_node: 4
```

亲和：

\[
C_{i\ell}=
\exp\!\left(-\frac{d(i,\ell)^2}{2\sigma^2}\right)
\]

不可达或不属于最近 `q` 的地标亲和为 0。

### 4.2 地标矩阵

地标之间只使用真实拓扑距离/边权，不使用欧氏捷径。

`W` 应做对称化和最小 degree 检查，随后构造固定小规模 normalized problem。

### 4.3 标签扩展

两种可选方法：

1. 正规 Nyström embedding 扩展后，对第二列做 sweep cut；
2. 仅在地标上切割，然后根据节点到各侧地标的总归一化亲和传播标签。

第一种理论更完整；第二种实现更稳。推荐先实现第二种做工程基线，再实现第一种作论文消融。

---

## 5. 区域商图与单目标规划

Nyström 只负责区域标签。将标签折叠为商图：

```cpp
struct QuotientRegion {
  int id = -1;
  std::vector<uint32_t> landmarks;
  std::vector<uint32_t> frontier_ids;
  std::vector<uint32_t> entry_h_ids;
  double total_gain = 0.0;
};
```

规划只比较：

- 当前区域中的 TOP-K frontier；
- 商图直接相邻区域入口；
- fail-open raw reachable frontier。

不生成完整 frontier tour。

下一区域代价：

\[
J(r)=T_{entry}(r)-\beta\log(1+G_r)+\eta H_r
\]

`H_r` 是实际访问后的低收益历史，不是区域锁定。

---

## 6. 路径与决策稳定性

### 6.1 区域 ID 持久化

新旧区域使用最大重叠/Jaccard 匹配。地标固定使该匹配比 V3 全图动态切割更稳定。

### 6.2 目标承诺

只在真实事件触发时重新选择目标：

- 目标失效；
- 路径失效；
- actual gain 连续低；
- 地标集/区域商图发生有效更新；
- 新候选时间优势超过固定裕量。

\[
T_{new}+\Delta_T<T_{current}
\]

没有 soft-lock 和时间 cooldown。

### 6.3 确定性约束

- 地标选择无随机数；
- farthest-point 起点固定；
- 平局按 H-node ID；
- Worker 输入按版本顺序合并；
- 每次仅消费最新合法版本；
- 参数固定，不根据 wall-time 随机切换模式。

---

## 7. 真正异步架构

### 7.1 Worker 持有全部重计算

```cpp
struct LandmarkGraphDelta {
  uint64_t version = 0;
  std::vector<NodeDelta> nodes;
  std::vector<EdgeDelta> edges;
  std::vector<FrontierDelta> frontiers;
};
```

Worker 内维护：

- 稀疏 DTG 副本；
- landmark set；
- 节点到最近 landmark 的亲和；
- `m×m` 地标矩阵；
- 最近 `u2/u3`；
- 区域标签；
- 商图。

主线程不执行多源 Dijkstra、矩阵构造、求解、扩展或传播。

### 7.2 永久 Worker

```cpp
class NystromWorker {
 public:
  void Start();
  void Stop();
  void Submit(std::shared_ptr<const LandmarkGraphDelta> delta);
  std::shared_ptr<const NystromState> TryConsume();

 private:
  void Run();

  std::thread worker_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::shared_ptr<const LandmarkGraphDelta> latest_delta_;
  std::shared_ptr<const NystromState> latest_result_;
  bool running_ = false;
};
```

新 delta 覆盖尚未开始处理的旧任务，避免 backlog。

### 7.3 过载策略

不使用 cooldown：

```text
Worker忙          新delta覆盖旧delta
结果过期          主线程继续当前目标
地标变化较小      增量更新
地标变化较大      Worker内部full refresh
求解失败          保留上一合法商图
版本落后>2        暂停谱建议，使用局部EDEN
新合法结果到达    自动恢复
```

恢复依赖数据有效性，不依赖随机或固定等待时间。

### 7.4 无锁队列选择

优先使用短锁 mailbox。若使用 SPSC：

- 容量 2；
- 传对象池索引；
- immutable snapshot；
- 满时覆盖最旧未消费任务；
- 使用 acquire/release 发布完整对象；
- 必须通过 TSan 和百万次压力测试。

---

## 8. Nyström Worker 伪代码

```cpp
NystromState SolveNystrom(
    SparseGraph& graph,
    LandmarkSet& landmarks,
    uint64_t version) {

  UpdateLandmarksDeterministically(graph, landmarks);

  SparseAffinity C = BuildNearestLandmarkAffinity(
      graph, landmarks, /*nearest_q=*/4);

  Eigen::MatrixXd W = BuildLandmarkMatrix(graph, landmarks);
  NormalizedNystromProblem problem = Normalize(W, C);

  Eigenpairs eig = DenseSmallestThree(problem.small_matrix);
  if (!ValidateEigenpairs(eig)) {
    return NystromState::Invalid(version);
  }

  Labels labels = PropagateOrExtend(problem, eig);
  RegionQuotientGraph quotient = BuildQuotient(graph, labels);

  return NystromState{
      version, landmarks, std::move(labels),
      std::move(quotient), eig.residual, true};
}
```

---

## 9. 顶层伪代码

```cpp
PlanCycleResult PlanV43(const RobotState& robot) {
  UpdateMapAndFrontiers();
  nystrom_worker_.Submit(BuildLightweightGraphDelta());

  if (auto result = nystrom_worker_.TryConsume()) {
    if (result->valid &&
        result->version + 2 >= dtg_version_) {
      stable_nystrom_state_ = std::move(result);
    }
  }

  if (CurrentTargetStillCommitted()) {
    return ContinueCurrentTrajectory();
  }

  CandidateSet candidates = LocalFrontiersTopK(8);
  if (stable_nystrom_state_) {
    AddAdjacentRegionEntries(
        stable_nystrom_state_->quotient, candidates);
  }

  if (candidates.empty()) {
    AddRawReachableFailOpen(candidates);
  }

  Candidate next = SelectOneMacroTarget(candidates);
  Path path = SearchToSingleTarget(next);
  return EdenLocalViewpointAndAseo(path, next);
}
```

---

## 10. 文件级改造

### 停止在线调用

| 当前模块 | V4.3 处理 |
|---|---|
| `BuildActiveDistanceMatrix()` | 删除 |
| `RunEohdtFallback()` 完整 tour | 删除 |
| `BuildSparseSupportSpectralGraph()` | 由 landmark affinity 替代 |
| `ComputeEdgeBetweenness()` | 删除 |
| `CompressDegreeTwoChains()` 全量调用 | 不再作为主降维手段 |
| `BuildRegionAwareRoute()` | 删除 |
| `ComputeRouteMetrics()` 双 tour | 删除 |
| `UpdateLockDebt()` | 删除 |
| `UpdateSpectralMode()` | 删除复杂状态机 |

### 新增

```text
src/nystrom_landmark_selector.cpp
src/nystrom_spectral_worker.cpp
src/region_quotient_planner.cpp
src/single_macro_target_planner.cpp

include/mr_dtg_plus/nystrom_landmark_selector.h
include/mr_dtg_plus/nystrom_spectral_worker.h
include/mr_dtg_plus/region_quotient_planner.h
```

---

## 11. 参数起点

```yaml
SpectralV43:
  landmark_count: 32
  landmark_max_count: 48
  nearest_landmarks_per_node: 4
  landmark_replace_margin: 0.20
  landmark_full_refresh_change_ratio: 0.25
  maximum_version_lag: 2

  region_max_count: 8
  local_frontier_top_k: 8
  target_switch_margin_sec: 0.7
  minimum_target_commit_sec: 1.0
```

---

## 12. 实施顺序

1. 实现确定性 landmark selector；
2. 用 32 个地标离线对 City DTG 做分区对照；
3. 实现 Worker 独立图副本；
4. 实现地标多源距离和稀疏亲和；
5. 实现小矩阵 normalized solve；
6. 先实现地标标签传播，再实现完整 Nyström 扩展；
7. 构建区域商图；
8. 删除全对距离、完整 MST tour、完整谱 tour；
9. 删除 soft-lock/debt/cooldown；
10. City、Maze、Parking 各运行 20 次。

---

## 13. 验收标准

| 指标 | 最低门槛 |
|---|---:|
| 在线特征矩阵最大维度 | ≤ 48×48 |
| 主线程谱相关 P99 | ≤ 4 ms |
| Worker P99 | ≤ 35 ms |
| UAV 等待 Worker | 0 |
| 同地图 landmark 集合一致率 | ≥ 95% |
| 探索时间 Std | ≤ EDEN 的 1.2 倍 |
| 路径相对 EDEN 增长 | ≤ 2% |
| cooldown/lock debt | 已删除 |

---

## 14. 风险

- landmark 质量直接决定区域质量；
- Nyström 归一化若实现不一致会得到错误嵌入；
- `O(nm)` 标签扩展必须留在 Worker；
- landmark 替换会引起标签变化，必须持久化区域 ID；
- 小矩阵稳定不代表完整扩展一定稳定，需要与精确 Fiedler 离线对照。

---

## 15. 推荐结论

V4.3 是全局区域信息、固定最坏求解规模和实现风险之间最均衡的折中。它比 V4.1 更容易锁定计算上界，比 V4.2 保留更多全局结构。

---

## 参考文献

1. Jingzhi Tu, Gang Mei, Francesco Piccialli. *An Improved Nyström Spectral Graph Clustering Using k-core Decomposition as a Sampling Strategy for Large Networks*. 2022.
2. Farhad Pourkamali-Anaraki. *Scalable Spectral Clustering with Nyström Approximation: Practical and Theoretical Aspects*. 2020.
3. Steinar Laenen, He Sun. *Dynamic Spectral Clustering with Provable Approximation Guarantee*. ICML, 2024.
