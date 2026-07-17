# Spectral-EDEN 研究方案：算法、代码与实验工程说明书

> **目标**：在保留 EDEN 的 DTG、EROI、曲率惩罚视点选择和 ASEO 高速安全轨迹规划的前提下，重构全局区域路由，使系统能够基于当前已知拓扑动态识别弱连接区域，并执行“当前区域尽量一次探索完、再跨瓶颈切换”的策略。  
> **工程原则**：谱计算失败时必须无缝回退到原始 EOHDT；任何“分区”都是当前时刻的在线估计，不是永久地图标签。

当前 EDEN 的真实调用链为：

```text
SingleExp::EdenPlan()
  └─ TargetPlanning()
       ├─ DTG_.TspApproxiPlan()      // 全局边界区域排序
       └─ DTG_.FindFastExpTarget()   // 第一边界区域内选择视点
```

`TargetPlanning()` 先调用 `TspApproxiPlan()` 得到 `route_h`，再调用 `FindFastExpTarget()` 选择局部探索目标。fileciteturn12file0L199-L223  
原 `TspApproxiPlan()` 先通过 `ParallelDijkstra()` 找出带有效 H-F 边的活跃 H-node，再构造距离矩阵并执行 `Prim()`。fileciteturn17file0L46-L136  
原距离缓存维护只对仍连接前沿的 H-node 进行优先更新，并限制单次回调约 25 ms。fileciteturn16file0L149-L204

---

# 第一模块：算法逻辑链与数据流（Algorithm Pipeline & Data Flow）

## 1.1 总体执行周期

Spectral-EDEN 不读取未知环境的完整分布。每次只使用当前已经建立的体素地图、H-node、H-H edge、H-F edge 和活跃 EROI，形成一个时变图：

```text
传感器观测
  → 动态体素地图更新
  → EROI / 有效视点更新
  → DTG 增量更新
  → 当前谱支撑图快照
  → 谱分区与谱序
  → 区域执行锁
  → 当前区域内视点选择
  → ASEO 轨迹
  → 飞行并获得新观测
  → 下一周期重新计算
```

系统维护两种不同状态：

- `estimated_partition_t`：本周期根据当前图计算出的动态谱分区，允许变化。
- `active_region_id_`：当前正在执行的区域，原则上锁定，直到区域前沿清空、失效或安全条件强制解锁。

---

## Step 0：地图和 DTG 增量更新

### 输入

- 深度图或点云。
- 当前位姿、速度、偏航角。
- 动态体素地图 `BM_`。
- 低分辨率可通行图 `LRM_`。
- EROI 数据 `EROI_`。
- 历史拓扑图 `DTG_`。

### 处理

1. 更新自由、占据和未知体素。
2. 更新前沿 EROI 及候选视点。
3. 删除已探索、遮挡或失效的 F-node/H-F edge。
4. 新建或重连 H-node、H-H edge、H-F edge。
5. 更新 H-node 间路径缓存 `h_dist_map_`。
6. 记录图变化版本号：

```cpp
uint64_t dtg_version_;
uint64_t frontier_version_;
```

EDEN 的 `BfUpdate()` 已经会获取局部有效视点、创建或更新 H-F 连接，并删除状态为已探索的 EROI。fileciteturn15file0L109-L170

### 输出

- 当前 DTG：$G_H(t)=(H_t,E_{HH,t})$。
- 当前活跃 H-F 连接集合。
- 当前地图版本号和拓扑版本号。
- `spectral_dirty_ = true/false`。

### 推荐代码位置

- `mr_dtg_plus.cpp::BfUpdate()`
- `eroi` 模块中的前沿更新函数
- `DistMaintTimerCallback()`

---

## Step 1：探索结束状态预检查

### 输入

- 当前可达活跃边界区域集合。
- 当前有效视点及其信息增益。
- `missed_regions_`。
- 最近 $K_{\mathrm{finish}}$ 次图更新状态。

### 处理

将“规划失败”和“探索完成”彻底分开。新增：

```cpp
enum class GlobalPlanStatus {
  SUCCESS,
  NO_ACTIVE_FRONTIER,
  NO_FEASIBLE_SEED,
  GRAPH_DISCONNECTED,
  SPECTRAL_FAILED,
  PATH_FAILED
};
```

EDEN 当前的 `TspApproxiPlan()` 在无法找到可行种子 H-node或 `tarhs.empty()` 时都会返回 `false`，上层统一解释为 route failed，不能可靠触发 FINISH。fileciteturn17file0L70-L94  
FSM 虽然存在 `FINISH` 状态，但当前 `LOCALPLAN` 失败分支并未区分真正完成和暂时规划失败。fileciteturn7file0L107-L129

在线结束条件采用连续确认：

```text
无可达活跃边界区域
AND 无有效高增益视点
AND missed_regions_ 为空
AND 图在连续若干周期内没有产生新前沿
```

### 输出

- `GlobalPlanStatus::NO_ACTIVE_FRONTIER`，或进入后续谱规划。
- `finish_confirm_count_`。
- 达到条件后切换 FSM 到 `FINISH`。

### 推荐代码位置

- `eden.h / eden.cpp`
- `eden_fsm.h / eden_fsm.cpp`
- `mr_dtg_plus.h`

---

## Step 2：提取当前活跃边界区域

### 输入

- 无人机当前位置 $p_t$。
- 当前局部可连接 H-node。
- 当前 DTG。

### 处理

沿用原 EDEN：

```text
GetHnodesBBX()
  → LRM_->GetDists()
  → ParallelDijkstra()
  → tarhs + paths
```

`ParallelDijkstra()` 将所有 `hf_edges_` 非空且从当前种子可达的 H-node 放入 `tarhs`。fileciteturn20file0L113-L195

定义：

- `tarhs`：活跃边界区域锚点。
- 一个锚点 H-node 及其 H-F edges 所连接的 EROI，构成一个边界区域。

### 输出

```cpp
std::vector<h_ptr> active_boundary_hnodes;  // 原 tarhs
std::vector<std::pair<double, std::list<h_ptr>>> root_paths;
```

若为空，返回 `NO_ACTIVE_FRONTIER`，不要进入特征分解。

---

## Step 3：构造谱支撑图，而不是只对 `tarhs` 构造瞬时完整图

### 输入

- 当前可达 H-node 子图。
- `active_boundary_hnodes`。
- H-H edge 的长度和路径。
- 可选的路径净空、边介数或通道宽度。

### 处理

### 推荐主方案：稀疏历史支撑图

谱节点集合定义为：

```text
所有活跃边界 H-node
+
连接这些活跃 H-node 的最短路径上的历史 H-node
+
当前无人机连接到谱图所需的种子 H-node
```

即：

$$
V_t^{\mathrm{spec}}
=
A_t
\cup
\bigcup_{h_i,h_j\in A_t}P_{ij}
\cup S_t
$$

其中：

- $A_t$ 是 `tarhs`；
- $P_{ij}$ 是 DTG 上连接 $h_i,h_j$ 的 H-node 路径；
- $S_t$ 是当前无人机可连接的种子 H-node。

这样做的原因：

1. 某个前沿被清空后，对应 H-node 不会立刻从谱图几何骨架中消失。
2. 分区根据已知通道结构计算，而不是只根据当前剩余前沿数量计算。
3. 图保持稀疏，谱割更接近真实拓扑瓶颈。

### 兼容对照方案：活跃区域完整相似图

保留原文档模式作为消融项：

```text
节点 = tarhs
边 = 任意可达节点对
权重 = exp(-最短路径距离 / sigma)
```

该模式修改少，但容易把“长距离分离”误当成“狭窄瓶颈”，并且当前沿消失时图维度突变。

### 输出

```cpp
SpectralGraphSnapshot snapshot;
snapshot.h_ids;
snapshot.id_to_index;
snapshot.active_anchor_indices;
snapshot.edge_list;
snapshot.graph_version;
```

### 推荐代码位置

- 新文件：`mr_dtg_plus_spectral.cpp`
- 声明：`mr_dtg_plus.h`
- 数据结构：`mr_dtg_plus_structures.h`

---

## Step 4：计算谱边权

### 输入

对每条谱图边 $e=(i,j)$：

- H-H edge 或短路径长度 $\ell_e$。
- 路径最小净空 $c_e$。
- 可选边介数/通行集中度 $b_e$。
- 当前所有边长的稳健尺度 $\sigma_\ell$。

### 处理

推荐由简单到复杂提供三级配置。

#### Level A：距离权重

$$
w_e
=
\exp\left(-\frac{\ell_e}{\sigma_\ell}\right)
$$

#### Level B：距离 + 净空

$$
w_e
=
\exp\left(-\frac{\ell_e}{\sigma_\ell}\right)
\left[
\min\left(1,\frac{c_e}{c_0}\right)
\right]^{\gamma_c}
$$

#### Level C：距离 + 净空 + 瓶颈集中度

$$
w_e
=
\exp\left(-\gamma_\ell\frac{\ell_e}{\sigma_\ell}\right)
\left[
\min\left(1,\frac{c_e}{c_0}\right)
\right]^{\gamma_c}
\frac{1}{1+\beta_b\widetilde b_e}
$$

工程约束：

```cpp
W(i, i) = 0.0;               // 不使用自环
w_e = std::max(w_e, w_min);  // 防止数值完全断开，可配置
sigma_l = max(median(lengths), eps);
```

### 输出

- 稀疏或稠密权重矩阵 $W_t$。
- 度向量 $d_i=\sum_jw_{ij}$。
- 图连通分量数量。
- 边权调试信息。

---

## Step 5：构造拉普拉斯矩阵并求前三个特征对

### 输入

- $W_t$。
- 旧特征向量和旧节点 ID 映射。
- 求解器配置。

### 处理

构造：

$$
D_t=\operatorname{diag}(d_1,\ldots,d_n)
$$

$$
L_t=D_t-W_t
$$

$$
L_{\mathrm{sym},t}
=
I-D_t^{-1/2}W_tD_t^{-1/2}
$$

求解：

$$
L_{\mathrm{sym},t}u_k(t)=\lambda_k(t)u_k(t)
$$

至少计算：

$$
\lambda_1,\lambda_2,\lambda_3,\quad u_2
$$

原因：

- $\lambda_2$ 衡量代数连通度。
- $\lambda_3-\lambda_2$ 才能判断“二分结构是否突出”。
- 只看 $\lambda_2<\bar\lambda_2/2$ 容易将图规模变化或普通稀疏化误判为瓶颈。

求解器策略：

```text
n <= n_dense_switch
  → Eigen::SelfAdjointEigenSolver

n > n_dense_switch
  → LOBPCG / 迭代求解，以上一周期特征向量热启动
```

### 输出

```cpp
double lambda2;
double lambda3;
Eigen::VectorXd fiedler;
double residual_norm;
bool converged;
double solve_time_ms;
```

求解失败、残差过大或图断开异常时，返回 `SPECTRAL_FAILED` 并回退 EOHDT。

---

## Step 6：增量热启动与新节点初始化

### 输入

- 上一周期节点 ID 到 Fiedler 值的映射。
- 新图节点。
- 新节点的已知邻居及边权。

### 处理

已有节点直接复制旧值。新节点初始化为邻居加权平均：

$$
f_{\mathrm{new}}
=
\frac{\sum_{j\in\mathcal N(\mathrm{new})}
w_{\mathrm{new},j}f_j}
{\sum_{j\in\mathcal N(\mathrm{new})}
w_{\mathrm{new},j}+\varepsilon}
$$

之后进行：

1. 去均值。
2. 与平凡特征向量正交。
3. 归一化。
4. 作为迭代求解器初值。

不要在 `BfUpdate()` 中直接向一个旧维度 `Eigen::VectorXd` 尾部插值后当作最终谱状态；应以 `h_id` 映射重建初始向量，避免节点删除、重排导致索引错位。

### 输出

- `warm_start_vector`。
- `hot_start_available`。
- 新旧节点匹配率。

---

## Step 7：Fiedler 向量时间对齐

### 输入

- 当前 $u_2(t)$。
- 上一周期扩维后的 $\widetilde u_2(t-1)$。
- 共同节点索引。

### 处理

Fiedler 向量符号不唯一，必须对齐：

$$
s_t
=
\operatorname{sign}
\left(
\widetilde u_2(t-1)^\top u_2(t)
\right)
$$

$$
u_2(t)\leftarrow s_tu_2(t)
$$

首次计算没有历史向量时，以当前无人机最近的谱节点定向：

```text
让当前节点位于谱坐标较小的一侧；
若无法判断，则比较正序和逆序的首段路径代价。
```

### 输出

- 符号稳定的 `fiedler_aligned`。
- `sign_flipped` 调试标志。

---

## Step 8：判断是否真的需要分区

### 输入

- $n$、$\lambda_2$、$\lambda_3$。
- Fiedler 候选切割。
- 历史指数滑动平均。
- 连续触发计数。

### 处理

维护：

$$
\bar\lambda_2(t)
=
\alpha\bar\lambda_2(t-1)
+
(1-\alpha)\lambda_2(t)
$$

候选二分由 Fiedler sweep cut 或一维 $k$-means 得到。推荐优先使用 sweep cut：对 Fiedler 值排序，枚举相邻切点并选最小 Ncut。

触发条件：

$$
\operatorname{Trigger}(t)=
\mathbf 1
\left[
\begin{aligned}
n&\ge N_{\min}\\
\lambda_2&\le\tau_{\mathrm{conn}}\\
\lambda_3-\lambda_2&\ge\tau_{\mathrm{gap}}\\
\operatorname{Ncut}(C_0,C_1)&\le\tau_{\mathrm{ncut}}
\end{aligned}
\right]
$$

可选加入相对下降条件：

$$
\frac{\lambda_2(t)}
{\bar\lambda_2(t)+\varepsilon}
\le\rho_\lambda
$$

只有连续 $M_{\mathrm{trigger}}$ 次成立，才确认分区。

### 输出

```cpp
bool partition_valid;
std::vector<int> raw_cluster_labels;
double ncut;
int trigger_streak;
```

不满足时仍可使用 Fiedler 谱序，但不启用强制区域锁。

---

## Step 9：动态分区标签匹配与滞回

### 输入

- 本周期原始簇标签。
- 上周期簇标签。
- 节点 ID 集合。
- Fiedler 阈值 $\theta_t$。

### 处理

### 9.1 标签匹配

通过共同节点的 Jaccard overlap 或 Hungarian matching，将新标签映射到旧 `persistent_region_id`，避免 0/1 标签交换。

$$
J(C_a^{t-1},C_b^t)
=
\frac{|C_a^{t-1}\cap C_b^t|}
{|C_a^{t-1}\cup C_b^t|}
$$

### 9.2 节点标签滞回

$$
c_i(t)=
\begin{cases}
0,&f_i<\theta_t-\delta_f\\
1,&f_i>\theta_t+\delta_f\\
c_i(t-1),&|f_i-\theta_t|\le\delta_f
\end{cases}
$$

### 9.3 分裂与合并

- 一个旧区域对应多个新区域：记录 `SPLIT`，当前执行区域继承与无人机重叠最大的子区域。
- 多个旧区域对应一个新区域：记录 `MERGE`，清理重复 `missed_regions_`。
- 匹配低于 $\tau_{\mathrm{match}}$：创建新持久区域 ID。

### 输出

```cpp
std::unordered_map<int, int> h_id_to_region_id;
std::vector<RegionState> regions_;
PartitionChangeType change_type;
```

---

## Step 10：区域执行锁与区域穷尽判断

### 输入

- 当前无人机所属区域。
- 每个区域的活跃前沿集合。
- `missed_regions_`。
- 上周期 `active_region_id_`。

### 处理

当前执行区域不因一次谱重算而立即变化。

区域 $r$ 的活跃边界集合：

$$
\mathcal B_r(t)
=
\left\{
h_i\in A_t:
\operatorname{region}(h_i)=r
\right\}
$$

区域完成条件：

$$
\operatorname{RegionDone}(r,t)=
\mathbf 1
\left[
\bigcap_{k=0}^{K_{\mathrm{region}}-1}
\mathcal B_r(t-k)=\varnothing
\right]
$$

执行状态转移：

```text
active_region_ 仍有有效前沿
  → 保持锁定

active_region_ 连续 K_region 周期无前沿
  → 标记 DONE
  → 从 missed_regions_ 选择下一区域

active_region_ 与当前图失配/不可达/路径失效
  → 进入 RECOVERY
  → 允许提前解锁，但必须记录原因
```

建议区域状态：

```cpp
enum class RegionExecState {
  CANDIDATE,
  ACTIVE,
  DORMANT,
  DONE,
  STALE,
  UNREACHABLE
};
```

### 输出

- `active_region_id_`。
- 当前允许参与全局排序的 H-node 子集。
- `missed_regions_`。
- 区域切换事件计数。

---

## Step 11：生成谱序区域路线

### 输入

- 当前允许访问的活跃区域 H-node。
- 对齐后的 Fiedler 值。
- 当前区域锁状态。
- 起点到各节点距离。
- 原 `dist_mat`，用于回退和路线方向比较。

### 处理

### 11.1 有区域锁

只排序当前区域内的活跃边界：

$$
R_t
=
\operatorname{Sort}_{f_i}
\left(
\mathcal B_{r_{\mathrm{active}}}(t)
\right)
$$

### 11.2 无区域锁

构造两条候选路线：

```text
R_forward  = Fiedler 升序
R_reverse  = Fiedler 降序
```

比较：

$$
J_{\mathrm{route}}(R)
=
d(p_t,R_1)
+
\lambda_{\mathrm{jump}}
\sum_{k=1}^{m-1}d(R_k,R_{k+1})
-
\lambda_{\mathrm{terminal}}d(p_t,R_m)
$$

取 $J_{\mathrm{route}}$ 较小者。最后一项保留 EDEN “较远端尽量最后执行”的思想。

### 11.3 分区级顺序

若存在多个已确认区域：

1. 当前区域优先。
2. 其他区域按区域中心谱坐标排序。
3. 同分情况下按转移距离和未完成前沿数排序。

### 输出

```cpp
std::vector<h_ptr> route_h;
std::vector<int> route_region_ids;
```

若为空或谱计算异常，执行原始 `Prim + banned_leaf + DFS`。

---

## Step 12：局部视点评分加入谱一致性项

### 输入

- EDEN 原始视点运动代价。
- 候选视点所属 H-node 的 Fiedler 值。
- 切割阈值 $\theta_t$。
- 候选视点区域和 `active_region_id_`。

### 处理

原文档使用 $|f_i|$ 判断节点是否靠近切割边界，但切割阈值不一定为零。推荐改为相对阈值距离：

$$
q_i^{\mathrm{boundary}}
=
1-
\frac{|f_i-\theta_t|}
{\max_j|f_j-\theta_t|+\varepsilon}
$$

跨区指示量：

$$
q_i^{\mathrm{cross}}
=
\mathbf 1
\left[
r_i\neq r_{\mathrm{active}}
\right]
$$

谱附加时间代价：

$$
\Delta t_{\mathrm{spec}}
=
\left(
\lambda_s q_i^{\mathrm{boundary}}
+
\lambda_{\mathrm{cross}}q_i^{\mathrm{cross}}
\right)
\frac{d}{v_{\max}}
$$

最终：

$$
t_{\mathrm{modified}}
=
t_{\mathrm{EDEN}}
+
\Delta t_{\mathrm{spec}}
$$

$$
G_{\mathrm{spec}}
=
\exp(-\lambda_e t_{\mathrm{modified}})
$$

注意：

- `FindFastExpTarget()` 正常只在 `route_h.front()` 对应边界区域内选视点，所以 $\lambda_{\mathrm{cross}}$ 常为零；它主要保护虚拟 H-F edge、局部补充候选或未来扩展。
- 谱项只做 tie-breaker，不能压过安全、可达性和曲率约束。
- H-node 保存 `fiedler_val_`、`spectral_region_id_`、`spectral_epoch_`，读取时必须检查 epoch，防止使用过期值。

### 输出

- `gain1/gain2`。
- 选中的 exploring、continuous、safety viewpoints。
- 谱惩罚调试量。

---

## Step 13：ASEO 轨迹、执行和反馈

### 输入

- `route_h.front()`。
- 探索、连续和安全视点。
- 当前动力学状态。

### 处理

沿用 EDEN 的 ASEO，不修改核心优化器：

```text
FindFastExpTarget()
  → corridor generation
  → AggressiveBranchTrajOptimize()
  → 发布轨迹
```

谱方法只改变“先探索哪个区域”和“同分候选时避免靠近跨区瓶颈”，不改底层安全轨迹约束。

### 输出

- 可执行轨迹。
- 下一周期的新观测。
- 区域切换、前沿清空、路线变化等日志。

---

## 1.2 推荐函数调用顺序

```cpp
GlobalPlanStatus MultiDtgPlus::PlanGlobalRoute(
    const Eigen::Vector3d& robot_pos,
    std::vector<h_ptr>& route_h,
    std::vector<Eigen::Vector3d>& path_to_first,
    double& dist_to_first) {

  auto status = CollectActiveBoundaryRegions(
      robot_pos, active_hnodes, root_paths, dist_mat);

  if (status != GlobalPlanStatus::SUCCESS) {
    return status;
  }

  if (NeedSpectralUpdate()) {
    SpectralGraphSnapshot snapshot;
    if (!BuildSpectralSupportGraph(active_hnodes, snapshot) ||
        !BuildSpectralWeights(snapshot) ||
        !SolveSpectralEmbedding(snapshot) ||
        !AlignFiedlerTemporal(snapshot)) {
      return RunEOHDTFallback(...);
    }

    DetectPartition(snapshot);
    MatchAndStabilizeRegions(snapshot);
    UpdateRegionExecutionState(snapshot);
    PublishSpectralStateToHnodes(snapshot);
  }

  if (!BuildSpectralRoute(active_hnodes, dist_mat, route_h)) {
    return RunEOHDTFallback(...);
  }

  if (!BuildPathToFirstRegion(...)) {
    return GlobalPlanStatus::PATH_FAILED;
  }

  return GlobalPlanStatus::SUCCESS;
}
```

---

## 1.3 推荐文件修改清单

| 文件 | 修改内容 |
|---|---|
| `mr_dtg_plus_structures.h` | 新增 `SpectralConfig`、`SpectralGraphSnapshot`、`RegionState`；在 `H_node` 中增加谱值、区域 ID、epoch |
| `mr_dtg_plus.h` | 声明谱图构建、求解、分区匹配、区域锁、回退接口；新增成员状态 |
| `mr_dtg_plus.cpp` | 修改 `BfUpdate()`、`DistMaintTimerCallback()`、全局路由入口；维护版本号 |
| `mr_dtg_plus_path_search.cpp` | 保留 `ParallelDijkstra()`、`Prim()`；修改 `FindFastExpTarget()` 的评分调用 |
| `mr_dtg_plus_spectral.cpp`（建议新增） | 集中实现图快照、权重、拉普拉斯、特征求解、切割、匹配、谱序 |
| `eroi.h/.cpp` | 提供区域活跃前沿数量、有效视点增益、前沿状态查询 |
| `eden.h/.cpp` | 将布尔规划结果改为状态码；实现结束确认器 |
| `eden_fsm.h/.cpp` | `NO_ACTIVE_FRONTIER` 连续确认后进入 `FINISH`；其他错误进入重规划/恢复 |
| ROS YAML 配置 | 暴露全部谱参数、阈值和开关 |
| 日志模块 | 记录特征值、Ncut、区域 ID、切换、BFM、求解时间和 fallback 原因 |

---

# 第二模块：核心公式与变量映射字典（Math & Variable Dictionary）

## 2.1 探索完成定义

理论严格定义：

$$
\mu_3\left(
\mathcal U_t\cap\mathcal R^\star
\right)=0
$$

在线工程定义：

$$
\operatorname{Finish}(t)=
\mathbf 1
\left[
\begin{aligned}
&\mathcal B_{t-k}=\varnothing,\\
&\sup_{q\in\mathcal Q_{t-k}^{\mathrm{valid}}}I_t(q)\le\varepsilon_I,\\
&\Delta_G(t-k)\le\varepsilon_G,
\end{aligned}
\quad
k=0,\ldots,K_{\mathrm{finish}}-1,
\quad
\mathcal M_t=\varnothing
\right]
$$

## 2.2 活跃边界区域集合

$$
A_t
=
\left\{
h\in\operatorname{Reach}_{G_H(t)}(S_t):
\deg_{HF,t}(h)>0
\right\}
$$

代码中近似对应 `ParallelDijkstra()` 输出的 `tarhs`。

## 2.3 谱支撑图

$$
V_t^{\mathrm{spec}}
=
A_t
\cup
\bigcup_{h_i,h_j\in A_t}P_{ij}
\cup S_t
$$

## 2.4 距离和瓶颈感知权重

$$
\sigma_\ell
=
\operatorname{median}
\left\{
\ell_e:e\in E_t^{\mathrm{spec}}
\right\}
$$

$$
w_e
=
\exp\left(-\gamma_\ell\frac{\ell_e}{\sigma_\ell}\right)
\left[
\min\left(1,\frac{c_e}{c_0}\right)
\right]^{\gamma_c}
\frac{1}{1+\beta_b\widetilde b_e}
$$

距离-only 对照组：

$$
w_{ij}
=
\exp\left(-\frac{d_{ij}}{\sigma_d}\right)
$$

## 2.5 图拉普拉斯与谱嵌入

$$
D_{ii}=\sum_jW_{ij}
$$

$$
L=D-W
$$

$$
L_{\mathrm{sym}}
=
I-D^{-1/2}WD^{-1/2}
$$

$$
L_{\mathrm{sym}}u_k
=
\lambda_ku_k
$$

$$
0=\lambda_1\le\lambda_2\le\lambda_3\le\cdots
$$

## 2.6 新节点热启动

$$
f_{\mathrm{new}}
=
\frac{
\sum_{j\in\mathcal N(\mathrm{new})}
w_{\mathrm{new},j}f_j
}{
\sum_{j\in\mathcal N(\mathrm{new})}
w_{\mathrm{new},j}+\varepsilon
}
$$

## 2.7 时间符号对齐

$$
u_2(t)
\leftarrow
\operatorname{sign}
\left(
\widetilde u_2(t-1)^\top u_2(t)
\right)u_2(t)
$$

## 2.8 归一化割

$$
\operatorname{cut}(C_0,C_1)
=
\sum_{i\in C_0,j\in C_1}w_{ij}
$$

$$
\operatorname{vol}(C)
=
\sum_{i\in C}\sum_jw_{ij}
$$

$$
\operatorname{Ncut}(C_0,C_1)
=
\frac{\operatorname{cut}(C_0,C_1)}
{\operatorname{vol}(C_0)}
+
\frac{\operatorname{cut}(C_0,C_1)}
{\operatorname{vol}(C_1)}
$$

## 2.9 分区触发

$$
\bar\lambda_2(t)
=
\alpha\bar\lambda_2(t-1)
+
(1-\alpha)\lambda_2(t)
$$

$$
\operatorname{Trigger}(t)=
\mathbf 1
\left[
n\ge N_{\min}
\land
\lambda_2\le\tau_{\mathrm{conn}}
\land
\lambda_3-\lambda_2\ge\tau_{\mathrm{gap}}
\land
\operatorname{Ncut}\le\tau_{\mathrm{ncut}}
\land
\frac{\lambda_2}{\bar\lambda_2+\varepsilon}\le\rho_\lambda
\right]
$$

## 2.10 标签滞回

$$
c_i(t)=
\begin{cases}
0,&f_i<\theta_t-\delta_f\\
1,&f_i>\theta_t+\delta_f\\
c_i(t-1),&|f_i-\theta_t|\le\delta_f
\end{cases}
$$

## 2.11 区域匹配

$$
J(C_a^{t-1},C_b^t)
=
\frac{|C_a^{t-1}\cap C_b^t|}
{|C_a^{t-1}\cup C_b^t|}
$$

## 2.12 区域完成

$$
\mathcal B_r(t)
=
\left\{
h_i\in A_t:
\operatorname{region}(h_i)=r
\right\}
$$

$$
\operatorname{RegionDone}(r,t)
=
\mathbf 1
\left[
\bigcap_{k=0}^{K_{\mathrm{region}}-1}
\mathcal B_r(t-k)=\varnothing
\right]
$$

## 2.13 谱一致性视点评分

$$
q_i^{\mathrm{boundary}}
=
1-
\frac{|f_i-\theta_t|}
{\max_j|f_j-\theta_t|+\varepsilon}
$$

$$
q_i^{\mathrm{cross}}
=
\mathbf 1[r_i\neq r_{\mathrm{active}}]
$$

$$
\Delta t_{\mathrm{spec}}
=
\left(
\lambda_s q_i^{\mathrm{boundary}}
+
\lambda_{\mathrm{cross}}q_i^{\mathrm{cross}}
\right)
\frac{d}{v_{\max}}
$$

$$
G_{\mathrm{spec}}
=
\exp
\left[
-\lambda_e
\left(
t_{\mathrm{EDEN}}+\Delta t_{\mathrm{spec}}
\right)
\right]
$$

## 2.14 路线方向选择

$$
J_{\mathrm{route}}(R)
=
d(p_t,R_1)
+
\lambda_{\mathrm{jump}}
\sum_{k=1}^{m-1}d(R_k,R_{k+1})
-
\lambda_{\mathrm{terminal}}d(p_t,R_m)
$$

## 2.15 BFM 指标定义

设 $r_t$ 为执行区域 ID，给定回访窗口 $T_B$：

$$
N_{\mathrm{BFM}}
=
\sum_t
\mathbf 1
\left[
r_{t-\Delta_1}=a,
r_t=b,
r_{t+\Delta_2}=a,
a\neq b,
0<\Delta_1,\Delta_2\le T_B
\right]
$$

更直接的工程指标：

$$
N_{\mathrm{cross}}
=
\sum_t\mathbf 1[r_t\neq r_{t-1}]
$$

$$
N_{\mathrm{premature}}
=
\sum_t
\mathbf 1
\left[
r_t\neq r_{t-1}
\land
|\mathcal B_{r_{t-1}}(t)|>0
\right]
$$

## 2.16 变量—代码映射表

| 数学符号 | 物理/几何含义 | 在代码中对应的参数/数据结构 | 数据类型 |
|---|---|---|---|
| $t$ | 当前规划/地图更新时间 | ROS time、规划周期索引 | 标量 |
| $p_t$ | 无人机当前位置 | `Eigen::Vector3d ps` / `p_` | 3D 向量 |
| $G_H(t)$ | 当前 H-node 历史拓扑图 | `H_list_`、`H_node::hh_edges_` | 图 |
| $H_t$ | 当前历史节点集合 | `std::list<h_ptr> H_list_` | 节点容器 |
| $E_{HH,t}$ | H-H 拓扑边集合 | `hhe_ptr`、`H_node::hh_edges_` | 边容器 |
| $E_{HF,t}$ | H-F 连接边集合 | `hfe_ptr`、`H_node::hf_edges_` | 边容器 |
| $S_t$ | 当前可连接种子 H-node | `p_hs` | `std::vector<h_ptr>` |
| $A_t$ | 当前活跃边界区域锚点集合 | `tarhs` / `active_boundary_hnodes` | `std::vector<h_ptr>` |
| $\deg_{HF}(h)$ | H-node 上有效前沿边数量 | `h->hf_edges_.size()`，需过滤失效边 | 整数 |
| $\mathcal M_t$ | 未完成区域集合 | `missed_regions_` | 栈/队列 |
| $V_t^{\mathrm{spec}}$ | 谱支撑图节点集合 | `SpectralGraphSnapshot::h_ids` | ID 向量 |
| $P_{ij}$ | 两 H-node 间支撑路径 | `h_dist_map_` 中路径，或 DTG shortest path | H-node/点序列 |
| $d_{ij}$ | 两边界区域 DTG 最短路距离 | `h_dist_map_[key].first`、`dist_mat(i,j)` | 标量，m |
| $\ell_e$ | 一条谱边的路径长度 | `hhe::length_` 或路径累积长度 | 标量，m |
| $c_e$ | 谱边路径上的最小净空 | 新增 `ComputePathClearance(edge.path_)` | 标量，m |
| $\widetilde b_e$ | 归一化边介数/集中度 | 新增 `edge_betweenness` 缓存 | 标量 |
| $\sigma_\ell$ | 边长稳健尺度 | `spectral_config_.sigma_scale * median_length` | 标量 |
| $c_0$ | 净空参考尺度 | `spectral_config_.clearance_ref` | 标量，m |
| $\gamma_\ell$ | 距离衰减指数系数 | `spectral_config_.length_weight` | 标量 |
| $\gamma_c$ | 净空项指数 | `spectral_config_.clearance_power` | 标量 |
| $\beta_b$ | 边介数惩罚权重 | `spectral_config_.betweenness_weight` | 标量 |
| $W$ | 谱图权重矩阵 | `spectral_data_.W` 或 sparse matrix | 矩阵 |
| $D$ | 度矩阵 | `spectral_data_.degree` / `D` | 向量/对角矩阵 |
| $L$ | 未归一化拉普拉斯 | `spectral_data_.L` | 矩阵 |
| $L_{\mathrm{sym}}$ | 对称归一化拉普拉斯 | `spectral_data_.L_norm` | 矩阵 |
| $\lambda_2$ | 代数连通度 | `spectral_data_.lambda2` | 标量 |
| $\lambda_3$ | 第三小特征值 | 新增 `spectral_data_.lambda3` | 标量 |
| $\bar\lambda_2$ | $\lambda_2$ 指数滑动平均 | `spectral_data_.lambda2_ema` | 标量 |
| $u_2,f$ | Fiedler 向量 | `spectral_data_.fiedler` | 动态向量 |
| $f_i$ | H-node 的当前谱坐标 | `H_node::fiedler_val_` | 标量 |
| $\theta_t$ | Fiedler 切割阈值 | `spectral_data_.cut_threshold` | 标量 |
| $C_0,C_1$ | 当前候选谱分区 | `raw_cluster_labels` | 整数标签向量 |
| $r_i$ | 节点持久区域 ID | `H_node::spectral_region_id_` | 整数 |
| $r_{\mathrm{active}}$ | 当前执行区域 ID | `active_region_id_` | 整数 |
| $\operatorname{Ncut}$ | 分区质量 | `spectral_data_.ncut` | 标量 |
| $\delta_f$ | 标签滞回宽度 | `spectral_config_.label_hysteresis` | 标量 |
| $N_{\min}$ | 允许谱分区的最少节点数 | `spectral_config_.min_partition_nodes` | 整数 |
| $\tau_{\mathrm{conn}}$ | 小连通度阈值 | `spectral_config_.lambda2_threshold` | 标量 |
| $\tau_{\mathrm{gap}}$ | 特征值间隔阈值 | `spectral_config_.eigengap_threshold` | 标量 |
| $\tau_{\mathrm{ncut}}$ | 最大可接受 Ncut | `spectral_config_.ncut_threshold` | 标量 |
| $\rho_\lambda$ | 相对历史连通度阈值 | `spectral_config_.lambda2_ratio` | 标量 |
| $\alpha$ | EMA 平滑系数 | `spectral_config_.lambda2_ema_alpha` | 标量 |
| $M_{\mathrm{trigger}}$ | 分区触发连续次数 | `spectral_config_.trigger_persistence` | 整数 |
| $K_{\mathrm{region}}$ | 区域完成连续确认次数 | `spectral_config_.region_done_cycles` | 整数 |
| $K_{\mathrm{finish}}$ | 全图完成连续确认次数 | `spectral_config_.finish_cycles` | 整数 |
| $\varepsilon_I$ | 最小有效信息增益 | `spectral_config_.finish_gain_eps` | 标量，体素数或 m³ |
| $\varepsilon_G$ | 图稳定阈值 | `spectral_config_.graph_change_eps` | 标量/整数 |
| $\tau_{\mathrm{match}}$ | 新旧区域匹配阈值 | `spectral_config_.region_match_threshold` | 标量 |
| $\lambda_s$ | 切割边界视点惩罚 | `spectral_config_.spectral_view_weight` | 标量 |
| $\lambda_{\mathrm{cross}}$ | 跨当前区域惩罚 | `spectral_config_.cross_region_weight` | 标量 |
| $d$ | 当前视点路径距离 | `dist` / `lrn->path_g_` | 标量，m |
| $v_{\max}$ | 最大线速度 | EDEN 原 `v_max_` | 标量，m/s |
| $\lambda_e$ | EDEN 指数评分系数 | EDEN 原 `lambda_e_` | 标量 |
| $\lambda_{\mathrm{jump}}$ | 谱序相邻跳跃代价权重 | `spectral_config_.route_jump_weight` | 标量 |
| $\lambda_{\mathrm{terminal}}$ | 最远端最后偏好 | `spectral_config_.terminal_bias` | 标量 |
| $\varepsilon$ | 数值稳定下界 | `spectral_config_.numeric_eps` | 标量 |
| $R$ | 最终边界区域路线 | `route_h` | `std::vector<h_ptr>` |
| $N_{\mathrm{BFM}}$ | A→B→A 型反复横跳次数 | 由 `active_region_id_` 日志离线统计 | 整数 |
| $N_{\mathrm{premature}}$ | 未清空前沿就离区次数 | 新增在线计数器 | 整数 |
| $\Delta_G$ | 相邻周期图变化量 | `dtg_version_`、节点/边增删统计 | 标量/结构体 |

---

# 第三模块：消融实验与超参数指南（Ablation Study Guide）

## 3.1 必做的模块级消融

先做模块级消融，再做参数微调；否则无法判断提升来自哪一部分。

| 编号 | 配置 | 目的 |
|---|---|---|
| A0 | 原始 EDEN：EOHDT + 原视点评分 | 基线 |
| A1 | 仅用 Fiedler 谱序替换 Prim+DFS | 验证谱序本身 |
| A2 | A1 + 历史支撑图，替代仅 `tarhs` 完整图 | 验证图建模稳定性 |
| A3 | A2 + 净空/瓶颈感知边权 | 验证是否真正识别窄入口 |
| A4 | A3 + $\lambda_2/\lambda_3$/Ncut 触发 | 验证可靠分区 |
| A5 | A4 + 符号对齐、标签匹配、滞回 | 验证动态稳定性 |
| A6 | A5 + 区域执行锁和 `missed_regions_` | 验证 BFMs 抑制核心 |
| A7 | A6 + 谱一致性视点评分 | 验证局部 tie-breaker |
| A8 | A7 + 热启动/增量求解 | 验证实时性 |
| A9 | 完整方案，但关闭 EOHDT fallback | 仅用于压力测试，不用于正式飞行 |

核心比较指标：

- 总探索时间 $T_{\mathrm{exp}}$。
- 总飞行路径长度 $L_{\mathrm{path}}$。
- 平均速度和速度标准差。
- $N_{\mathrm{BFM}}$、$N_{\mathrm{cross}}$、$N_{\mathrm{premature}}$。
- 区域回访总距离。
- 完成率。
- 全局规划平均、P95、P99 时间。
- 谱计算平均、P95 时间。
- fallback 次数和原因。
- CPU 峰值、内存峰值。
- 碰撞/紧急制动次数。

---

## 3.2 统一实验场景矩阵

所有参数至少在以下场景上测试，避免只对一种迷宫过拟合。

| 场景 | 必须包含的结构 | 用途 |
|---|---|---|
| S1 长走廊+死胡同 | 单入口深区、多个支路 | 测 BFMs 和区域穷尽 |
| S2 多房间门洞 | 短但窄的连接 | 测净空权重 |
| S3 环路地图 | 两个区域之间存在多条通路 | 测错误切割和区域合并 |
| S4 大型开放空间 | 前沿多、瓶颈弱 | 测误分区率 |
| S5 多层/坡道 3D | 垂向连接、不同高度 | 测三维适用性 |
| S6 动态揭示通道 | 后期出现第二连接路径 | 测动态重分区 |
| S7 稀疏前沿后期 | 前沿逐渐减少 | 测支撑图稳定性 |
| S8 传感器噪声/遮挡 | 前沿短暂消失再出现 | 测完成条件和滞回 |

建议：

- 每组参数每个场景至少 10 个随机种子。
- 报告均值、标准差、95% 置信区间。
- 参数选择基于验证场景，最终结果在独立测试场景报告。
- 不允许只报告最好一次运行。

---

## 3.3 参数级消融表

### A. 谱图建模参数

| 待调参数 | 为什么不能固定 | 建议搜索空间 | 核心指标 |
|---|---|---|---|
| `graph_mode`：`ACTIVE_COMPLETE` / `SUPPORT_SPARSE` | 活跃完整图修改少但不稳定；支撑图稳定但节点更多 | 离散二选一 | 探索时间、分区稳定度、谱耗时 |
| `support_path_mode` | 全部两两路径可能引入太多节点；MST 路径又可能丢结构 | `all_shortest_paths`、`k_nearest_paths`、`union_of_route_paths` | 节点数、内存、误切率 |
| $k_{\mathrm{nn}}$ | 完整图过密会抹平瓶颈，过稀会断图 | $\{3,4,5,6,8\}$ | 连通率、Ncut、误分区率 |
| $r_{\mathrm{spec}}$ | 固定半径对不同地图尺度不通用 | $[5,30]$ m 或 $\{1,1.5,2,3\}\sigma_\ell$ | 图连通率、规划时间 |
| $\gamma_\ell$ | 控制距离衰减强弱，过大导致图碎裂，过小导致所有节点相似 | $\{0.25,0.5,1,1.5,2,3\}$ | Ncut、路径长度、分区数量 |
| $\sigma_\ell$ 的缩放因子 $\kappa_\sigma$ | 中位数适应地图尺度，但不同图密度仍需调整 | $\{0.5,0.75,1,1.5,2\}$ | 分区稳定性、误切率 |
| $c_0$ | 与 UAV 尺寸、地图分辨率、通道宽度有关 | $[1.2,4.0]\times$ UAV 安全直径 | 窄门识别率、碰撞裕度 |
| $\gamma_c$ | 决定净空对边权的非线性影响 | $\{0,0.5,1,2,3\}$ | 窄门场景探索时间、误切率 |
| $\beta_b$ | 介数高不一定总是瓶颈；环路中可能误惩罚主干 | $\{0,0.25,0.5,1,2\}$ | BFMs、错误切割、额外计算 |
| $w_{\min}$ | 太小导致数值断图，太大掩盖真实弱连接 | $\{0,10^{-6},10^{-5},10^{-4},10^{-3}\}$ | 连通分量数、残差、fallback |
| `max_spectral_nodes` | 限制计算量，但裁剪会丢全局结构 | $\{30,50,80,120,200\}$ | P95 求解时间、路径质量 |

### B. 分区触发参数

| 待调参数 | 为什么不能固定 | 建议搜索空间 | 核心指标 |
|---|---|---|---|
| $N_{\min}$ | 节点太少时特征值不稳定；太大则错过早期瓶颈 | $\{4,6,8,10,12\}$ | 首次正确分区时间、误触发率 |
| $\alpha$ | 平滑过强响应慢，过弱对图变化敏感 | $\{0.7,0.8,0.9,0.92,0.95,0.98\}$ | 触发延迟、标签抖动 |
| $\tau_{\mathrm{conn}}$ | $\lambda_2$ 随图规模和权重尺度变化 | 训练集分位数 $P_{10}$～$P_{40}$；或 $[0.01,0.3]$ | precision/recall、误触发率 |
| $\tau_{\mathrm{gap}}$ | 二簇和多簇结构的区分依赖环境 | 绝对值 $[0.01,0.3]$；或比率 $(\lambda_3-\lambda_2)/(\lambda_2+\varepsilon)\in[0.5,5]$ | 二分识别 F1 |
| $\tau_{\mathrm{ncut}}$ | 开放空间和窄通道的正常 Ncut 差别很大 | $\{0.05,0.1,0.2,0.3,0.4,0.5\}$ | 误切率、漏切率 |
| $\rho_\lambda$ | 相对下降阈值依赖更新频率和图增长速度 | $\{0.3,0.4,0.5,0.6,0.7,0.8\}$ | 触发稳定性、延迟 |
| $M_{\mathrm{trigger}}$ | 太小容易瞬时误触发，太大错过及时锁区 | $\{1,2,3,4,5\}$ 个谱更新周期 | BFMs、触发延迟 |
| `partition_method` | k-means 快，但不直接最小化 Ncut；sweep cut 更稳 | `kmeans_1d`、`median`、`sweep_ncut` | Ncut、运行时间、正确率 |

### C. 动态稳定参数

| 待调参数 | 为什么不能固定 | 建议搜索空间 | 核心指标 |
|---|---|---|---|
| $\delta_f$ | 小则标签抖动，大则新结构响应慢 | Fiedler 范围的 $\{0,2\%,5\%,10\%,15\%,20\%\}$ | 节点标签切换率、响应时间 |
| $\tau_{\mathrm{match}}$ | 太低会错误继承旧 ID，太高会频繁新建区域 | $\{0.3,0.4,0.5,0.6,0.7,0.8\}$ | ID 连续性、误合并/误分裂 |
| $K_{\mathrm{region}}$ | 前沿可能短暂消失；确认太久增加等待 | $\{1,2,3,5,8\}$ 个 frontier update 周期 | 早退率、额外等待时间 |
| $K_{\mathrm{finish}}$ | 太小误判完成，太大增加尾部时间 | $\{2,3,5,8,10\}$ | 错误完成率、尾部耗时 |
| $\varepsilon_I$ | 受传感器分辨率、射程和 EROI 大小影响 | $\{0,5,10,20,50,100\}$ 体素，或 $\{0,0.01,0.02,0.05,0.1\}$ m³ | 完成率、残余未知体积 |
| $\varepsilon_G$ | 图小变化可能只是噪声，大变化代表新结构 | $\{0,1,2,5,10\}$ 个节点/边变化 | 完成误判、等待时间 |
| `stale_region_timeout` | 区域锁可能因前沿更新故障死锁 | $\{1,2,3,5,8,10\}$ s | 死锁次数、非必要解锁 |
| `unreachable_confirm_cycles` | 单次 A* 失败不应立即删除区域 | $\{1,2,3,5\}$ | 恢复成功率、漏探索率 |

### D. 路线与区域锁参数

| 待调参数 | 为什么不能固定 | 建议搜索空间 | 核心指标 |
|---|---|---|---|
| $\lambda_{\mathrm{jump}}$ | 权衡严格谱序与实际转移距离 | $\{0,0.25,0.5,1,2\}$ | 路径长度、BFMs、转向次数 |
| $\lambda_{\mathrm{terminal}}$ | 过强会牺牲当前可达性，过弱丢失最远端最后优势 | $\{0,0.1,0.25,0.5,1\}$ | 路径长度、最终回程代价 |
| `region_selection_score` 权重 | 下一区域可按距离、前沿数、孤立度选择，权衡不同 | 网格：距离 $[0.25,2]$，前沿数 $[0,1]$，孤立度 $[0,2]$ | 总时间、漏探索、回访距离 |
| `lock_enabled` | 区域锁是 BFMs 改善来源，也可能阻碍更优目标 | on/off | BFMs、探索时间 |
| `allow_emergency_unlock` | 完全禁止解锁可能在动态图中死锁 | on/off；超时 $[1,10]$ s | 死锁、恢复率、安全事件 |

### E. 局部谱视点评分参数

| 待调参数 | 为什么不能固定 | 建议搜索空间 | 核心指标 |
|---|---|---|---|
| $\lambda_s$ | 太小不起作用，太大会压过 EDEN 曲率和距离评分 | $\{0,0.01,0.025,0.05,0.1,0.2,0.4\}$ | 局部目标变化率、速度、BFMs |
| $\lambda_{\mathrm{cross}}$ | 对虚拟候选和恢复目标有效，过大可能阻止必要跨区 | $\{0,0.05,0.1,0.25,0.5,1\}$ | 跨区次数、恢复成功率 |
| `boundary_confidence_mode` | $|f|$、$|f-\theta|$、softmax 置信度表现不同 | 三种离散配置 | 目标稳定度、误惩罚率 |
| `spectral_epoch_max_age` | 允许使用过期谱值会降低实时开销，但可能错误 | $\{0,1,2,3\}$ 个谱周期 | 规划耗时、错误目标率 |

### F. 特征求解与实时性参数

| 待调参数 | 为什么不能固定 | 建议搜索空间 | 核心指标 |
|---|---|---|---|
| `solver_type` | 小图稠密求解常比手写 LOBPCG 更快、更稳 | `SelfAdjointEigenSolver`、LOBPCG | 平均/P95 时间、残差 |
| `n_dense_switch` | 取决于 CPU、矩阵稀疏度和 Eigen 编译优化 | $\{20,30,40,60,80,120\}$ | P95 求解时间 |
| `lobpcg_hot_iter` | 太少不收敛，太多浪费计算 | $\{2,3,5,8,10,15\}$ | 残差、时间、fallback |
| `lobpcg_cold_iter` | 初始图更难收敛 | $\{10,20,30,50,80\}$ | 冷启动时间、成功率 |
| `lobpcg_tol` | 过严没有规划收益，过松标签不稳定 | $\{10^{-2},10^{-3},10^{-4},10^{-5},10^{-6}\}$ | 残差、标签稳定、耗时 |
| `spectral_update_period` | 每帧算太贵，太慢则区域状态滞后 | $\{0.1,0.2,0.4,0.8,1.0,2.0\}$ s | CPU、BFMs、响应延迟 |
| `dirty_change_threshold` | 图只变一条边是否重算取决于稳定需求 | $\{1,2,5,10\}$ 个变化 | 重算次数、延迟 |
| `spectral_time_budget_ms` | 超预算必须 fallback，阈值依硬件而定 | $\{0.5,1,1.5,2,3,5,10\}$ ms | 实时违约率、路线质量 |
| `max_fallback_rate` | 过高说明谱模块不可用 | $\{1\%,5\%,10\%,20\%\}$ 作为验收阈值 | 系统可靠性 |

---

## 3.4 推荐调参顺序

不要一次性联合搜索全部参数。采用六阶段流程：

### 阶段 1：验证基线与日志

固定原 EDEN，确保以下指标能稳定统计：

```text
T_exp, L_path, v_avg, planning_time,
N_cross, N_premature, N_BFM,
frontier_count, active_region_id
```

### 阶段 2：只调谱图和求解器

关闭区域锁和局部谱惩罚，搜索：

```text
graph_mode
gamma_l
k_nn / r_spec
solver_type
n_dense_switch
tol
```

目标：特征值稳定、P95 计算时间合格。

### 阶段 3：调分区正确性

在有真值地图的仿真中标注房间/走廊区域，搜索：

```text
clearance_ref
gamma_c
beta_b
tau_conn
tau_gap
tau_ncut
rho_lambda
```

目标：分区 precision/recall、ARI/NMI、瓶颈边识别率。

### 阶段 4：调动态稳定

搜索：

```text
alpha
M_trigger
delta_f
tau_match
```

目标：减少标签抖动，同时限制新结构响应延迟。

### 阶段 5：调区域执行

搜索：

```text
K_region
stale_timeout
lambda_jump
lambda_terminal
```

目标：最小化 $N_{\mathrm{premature}}$ 和回访距离，同时不显著增加总探索时间。

### 阶段 6：最后调局部谱惩罚

只搜索：

```text
lambda_s
lambda_cross
```

目标：作为 tie-breaker 改善目标稳定性，且平均速度下降不超过预设容忍度，例如 3%。

---

## 3.5 推荐优化目标

单一指标会导致偏置。建议使用加权综合目标：

$$
J
=
w_T\frac{T_{\mathrm{exp}}}{T_{\mathrm{EDEN}}}
+
w_L\frac{L_{\mathrm{path}}}{L_{\mathrm{EDEN}}}
+
w_B\frac{N_{\mathrm{BFM}}}{N_{\mathrm{BFM,EDEN}}+1}
+
w_P\frac{T_{\mathrm{plan,P95}}}{T_{\mathrm{budget}}}
+
w_F(1-\operatorname{CompletionRate})
$$

推荐初始权重：

```text
w_T = 0.35
w_L = 0.20
w_B = 0.25
w_P = 0.10
w_F = 0.10
```

正式论文中仍需分别报告原始指标，不能只报告综合分数。

---

## 3.6 推荐验收门槛

完整 Spectral-EDEN 至少满足：

```text
1. 完成率不低于原 EDEN。
2. N_premature 和 N_BFM 显著下降。
3. 总探索时间或路径长度至少一项显著改善，另一项不得严重退化。
4. 平均全局规划时间增加可控，P95 不超过实时预算。
5. 谱求解失败时 100% 能回退到 EOHDT。
6. 动态出现新通道时，区域允许重新合并或重分配，不发生永久错误锁定。
7. 开放空间中不会频繁产生无意义二分。
8. 全图完成与普通规划失败使用不同状态码。
```

建议论文主张采用可验证措辞：

```text
“显著减少 BFMs 和区域提前切换”
```

不要在没有跨场景统计检验前使用：

```text
“彻底消除 BFMs”
```

---

## 3.7 最小可行实现（MVP）与最终论文版本

### MVP：先跑通代码

```text
- 复用 tarhs + dist_mat
- SelfAdjointEigenSolver
- Fiedler 正/逆序二选一
- 符号对齐
- EOHDT fallback
- 独立 PlanStatus
- 完整日志
```

### 论文版本

```text
- 历史支撑稀疏图
- 净空/瓶颈感知边权
- lambda2 + eigengap + Ncut 触发
- 新旧区域匹配和标签滞回
- active_region 锁定与 missed_regions 回溯
- 阈值中心化的局部谱惩罚
- 热启动与按需更新
- 完整模块级和参数级消融
```

原始修改文档提出的五阶段谱图方案、LOBPCG 热启动、$\lambda_2$ 监控和局部谱一致性项，是本说明书的基础输入；本说明书进一步补齐了动态分区稳定、真实结束状态、瓶颈感知边权、特征值间隔、区域执行锁和代码状态码。fileciteturn30file0
