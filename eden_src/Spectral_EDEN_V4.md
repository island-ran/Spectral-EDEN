# Spectral-EDEN V4：支持原始EDEN/V4切换的预算约束统一谱二分探索框架

> **文件名**：`Spectral_EDEN_V4.md`  
> **代码基线**：`island-ran/Spectral-EDEN`，`main` 分支，提交 `b342d901313d3810a240e09e6866cb21224e435f`  
> **主要目标**：同时解决当前版本中无人机目标来回振荡和规划计算量过大的问题，并在同一代码仓库中保留可独立运行、可公平对照的原始EDEN模式与Spectral-EDEN V4模式。  
> **融合原则**：全局规划器在节点启动时只选择`EDEN_BASELINE`或`SPECTRAL_V4`之一；进入V4后，V4.1、V4.2、V4.3不作为三套并行算法运行，而是被裁剪并融合为一个统一框架，每次图分析任务只允许选择一种计算后端，并统一输出同一种谱二分提案。

---

# 1. 最终结论

整个工程应首先被定义为一个**双模式全局规划系统**：

\[
\boxed{
\text{GlobalPlanner}
=
\begin{cases}
\text{EDEN\_BASELINE}, & \text{运行原始EOHDT}\\
\text{SPECTRAL\_V4}, & \text{运行统一V4}
\end{cases}
}
\]

两种模式共享地图、DTG、EROI、局部视点、ASEO轨迹优化和完成判断，但使用不同的全局宏观目标生成器：

```text
EDEN_BASELINE
    → 原始EOHDT完整anchor访问路线
    → 不启动V4 Worker
    → 不维护区域树
    → 不运行Fiedler、Nyström或APPR

SPECTRAL_V4
    → 统一V4谱二分框架
    → 启动固定Worker
    → 只选择一个已承诺宏观目标
    → 不构造完整EOHDT基线路线
```

Spectral-EDEN V4本身定义为：

\[
\boxed{
\text{Persistent Topological Skeleton}
+
\text{Budget-Bounded Binary Cut Backend}
+
\text{Deterministic Region Manager}
+
\text{Committed Single-Target Planner}
}
\]

三个原方案在V4中的角色如下：

| 原方案 | 在统一V4中的角色 | 是否独立运行 |
|---|---|---:|
| V4.1 增量谱骨架 | **主图表示和主求解模式**：维护持久化拓扑骨架，在小规模骨架上求Fiedler二分 | 否 |
| V4.3 Nyström | **固定规模的全局近似后端**：骨架超过精确求解预算时，对确定性地标矩阵求近似Fiedler向量 | 否 |
| V4.2 Local APPR | **局部低成本后端**：全局谱状态过期、残差过大或预算不足时，只提取UAV所在局部区域 | 否 |

任意一次V4 Worker任务只能执行：

```text
INCREMENTAL_FIEDLER
或 LANDMARK_NYSTROM
或 LOCAL_APPR
```

三者统一输出：

```cpp
BinaryRegionProposal
```

由唯一的区域管理器更新区域树和区域商图。三种后端不能分别维护三套区域、三套目标排序和三套状态机。

原始EDEN不会被删除，而是被隔离为独立的`EDEN_BASELINE`模块。V4不再生成覆盖全部frontier的完整访问序列，每个规划周期只做一件事：

```text
保持当前目标
或选择一个新frontier
或选择一个相邻区域入口
```

EDEN模式和V4模式通过Launch/YAML在节点启动时二选一，不在同一规划周期内同时运行和相互比较。

---

# 2. 原始EDEN与Spectral V4运行切换架构

## 2.1 切换原则

同一个ROS包、同一个节点和同一套公共地图接口中保留两种互斥模式：

```text
EDEN_BASELINE
    原始EDEN/EOHDT基线
    不启动V4 Worker
    不读取RegionState
    不运行任何谱后端

SPECTRAL_V4
    统一V4算法
    启动固定Worker
    Fiedler/Nyström/APPR三选一
    不计算完整EOHDT tour
```

这里的“保留原始EDEN”不表示V4运行时仍要计算一条EOHDT基线路线。两种算法只在启动时二选一，以保证：

- EDEN实验是纯净基线；
- V4实验不承担EDEN完整路线的额外计算；
- 两种模式使用同一份地图、参数框架和统计接口；
- 同一个仓库即可完成公平对照和消融实验。

## 2.2 明确的模式枚举

在`mr_dtg_plus_structures.h`新增：

```cpp
enum class GlobalPlannerMode {
    EDEN_BASELINE = 0,
    SPECTRAL_V4 = 1
};
```

不要继续让：

```yaml
MR_DTG/SpectralV41/enabled: true/false
```

承担全局模式切换。`enabled=false`可能同时表示“运行EDEN”“V4临时不可用”“谱求解失败”或“关闭某项V4功能”，语义不明确。

统一使用：

```yaml
MR_DTG/GlobalPlanner/mode: "eden"
```

或：

```yaml
MR_DTG/GlobalPlanner/mode: "spectral_v4"
```

非法字符串必须在初始化阶段报错并终止节点，不能静默选择某个默认算法。

## 2.3 顶层分发器

`PlanGlobalRoute()`只负责公共检查和模式分发：

```cpp
GlobalPlanStatus MultiDtgPlus::PlanGlobalRoute(
    const Eigen::Vector3d& ps,
    std::vector<h_ptr>& route_h,
    std::vector<Eigen::Vector3d>& path2fh,
    double& d1) {

    route_h.clear();
    path2fh.clear();
    d1 = 0.0;

    if (!ps.allFinite()) {
        return GlobalPlanStatus::INTERNAL_ERROR;
    }

    switch (global_planner_mode_) {
    case GlobalPlannerMode::EDEN_BASELINE:
        return PlanGlobalRouteEden(ps, route_h, path2fh, d1);

    case GlobalPlannerMode::SPECTRAL_V4:
        return PlanGlobalRouteV4(ps, route_h, path2fh, d1);
    }

    return GlobalPlanStatus::INTERNAL_ERROR;
}
```

分发器中禁止：

```text
先计算EDEN完整路线
再计算V4路线
最后比较两条路线
```

否则仍会保留当前代码最严重的双重计算问题。

## 2.4 原始EDEN执行路径

新增：

```text
src/mr_dtg_plus_eden_baseline.cpp
```

其中保留原始EDEN/EOHDT逻辑：

```cpp
PlanGlobalRouteEden()
BuildEdenActiveDistanceMatrix()
BuildOriginalEohdtRoute()
Prim()
```

执行流程：

```text
收集全部可达活动anchor
→ 构造原始距离矩阵
→ Prim/MST和原始EOHDT遍历
→ 输出完整route_h
→ BuildPathToFirst
```

EDEN模式不得访问：

```text
SpectralWorker
RegionTreeManager
RegionStateSnapshot
Incremental Fiedler
Nyström
APPR
V4目标承诺的区域字段
```

这样才能保证原始EDEN结果不受V4状态污染。

## 2.5 V4执行路径

新增：

```text
src/mr_dtg_plus_v4_global.cpp
```

其中实现：

```cpp
PlanGlobalRouteV4()
```

执行流程：

```text
非阻塞读取RegionState
→ 检查CommittedTarget
→ 构建固定小候选集
→ 计算候选精确首段路径
→ 选择一个anchor或区域入口
→ 输出单一主目标
```

V4模式不得调用：

```text
BuildEdenActiveDistanceMatrix()
BuildOriginalEohdtRoute()
Prim()完整tour
EOHDT/V4双路线比较
```

为兼容现有`FindFastExpTarget()`接口，V4可暂时输出：

```cpp
route_h = {selected_anchor};
```

若局部连续轨迹确实需要第二目标，只能加入同一区域内的一个局部次目标，不能恢复完整全局tour。

## 2.6 Worker生命周期

Worker只在V4模式创建：

```cpp
if (global_planner_mode_ == GlobalPlannerMode::SPECTRAL_V4) {
    spectral_worker_ =
        std::make_unique<SpectralWorker>(spectral_v4_config_);
    spectral_worker_->Start();
}
```

EDEN模式应满足：

```cpp
spectral_worker_ == nullptr
```

析构阶段：

```cpp
MultiDtgPlus::~MultiDtgPlus() {
    if (spectral_worker_) {
        spectral_worker_->Stop();
    }
}
```

`Stop()`可以在ROS节点关闭时执行`join()`；正常规划周期中主线程禁止等待Worker。

## 2.7 公共模块与隔离边界

两种模式共享：

```text
地图和EROI更新
DTG节点与真实边维护
frontier生成
UAV到DTG的可达性
局部viewpoint检查
FindFastExpTarget
目标细化
ASEO轨迹优化
轨迹安全检查
完成判断
统计和日志
```

两者只在宏观目标产生方式上不同：

```text
EDEN_BASELINE：完整EOHDT anchor顺序
SPECTRAL_V4：一个已承诺anchor或相邻区域入口
```

## 2.8 推荐切换方式

第一版只实现**节点启动时切换**，不实现运行中的热切换。

Launch文件：

```xml
<arg name="planner_mode" default="spectral_v4"/>

<param
    name="MR_DTG/GlobalPlanner/mode"
    value="$(arg planner_mode)"/>
```

运行原始EDEN：

```bash
roslaunch eden single_exp_demo_city_lite_low.launch \
  planner_mode:=eden
```

运行V4：

```bash
roslaunch eden single_exp_demo_city_lite_low.launch \
  planner_mode:=spectral_v4
```

启动时切换适合科研对比，因为每次实验的算法状态、线程和缓存均从干净状态开始。

## 2.9 为什么暂不实现热切换

从V4热切换到EDEN需要：

```text
停止Worker
清空未完成任务
清空区域树和商图
解除V4目标承诺中的区域关联
保留或安全结束当前轨迹
重新构造EDEN路线
```

从EDEN热切换到V4需要：

```text
启动Worker
提交完整初始GraphDelta
等待首个合法RegionState
等待期间继续当前安全目标
首个V4结果到达后也不能强制立即换目标
```

处理不完整会重新引入目标振荡，因此第一版只做启动时二选一。

## 2.10 V4内部降级不等于切换到EDEN

V4 Worker没有新结果、结果过期或某个谱后端失败时，应按以下顺序处理：

```text
继续当前安全目标
→ 当前目标失效时选择局部可达frontier
→ 无局部目标时执行安全保持并重新检查frontier
```

V4模式不能自动运行完整EOHDT tour，否则重新形成：

```text
V4图计算
+
EDEN全对距离矩阵
+
完整EOHDT
```

只有启动参数明确为`eden`时，才执行完整原始EDEN。

## 2.11 模式诊断

每个规划周期至少记录：

```text
planner_mode
v4_backend
worker_graph_version
region_state_version
committed_target_id
target_switch_reason
planning_ms
```

EDEN示例：

```text
[GlobalPlanner]
mode=EDEN_BASELINE
route_nodes=31
plan_ms=49.2
```

V4示例：

```text
[GlobalPlanner]
mode=SPECTRAL_V4
backend=LANDMARK_NYSTROM
region_version=380
target=116
decision=KEEP_COMMITTED
plan_ms=18.4
```

---

# 3. 对当前仓库代码的审查结论

## 2.1 当前仓库存在两套不一致源码

仓库同时包含：

```text
catkin_ws/src/Mapping_mr_dtg_plus
eden_src/EDEN/Mapping/mr_dtg_plus
```

当前实验脚本加载：

```text
/home/island/EDEN/catkin_ws/devel/setup.bash
```

因此真正运行的是Catkin工作空间构建出来的库。但是当前GitHub仓库中：

- `catkin_ws/src`仍保留大量V2/V3完整路线、soft mode、lock debt和旧谱快照逻辑；
- `eden_src/EDEN`中出现了V4.1、所谓V4.2和V4.3的部分代码；
- 两套目录的内容已经发生明显分叉。

这会导致以下严重问题：

1. 修改了`eden_src`，实际运行的却可能是`catkin_ws/src`；
2. 同名文件在两个目录中的逻辑不同，无法确定实验对应哪一版代码；
3. 编译产物可能没有反映GitHub中最新修改；
4. 后续任何性能结论都可能建立在错误版本上。

### V4实施前的强制动作

只保留一个源码真源。建议以：

```text
catkin_ws/src/Mapping_mr_dtg_plus
catkin_ws/src/Exploration_eden
```

作为唯一编译源码，将`eden_src/EDEN`删除、归档或改成只读备份，禁止两套源码同时维护。

---

## 2.2 当前V4参数已加入，但主流程仍是V2/V3

当前初始化代码加载了：

```yaml
MR_DTG/SpectralV41/enabled
MR_DTG/SpectralV41/skeleton_max_nodes
MR_DTG/SpectralV41/local_frontier_top_k
MR_DTG/SpectralV41/target_switch_margin
MR_DTG/SpectralV41/min_target_commit_sec
```

但当前`PlanGlobalRoute()`在收集活动区域后，仍直接进入：

```cpp
// Original V2/V3 pipeline
ConsumeSpectralResult(...);
ReassignFrontierOwners();
UpdateRegionExecutionState(...);
DetectAndHandleRegionStall(...);
...
RunEohdtFallback(...);
...
BuildRegionAwareRoute(...);
EvaluateRegionRoute(...);
UpdateLockDebt(...);
```

因此当前V4参数并没有真正替换旧主流程。

这意味着：

> 当前代码不是“V4主架构”，而是在V2/V3主架构旁边增加了一些V4辅助函数。

---

## 2.3 当前所谓V4.2并不是真正的APPR

当前`ComputeLocalAPPRFallback()`实际执行的是：

```text
收集全部活动anchor
→ 按robot-root距离排序
→ 保留前K个
→ 按log(1+hf_edges数量)/距离选择目标
```

代码没有维护Personalized PageRank向量，没有residual push，也没有conductance sweep。

所以它本质上是：

```text
Top-K nearest gain/distance heuristic
```

不能称为Local APPR，也不能获得局部谱聚类的计算局部性和边界质量。

V4必须将它替换成真正的有界APPR push算法。

---

## 2.4 当前所谓V4.3并不是真正的Nyström

当前`SelectSkeletonLandmarks()`执行：

```text
用三维欧氏坐标做farthest-point sampling
→ 只保留landmark active_hnodes
→ 再调用原BuildSparseSupportSpectralGraph()
```

它没有构造：

\[
W_{mm},\quad C_{nm}
\]

没有在地标子矩阵上求谱，也没有Nyström扩展或标签传播。

因此当前实现只是：

```text
landmark subsampling
```

而不是Nyström谱近似。

更严重的是，它使用三维欧氏距离选择地标。在墙体、U型建筑、停车场坡道等环境中，欧氏距离相近的节点可能拓扑距离很远，容易破坏瓶颈结构。

V4必须采用持久化、确定性的**拓扑地标**，并实现真实Nyström归一化与扩展。

---

## 2.5 当前“异步”只覆盖了特征求解

当前`SubmitSpectralJobAsync()`的执行顺序是：

```text
主线程：BuildSpectralGraphSnapshot()
主线程：kNN/MST和路径materialization
主线程：corridor compression
主线程：节点数检查
然后才 std::async(router.Solve)
```

所以：

```text
Eigen solve异步
≠
整个谱流水线异步
```

主线程仍然承担最不稳定的图构建开销，因此P99尖峰仍会传递到UAV规划线程。

仓库中已经存在一个纯值类型的`spectral_snapshot_builder.cpp`，但当前`CMakeLists.txt`没有把它加入`cs_add_library()`，旧的成员函数构图仍在运行。这部分属于未接入的死代码。

---

## 2.6 当前区域状态会诱发目标振荡

当前区域逻辑存在多处振荡源。

### 振荡源A：单次Ncut失败立即使partition失效

当前结果消费逻辑近似为：

```cpp
if (has_valid_cut && ncut <= threshold)
    partition_valid_ = true;
else
    partition_valid_ = false;
```

一次求解波动就可能让全局区域状态从有效变为无效，下一次又恢复。

### 振荡源B：每次更新重建missed region队列

当前代码会按照`spectral_center`重新排序区域，并重建`missed_regions_`。当Fiedler符号、节点集合或区域匹配发生小变化时，区域访问顺序也会变化。

### 振荡源C：旧V2/V3模式机仍参与路线决策

当前主流程仍使用：

```text
OBSERVING
ACTIVE_SOFT
RECOVERY
lock debt
route feedback
recovery_until
```

这会在不同运行中因为线程完成时刻、frontier版本变化和规划周期边界不同而产生不同的模式切换时序。

### 振荡源D：每周期重新生成完整baseline和candidate路线

每个规划周期都重新构建：

```text
EOHDT完整路线
+
区域感知完整路线
```

即便UAV正稳定飞向当前目标，新的完整路线也可能把另一个frontier放到首位。

### 振荡源E：目标承诺参数没有真正控制主流程

虽然配置中存在：

```yaml
target_switch_margin
min_target_commit_sec
```

但当前`PlanGlobalRoute()`仍由旧完整路线首节点决定目标，没有形成统一的`CommittedTargetState`。

---

## 2.7 当前计算量不是单一的O(n³)

当前在线计算叠加了：

1. 活动anchor收集；
2. `BuildActiveDistanceMatrix()`全对距离矩阵；
3. 缺失H-H距离维护；
4. `RunEohdtFallback()`的Prim和完整树遍历；
5. kNN支撑对构造；
6. 谱层第二棵MST；
7. 支撑路径materialization；
8. corridor compression；
9. 可选edge betweenness；
10. Laplacian和特征求解；
11. Ncut sweep；
12. `BuildRegionAwareRoute()`完整候选路线；
13. baseline/candidate完整路线代价比较；
14. frontier owner、区域状态、lock debt和recovery维护。

因此必须删除重复结构，而不是只把Eigen求解器换成更快的求解器。

---

# 4. V4统一算法定义

## 3.1 V4是有界递归谱二分

V4底层只输出二分：

\[
R\rightarrow R_0\cup R_1
\]

通过有限递归形成多个宏观区域：

```text
完整区域
├── Region A
└── Region B
    ├── Region B1
    └── Region B2
```

但每次Worker更新最多接受一个父区域的一次二分：

\[
K_{t+1}\le K_t+1
\]

不允许在一个周期内递归切完整棵树，也不使用一次性K-way聚类。

---

## 3.2 统一数据流

```text
DTG更新
  ↓
GraphDelta
  ↓
Persistent Contracted Skeleton
  ↓
Budget Mode Selector
  ├── Incremental Fiedler
  ├── Landmark Nyström
  └── Local APPR
  ↓
BinaryRegionProposal
  ↓
Deterministic Region Tree Manager
  ↓
Region Quotient Graph
  ↓
Committed Single-Target Planner
  ↓
EDEN local viewpoint + ASEO trajectory
```

---

# 5. 统一持久化拓扑骨架

## 4.1 骨架节点

只保留下列节点：

1. DTG分叉节点，`degree != 2`；
2. 活跃frontier所属anchor；
3. 已识别瓶颈边的两端；
4. UAV当前投影到DTG的节点；
5. 区域入口节点；
6. 多层环境中的楼梯、坡道和电梯连接节点；
7. 为保持环结构而选择的确定性break node。

普通度2走廊节点被压缩到chain edge中。

## 4.2 骨架边

```cpp
struct SkeletonEdge {
    uint32_t from;
    uint32_t to;
    double length;
    double min_clearance;
    uint32_t raw_edge_count;
    uint64_t version;
};
```

权重只使用路径长度和净空：

\[
w_{ij}=
\exp\left(-\frac{\ell_{ij}}{\sigma_\ell}\right)
\left(
\frac{c_{ij}+\epsilon}{c_{ref}+\epsilon}
\right)^p
\]

删除在线edge betweenness。边介数与谱切割表达的瓶颈信息高度重叠，同时其全源最短路计算会显著增加开销。

## 4.3 增量更新

主线程不再每周期完整扫描和重新压缩。

```cpp
struct GraphDelta {
    uint64_t graph_version;
    uint64_t frontier_version;
    std::vector<NodeInsert> node_inserts;
    std::vector<NodeErase> node_erases;
    std::vector<EdgeUpsert> edge_upserts;
    std::vector<EdgeErase> edge_erases;
    std::vector<FrontierAnchorUpdate> frontier_updates;
    uint32_t robot_h_id;
};
```

Worker维护自己的骨架副本：

```text
新增度2节点        → 扩展对应chain
新增分叉边          → 拆分局部chain
边失效              → 只重建受影响连通分量
frontier状态变化    → 更新mandatory anchor标志
```

禁止每次调用完整`CompressDegreeTwoChains()`。

---

# 6. 三种方法的真实融合方式

## 5.1 模式A：Incremental Fiedler

### 适用条件

```text
骨架节点数 <= exact_max_nodes
图变化量较小
上次残差合格
Worker版本落后不超过1
```

建议：

```yaml
exact_max_nodes: 48
```

### 算法

1. 在持久化骨架上构造稀疏normalized Laplacian；
2. 只求`u2`和`u3`；
3. 使用上一稳定结果做warm start；
4. 使用块LOBPCG、Lanczos或现有块Rayleigh–Ritz；
5. 用Fiedler sweep得到候选二分；
6. 计算Ncut、balance和残差；
7. 输出`BinaryRegionProposal`。

复杂度近似：

\[
O(I|E_s|),\qquad |V_s|\le48
\]

不再生成稠密完整特征向量矩阵。

---

## 5.2 模式B：Landmark Nyström

### 适用条件

```text
exact_max_nodes < 骨架节点数 <= nystrom_max_support
需要全局区域结构
Worker仍有预算
```

建议：

```yaml
landmark_count: 32
landmark_max_count: 48
nystrom_max_support: 512
```

### 地标选择

必须确定性并持久化：

1. 所有区域入口和真实分叉节点优先；
2. 活跃frontier anchor优先；
3. 低净空连接端点优先；
4. 不足部分用**拓扑距离**farthest-point sampling补足；
5. 平局按H-node ID处理；
6. 地标不在每周期重新选择，只在失效或重要性替换条件满足时更新。

禁止使用三维欧氏距离替代拓扑距离。

### Nyström计算

设地标数为`m`，骨架节点数为`n`：

\[
W\in\mathbb R^{m\times m},\qquad
C\in\mathbb R^{n\times m}
\]

每个节点只连接最近的`q`个地标：

```yaml
nearest_landmarks_per_node: 4
```

只对`m×m`归一化地标矩阵求`u2/u3`，再进行Nyström扩展：

\[
\tilde U=C U_m\Lambda_m^{-1}
\]

最终仍按近似Fiedler向量做二分，不执行随机K-means。

复杂度：

\[
O(m^3)+O(qn),\qquad m\le48
\]

---

## 5.3 模式C：Local APPR

### 适用条件

```text
骨架过大
全局结果过期
Nyström残差失败
Worker落后超过最大版本
或只需要判断UAV所在局部区域
```

### 真正的APPR

以UAV当前H-node为seed，维护：

```text
p(v)       PageRank近似值
r(v)       residual
```

当：

\[
\frac{r(v)}{d(v)}>\varepsilon_{ppr}
\]

时执行push。

```cpp
while (!queue.empty() && pushes < max_pushes) {
    v = queue.pop();
    if (r[v] / degree[v] <= epsilon) continue;

    p[v] += alpha * r[v];
    const double remain = (1.0 - alpha) * r[v];
    r[v] = 0.5 * remain;

    for (edge : neighbors(v)) {
        r[edge.to] += 0.5 * remain * edge.probability;
        if (r[edge.to] / degree[edge.to] > epsilon)
            queue.push(edge.to);
    }
    ++pushes;
}
```

然后按：

\[
\frac{p(v)}{d(v)}
\]

排序并执行conductance sweep，得到：

```text
UAV所在局部区域
vs
外部区域
```

### 硬预算

```yaml
ppr_max_pushes: 20000
ppr_max_support_nodes: 512
```

达到上限时返回当前最佳局部cut并标记`truncated`，主线程不得等待。

复杂度由push操作数上限直接约束。

---

# 7. 统一模式选择器

## 6.1 状态不是规划状态机

V4只保留计算后端状态：

```cpp
enum class CutBackend {
    INCREMENTAL_FIEDLER,
    LANDMARK_NYSTROM,
    LOCAL_APPR,
    UNAVAILABLE
};
```

删除：

```text
OBSERVING
ACTIVE_SOFT
RECOVERY
random cooldown
lock debt
```

## 6.2 确定性选择规则

```cpp
CutBackend SelectBackend(const WorkerGraphState& g,
                         const SolverHistory& h,
                         const BudgetConfig& cfg) {
    if (g.skeleton_nodes <= cfg.exact_max_nodes &&
        h.exact_failure_streak == 0) {
        return INCREMENTAL_FIEDLER;
    }

    if (g.skeleton_nodes <= cfg.nystrom_max_support &&
        h.nystrom_failure_streak < 2) {
        return LANDMARK_NYSTROM;
    }

    if (g.local_seed_valid) {
        return LOCAL_APPR;
    }

    return UNAVAILABLE;
}
```

不使用wall-time cooldown恢复。恢复条件只能是：

```text
新的合法结果到达
图规模重新进入阈值
失败原因被新的图版本消除
```

---

# 8. 统一二分输出

```cpp
struct BinaryRegionProposal {
    uint64_t graph_version;
    uint64_t frontier_version;
    CutBackend backend;

    int parent_region_id;
    std::vector<uint32_t> side_a;
    std::vector<uint32_t> side_b;

    double ncut;
    double balance;
    double residual;
    double support_coverage;
    bool truncated;
    bool valid;

    uint64_t deterministic_signature;
};
```

三种后端不得直接修改：

```text
active_region_id
frontier owner
当前飞行目标
完整路线
```

它们只生成proposal。

---

# 9. 确定性区域管理器

## 8.1 区域树

```cpp
struct RegionNode {
    int id;
    int parent_id;
    int child_a;
    int child_b;
    std::unordered_set<uint32_t> h_ids;
    uint64_t accepted_graph_version;
    double cut_quality;
    bool leaf;
};
```

规划只使用叶子区域。

## 8.2 每次只允许一个切割

从叶子区域中选择一个候选父区域：

\[
R^*=\arg\max_R
\left[
G(R)\cdot B(R)
\right]
\]

其中：

- `G(R)`：区域内frontier总增益；
- `B(R)`：瓶颈候选强度。

一次Worker结果最多切一个区域，防止区域体系整体洗牌。

## 8.3 接受条件

```text
两个子区域均非空
balance >= min_balance
Ncut <= max_ncut
每侧至少存在一个可行动frontier或有效入口
proposal版本在允许范围内
与上一候选的Jaccard一致率达到阈值
```

建议：

```yaml
min_balance: 0.20
max_ncut: 0.30
proposal_confirm_versions: 2
proposal_jaccard_threshold: 0.80
```

这里的“两次确认”按连续图版本计数，不依赖随机线程完成时刻或wall-time cooldown。

## 8.4 不因单次失败删除旧区域

若新proposal无效：

```text
保留最后一个稳定RegionState
```

只有下列情况才使其过期：

- 关键入口边被删除；
- 区域节点超过一定比例失效；
- 版本落后超过`max_region_version_lag`；
- 两个区域被新真实通路明确合并。

建议：

```yaml
max_region_version_lag: 2
region_invalidation_ratio: 0.30
```

---

# 10. 区域商图

```cpp
struct QuotientRegion {
    int region_id;
    double total_gain;
    std::vector<uint32_t> frontier_ids;
    std::vector<uint32_t> entry_h_ids;
};

struct QuotientEdge {
    int from_region;
    int to_region;
    uint32_t entry_from;
    uint32_t entry_to;
    double traversal_time;
};
```

商图只表达宏观连接，不生成所有frontier访问顺序。

商图规模建议限制：

```yaml
max_leaf_regions: 6
max_recursive_depth: 3
```

---

# 11. 解决来回振荡：Committed Single-Target Planner

这是V4最重要的修改。

## 10.1 删除完整路线概念

当前代码每周期同时生成完整EOHDT路线和完整区域感知路线。V4删除这两条tour，只维护一个已承诺目标：

```cpp
struct CommittedTarget {
    bool valid;
    uint32_t frontier_id;
    uint8_t viewpoint_id;
    uint32_t anchor_h_id;
    int region_id;

    uint64_t selected_graph_version;
    double accepted_time;
    double exact_path_time;
    double selection_cost;
    double actual_gain_since_accept;
};
```

## 10.2 候选集合必须很小

每次最多考虑：

1. 当前已承诺目标；
2. 当前区域内TOP-K frontier；
3. 最多两个相邻区域入口；
4. APPR模式返回的一个局部出口候选。

建议：

```yaml
local_frontier_top_k: 8
max_adjacent_region_entries: 2
```

## 10.3 只为候选计算精确首段路径

对小候选集执行一次多目标Dijkstra或逐候选A*。

删除：

```text
全部anchor全对距离
完整MST
完整tour代价
root-star近似的全路线后悔
```

## 10.4 目标评分

\[
J(c)=
T_{exact}(c)
-
\beta\log(1+G(c))
+
\gamma R_{actual}(c)
\]

其中：

- `T_exact`：UAV到候选入口或frontier的精确预计时间；
- `G`：frontier或区域的有效信息增益；
- `R_actual`：该目标被真实执行后低收益的累计证据。

不使用跨区域soft-lock惩罚。

## 10.5 确定性切换条件

当前目标仍有效时，新目标只有满足：

\[
J_{new}+\Delta_{switch}<J_{current}
\]

才允许切换，其中：

\[
\Delta_{switch}=
\max(\Delta_{abs},\eta|J_{current}|)
\]

同时满足最小承诺时间：

\[
t-t_{accept}\ge T_{commit}
\]

建议：

```yaml
min_target_commit_sec: 1.5
target_switch_abs_margin_sec: 0.7
target_switch_relative_margin: 0.10
```

## 10.6 可以立即切换的硬事件

不受承诺时间限制：

- frontier失效；
- viewpoint失效；
- 路径被阻断；
- 当前轨迹碰撞检查失败；
- 当前目标实际增益连续两次低于阈值；
- 当前入口所属真实DTG边被删除。

## 10.7 区域重分割不能强制切换当前目标

若当前目标在新区域树中仍存在，则只更新其`region_id`，继续执行。

不能因为：

```text
Fiedler符号变化
region ID变化
后端从Fiedler切到Nyström
```

就更换飞行目标。

## 10.8 目标记录必须发生在轨迹成功之后

当前代码在`FindFastExpTarget()`末尾调用目标记录函数，此时轨迹优化和发布尚未完成。

V4改为：

```text
候选选择
→ 路径构造
→ viewpoint检查
→ ASEO优化
→ 轨迹有限数和安全检查
→ PublishTraj成功
→ CommitTargetAccepted()
```

规划失败不能增加“重复访问”计数。

---

# 12. Frontier生命周期裁剪

V4不再使用复杂的：

```text
NEW/ACTIVE/SUSPECT/DEFERRED/QUARANTINED/DEAD
+
区域owner重分配
+
重复选择计数
```

作为路线控制系统。

建议简化为：

```cpp
enum class FrontierStatus {
    ACTIVE,
    LOW_GAIN,
    UNREACHABLE,
    EXPLORED
};
```

规则：

- `expected_gain`只参与排序，不能使frontier失效；
- `LOW_GAIN`必须基于实际执行后的未知体积下降；
- `UNREACHABLE`必须来自多次真实路径失败；
- 无可行动目标但仍有raw reachable frontier时，必须fail-open重新启用最低成本候选。

---

# 13. 真正的C++异步架构

## 12.1 线程职责

### ROS规划主线程

只允许执行：

```text
DTG/Frontier更新
生成轻量GraphDelta
非阻塞读取RegionState
维护当前目标
构造小候选集
搜索到单一目标的路径
调用EDEN局部视点和ASEO
```

### 固定Worker线程

执行：

```text
维护稀疏图副本
增量chain contraction
模式选择
Fiedler/Nyström/APPR
Ncut或conductance sweep
二分proposal
区域树验证
区域商图构建
```

## 12.2 不再使用每任务std::async

使用一个长期存活线程：

```cpp
class SpectralWorker {
public:
    void Start();
    void Stop();
    void Submit(std::shared_ptr<const GraphDelta> delta);
    std::shared_ptr<const RegionStateSnapshot> TryConsume();

private:
    void Run();

    std::thread thread_;
    std::mutex mailbox_mutex_;
    std::condition_variable cv_;
    std::shared_ptr<const GraphDelta> latest_delta_;
    std::shared_ptr<const RegionStateSnapshot> latest_result_;
    bool running_ = false;
};
```

## 12.3 latest-only mailbox

动态探索中旧任务没有价值。新delta覆盖尚未处理的旧delta：

```cpp
void SpectralWorker::Submit(
    std::shared_ptr<const GraphDelta> delta) {
    {
        std::lock_guard<std::mutex> lock(mailbox_mutex_);
        latest_delta_ = std::move(delta);
    }
    cv_.notify_one();
}
```

Worker取走指针后立即释放锁，计算全部在锁外完成。

## 12.4 结果发布

在C++14中可以使用：

```cpp
std::atomic_store(&published_state_, new_state);
auto state = std::atomic_load(&published_state_);
```

其中`published_state_`为：

```cpp
std::shared_ptr<const RegionStateSnapshot>
```

也可以使用一个极短的结果mutex。不要为了追求“无锁”把可变`std::vector`直接放进无锁队列。

## 12.5 版本规则

每个结果包含：

```text
graph_version
frontier_version
source_backend
```

主线程接受条件：

```text
result.graph_version <= current.graph_version
current.graph_version - result.graph_version <= max_version_lag
结果涉及的入口边仍有效
当前目标不因结果到达而自动切换
```

## 12.6 Worker永远不能阻塞主线程

主线程禁止：

```cpp
future.get();
future.wait();
thread.join();  // 节点退出阶段除外
```

Worker慢时：

```text
继续使用最后稳定RegionState
或继续当前目标
或使用纯局部EDEN候选
```

UAV不能等待谱计算。

---

# 14. 完整V4算法流程

## 13.1 地图和图更新

```text
1. 更新block map和EROI
2. 更新DTG节点和真实H-H边
3. 记录GraphDelta
4. 将delta提交给Worker
```

## 13.2 Worker流程

```text
1. 合并收到的最新delta
2. 更新PersistentSkeleton
3. 选择一个最值得二分的叶子区域
4. 选择唯一后端：
   - 小骨架：Incremental Fiedler
   - 中大骨架：Landmark Nyström
   - 全局不可用：Local APPR
5. 生成BinaryRegionProposal
6. 执行确定性合法性检查
7. 更新区域树
8. 构建区域商图
9. 发布不可变RegionStateSnapshot
```

## 13.3 主线程目标规划

```text
1. 非阻塞读取最新RegionState
2. 检查当前CommittedTarget
3. 若当前目标仍满足承诺条件：保持现有轨迹
4. 否则建立小候选集
5. 计算到候选的精确路径时间
6. 应用确定性切换裕量
7. 选出一个目标
8. 调用FindFastExpTarget/目标细化
9. 调用ASEO
10. 验证轨迹并发布
11. 发布成功后提交目标承诺事件
```

## 13.4 完成判断

任务完成检查保持独立：

```text
已知地图：覆盖率达到阈值并稳定
未知地图：无raw reachable frontier、无恢复候选且连续无新增未知空间
```

完成判断不能依赖谱后端、区域模式或规划失败。

---

# 15. 顶层伪代码

```cpp
PlanCycleResult MultiDtgPlus::PlanV4(
    const RobotState& robot) {

    UpdateMapDtgAndFrontiers();

    if (auto delta = BuildGraphDelta(); !delta.empty()) {
        spectral_worker_.Submit(
            std::make_shared<const GraphDelta>(std::move(delta)));
    }

    if (auto snapshot = spectral_worker_.TryConsume()) {
        if (ValidateRegionSnapshot(*snapshot)) {
            region_snapshot_ = snapshot;
        }
    }

    if (completion_monitor_.Finished()) {
        return PlanCycleResult::MAP_FINISHED;
    }

    if (target_manager_.CanKeepCurrent(
            robot, map_state_, frontier_state_)) {
        return ContinueCurrentSafeTrajectory();
    }

    CandidateSet candidates;
    candidates.AddCurrentTargetIfValid();
    candidates.AddLocalFrontiers(
        CollectTopKLocalFrontiers(/*K=*/8));

    if (region_snapshot_) {
        candidates.AddAdjacentRegionEntries(
            region_snapshot_->quotient_graph,
            /*max_entries=*/2);
    }

    if (candidates.empty()) {
        candidates = BuildFailOpenCandidates();
    }

    ComputeExactTravelTimes(robot.position, candidates);
    const Candidate next = target_manager_.SelectWithHysteresis(candidates);

    LocalPlan local = BuildEdenLocalViewpoint(next);
    TrajectoryResult trajectory = OptimizeAseo(local);

    if (!trajectory.valid) {
        return KeepCurrentOrSafetyTrajectory();
    }

    PublishTrajectory(trajectory);
    target_manager_.CommitAfterPublish(next, trajectory);
    return PlanCycleResult::EXECUTABLE_SUCCESS;
}
```

---

# 16. 相较当前代码的模块裁剪与基线隔离清单

## 16.1 从V4路径移除、但在EDEN基线中保留

| 当前模块/函数 | 处理方式 |
|---|---|
| `BuildActiveDistanceMatrix()` | 重命名为`BuildEdenActiveDistanceMatrix()`，只供`EDEN_BASELINE`调用；V4不调用 |
| `RunEohdtFallback()`完整tour | 迁移为`PlanGlobalRouteEden()`中的原始EOHDT实现；V4不调用 |
| `Prim()`用于完整全局tour | 仅保留在`mr_dtg_plus_eden_baseline.cpp` |
| 原始EOHDT完整anchor遍历 | 保留用于EDEN基线；从V4调用链完全隔离 |

这里不是物理删除原始EDEN，而是确保：

```text
EDEN模式可以完整复现原始算法
V4模式不承担EDEN完整路线的计算量
```

## 16.2 从V4和Release在线主流程彻底删除

| 当前模块/函数 | V4处理 |
|---|---|
| `BuildRegionAwareRoute()` | 删除完整区域候选路线 |
| `ComputeRouteMetrics()`完整route比较 | 删除 |
| `EvaluateRegionRoute()` | 删除 |
| `ComputeDynamicSwitchPenalty()` | 删除 |
| `UpdateLockDebt()` | 删除 |
| `UpdateSpectralMode()`旧四状态机 | 删除 |
| `EnterRecovery()`和`recovery_until_` | 删除随机时间恢复 |
| `missed_regions_`按spectral center排序 | 删除 |
| `ComputeEdgeBetweenness()` | 在线路径删除 |
| 每周期完整`CompressDegreeTwoChains()` | 替换为Worker中的增量chain维护 |
| `frontier_expected_gain_eps`硬过滤 | 删除硬过滤，只保留排序降权 |
| 基于重复规划次数的quarantine | 删除 |
| EOHDT与V4双路线同时计算和比较 | 删除 |
| 每任务`std::async` | 替换为长期Worker |

## 16.3 顶层调用不变量

```text
planner_mode == EDEN_BASELINE
    → 只进入PlanGlobalRouteEden()
    → 不启动V4 Worker

planner_mode == SPECTRAL_V4
    → 只进入PlanGlobalRouteV4()
    → 不计算完整EOHDT tour
```

---

# 17. 必须保留的EDEN能力

| 能力 | 保留原因 |
|---|---|
| DTG真实拓扑 | V4所有图方法的基础 |
| `ParallelDijkstra`或等价可达性搜索 | 用于root可达性和小候选路径 |
| `Astar()`/`BuildPathToFirst()` | 只搜索到单一目标 |
| EROI和viewpoint生成 | 保证完整探索 |
| `FindFastExpTarget()`中的局部运动可行性 | 继续用于局部目标细化 |
| ASEO exploring/continuous/safety轨迹 | 保持EDEN的连续和安全飞行能力 |
| 当前安全轨迹 | Worker或新规划未完成时继续执行 |

---

# 18. 文件级代码修改建议

## 18.1 统一唯一源码树

### 保留

```text
catkin_ws/src/Mapping_mr_dtg_plus
catkin_ws/src/Exploration_eden
```

### 删除或移出编译仓库

```text
eden_src/EDEN/Mapping/mr_dtg_plus
eden_src/EDEN/Exploration/eden
```

必须确保EDEN基线和V4都位于同一个真实源码树中，而不是分别维护两套目录。

## 18.2 `mr_dtg_plus_structures.h`

删除旧V2/V3运行策略字段：

```text
soft mode
lock debt
random recovery
完整双路线比较状态
复杂frontier隔离状态
```

新增：

```cpp
enum class GlobalPlannerMode {
    EDEN_BASELINE = 0,
    SPECTRAL_V4 = 1
};

enum class CutBackend {
    INCREMENTAL_FIEDLER,
    LANDMARK_NYSTROM,
    LOCAL_APPR,
    UNAVAILABLE
};

GraphDelta
BinaryRegionProposal
RegionTreeNode
QuotientRegion
QuotientEdge
RegionStateSnapshot
CommittedTarget
V4BudgetConfig
```

`SpectralV4Config`必须成为唯一完整的V4配置结构，不能再同时存在`SpectralV41`和`SpectralV2`参数控制主流程。

## 18.3 `mr_dtg_plus.h`

新增接口：

```cpp
GlobalPlanStatus PlanGlobalRouteEden(
    const Eigen::Vector3d& ps,
    std::vector<h_ptr>& route_h,
    std::vector<Eigen::Vector3d>& path2fh,
    double& d1);

GlobalPlanStatus PlanGlobalRouteV4(
    const Eigen::Vector3d& ps,
    std::vector<h_ptr>& route_h,
    std::vector<Eigen::Vector3d>& path2fh,
    double& d1);
```

新增成员：

```cpp
GlobalPlannerMode global_planner_mode_ =
    GlobalPlannerMode::EDEN_BASELINE;

std::unique_ptr<SpectralWorker> spectral_worker_;
std::shared_ptr<const RegionStateSnapshot> region_snapshot_;
CommittedTargetManager target_manager_;
CompletionMonitor completion_monitor_;
```

原始EDEN专用函数可以保留为私有接口，但必须由`PlanGlobalRouteEden()`单独调用。

## 18.4 `mr_dtg_plus_global.cpp`

将其缩减为公共入口：

```text
公共输入检查
公共完成状态读取
诊断初始化
按GlobalPlannerMode分发
```

核心结构：

```cpp
switch (global_planner_mode_) {
case GlobalPlannerMode::EDEN_BASELINE:
    return PlanGlobalRouteEden(...);
case GlobalPlannerMode::SPECTRAL_V4:
    return PlanGlobalRouteV4(...);
}
```

该文件不再包含：

```text
EDEN/V4双路线构造
route regret
soft mode
lock debt
random recovery
```

## 18.5 新增 `mr_dtg_plus_eden_baseline.cpp`

迁移并冻结原始EDEN逻辑：

```text
PlanGlobalRouteEden
BuildEdenActiveDistanceMatrix
BuildOriginalEohdtRoute
Prim
BuildPathToFirst
```

要求：

- 尽量保持原始算法行为不变；
- 不读取任何V4区域状态；
- 不启动或等待Worker；
- 用于原始EDEN复现和公平对照。

## 18.6 新增 `mr_dtg_plus_v4_global.cpp`

实现：

```text
PlanGlobalRouteV4
CommittedTarget检查
小候选集构造
相邻区域入口生成
精确首段路径计算
单一anchor输出
```

V4不得调用：

```text
BuildEdenActiveDistanceMatrix
BuildOriginalEohdtRoute
Prim完整tour
```

兼容现有接口时：

```cpp
route_h = {selected_anchor};
```

## 18.7 拆分旧 `mr_dtg_plus_spectral.cpp`

拆分为：

```text
incremental_skeleton.cpp
incremental_fiedler.cpp
nystrom_bisection.cpp
local_appr.cpp
region_tree_manager.cpp
region_quotient_graph.cpp
spectral_worker.cpp
```

删除旧文件中的：

- 欧氏FPS伪Nyström；
- Top-K距离伪APPR；
- 每任务`std::async`；
- 全量snapshot成员函数；
- edge betweenness；
- active/missed region路线状态；
- V2/V3模式机。

## 18.8 `spectral_router.cpp`

保留纯数值求解能力，接口改成：

```cpp
SpectralSolveResult SolveSmallSparseBisection(
    const ContractedSkeleton& graph,
    const WarmStart& warm_start);
```

要求：

- 只求前三个最小特征对；
- 使用`u2/u3`；
- 稀疏矩阵直接乘法；
- 最大迭代数；
- residual和有限数检查；
- 小图稠密刷新只允许在Worker中执行。

## 18.9 `spectral_v2_policy.cpp/.h`

不再进入V4在线路径。

处理方式：

```text
迁移到legacy目录
或从Release CMake删除
或仅保留旧实验回放
```

EDEN基线也不需要该模块。

## 18.10 `spectral_snapshot_builder.cpp/.h`

当前纯值类型构图器不能继续与新的增量骨架并存两套实现。

建议：

```text
保留其中输入验证和纯函数单元测试
删除全量kNN/MST、全量compression和betweenness
其余由incremental_skeleton.cpp替代
```

## 18.11 `mr_dtg_plus_path_search.cpp`

修改：

1. `frontier_expected_gain_eps`不再作为硬有效性条件；
2. V4只从当前单一anchor或小候选集生成视点；
3. 删除区域惩罚对局部gain的重复影响；
4. 不在`FindFastExpTarget()`末尾记录目标已执行；
5. 返回候选目标，等待轨迹发布成功后再commit。

EDEN模式继续按原路线处理，但共用相同的frontier合法性和数值安全修复。

## 18.12 `eden.cpp`

增加：

```text
当前轨迹继续执行逻辑
轨迹发布成功回调
实际目标增益回传
目标完成/失败事件
```

V4调用顺序：

```text
目标选择
→ ASEO
→ 轨迹校验
→ 发布
→ target_manager.CommitAfterPublish()
```

Worker没有结果或新规划未完成时：

```text
当前轨迹仍安全 → 继续执行
否则 → safety trajectory
```

EDEN模式不使用V4的区域目标承诺，但仍使用公共轨迹校验和安全执行逻辑。

## 18.13 `mr_dtg_plus_initialization.cpp`

首先读取全局模式：

```cpp
std::string planner_mode;
nh_private.param(
    ns + "/MR_DTG/GlobalPlanner/mode",
    planner_mode,
    std::string("spectral_v4"));

if (planner_mode == "eden") {
    global_planner_mode_ = GlobalPlannerMode::EDEN_BASELINE;
} else if (planner_mode == "spectral_v4") {
    global_planner_mode_ = GlobalPlannerMode::SPECTRAL_V4;
} else {
    ROS_FATAL_STREAM(
        "Unknown global planner mode: " << planner_mode);
    throw std::runtime_error(
        "invalid global planner mode");
}
```

只有V4模式才：

```text
加载MR_DTG/SpectralV4/*
创建SpectralWorker
启动Worker
```

EDEN模式不创建Worker。

删除旧主流程参数：

```yaml
MR_DTG/SpectralV2/*
MR_DTG/SpectralV41/*
```

启动日志必须打印：

```text
planner_mode
exact_max_nodes
landmark_count
ppr_max_pushes
worker_max_version_lag
candidate_top_k
switch margins
```

## 18.14 `CMakeLists.txt`

建议：

```cmake
cs_add_library(${PROJECT_NAME}
  src/mr_dtg_plus.cpp
  src/mr_dtg_plus_global.cpp
  src/mr_dtg_plus_eden_baseline.cpp
  src/mr_dtg_plus_v4_global.cpp
  src/mr_dtg_plus_path_search.cpp
  src/mr_dtg_plus_initialization.cpp
  src/mr_dtg_plus_target_refine.cpp
  src/incremental_skeleton.cpp
  src/incremental_fiedler.cpp
  src/nystrom_bisection.cpp
  src/local_appr.cpp
  src/region_tree_manager.cpp
  src/region_quotient_graph.cpp
  src/spectral_worker.cpp
  src/committed_target_manager.cpp)
```

从Release在线库删除：

```text
spectral_v2_policy.cpp
旧mr_dtg_plus_spectral.cpp
重复的旧snapshot主流程
```

EDEN和V4文件可以同时被编译进同一库，但运行时必须由`GlobalPlannerMode`保证只进入其中一条路径。

---

# 19. 推荐双模式配置与启动方式

## 19.1 运行原始EDEN

```yaml
MR_DTG/GlobalPlanner/mode: "eden"
```

EDEN模式：

```text
不加载V4运行参数
不创建SpectralWorker
不维护区域树
只运行原始EOHDT
```

启动示例：

```bash
roslaunch eden single_exp_demo_city_lite_low.launch \
  planner_mode:=eden
```

## 19.2 运行Spectral-EDEN V4

```yaml
MR_DTG/GlobalPlanner/mode: "spectral_v4"

MR_DTG/SpectralV4/enabled: true

# 统一骨架
MR_DTG/SpectralV4/skeleton_max_nodes: 512
MR_DTG/SpectralV4/max_leaf_regions: 6
MR_DTG/SpectralV4/max_recursive_depth: 3

# Incremental Fiedler
MR_DTG/SpectralV4/exact_max_nodes: 48
MR_DTG/SpectralV4/exact_max_iterations: 12
MR_DTG/SpectralV4/exact_residual_threshold: 1.0e-4

# Nyström
MR_DTG/SpectralV4/landmark_count: 32
MR_DTG/SpectralV4/landmark_max_count: 48
MR_DTG/SpectralV4/nearest_landmarks_per_node: 4
MR_DTG/SpectralV4/nystrom_max_support: 512
MR_DTG/SpectralV4/nystrom_residual_threshold: 5.0e-4

# Local APPR
MR_DTG/SpectralV4/ppr_alpha: 0.15
MR_DTG/SpectralV4/ppr_epsilon: 1.0e-4
MR_DTG/SpectralV4/ppr_max_pushes: 20000
MR_DTG/SpectralV4/ppr_max_support_nodes: 512

# 二分接受
MR_DTG/SpectralV4/max_ncut: 0.30
MR_DTG/SpectralV4/min_balance: 0.20
MR_DTG/SpectralV4/proposal_confirm_versions: 2
MR_DTG/SpectralV4/proposal_jaccard_threshold: 0.80
MR_DTG/SpectralV4/max_region_version_lag: 2

# 目标承诺
MR_DTG/SpectralV4/local_frontier_top_k: 8
MR_DTG/SpectralV4/max_adjacent_region_entries: 2
MR_DTG/SpectralV4/min_target_commit_sec: 1.5
MR_DTG/SpectralV4/target_switch_abs_margin_sec: 0.7
MR_DTG/SpectralV4/target_switch_relative_margin: 0.10

# Worker
MR_DTG/SpectralV4/latest_only_mailbox: true
MR_DTG/SpectralV4/worker_max_version_lag: 2
MR_DTG/SpectralV4/main_thread_delta_budget_ms: 2.0
```

启动示例：

```bash
roslaunch eden single_exp_demo_city_lite_low.launch \
  planner_mode:=spectral_v4
```

## 19.3 Launch文件

```xml
<arg name="planner_mode" default="spectral_v4"/>

<param
    name="MR_DTG/GlobalPlanner/mode"
    value="$(arg planner_mode)"/>
```

## 19.4 配置不变量

```text
mode=eden
    → SpectralWorker必须为null
    → Fiedler/Nyström/APPR调用次数为0

mode=spectral_v4
    → 不调用完整EOHDT tour
    → Worker永不阻塞主线程
```

这些V4参数是安全起点，不是未经实验即可宣称最优的最终值。

---

# 20. 计算复杂度

## 19.1 当前方法

主要成本近似包含：

\[
O(n^2\cdot C_{path})
+
O(n^2)
+
O(n^3)
+
O(n^2)
\]

其中：

- 全对距离维护的实际成本还乘以DTG路径搜索；
- 稠密谱求解为立方复杂度；
- 两条完整路线和全量区域状态进一步增加常数和方差。

## 19.2 V4主线程

候选数固定为`K`：

\[
O(C_{root}+K\log|V|)
\]

`K≤10`，不再维护全对矩阵。

## 19.3 Incremental Fiedler Worker

\[
O(I|E_s|),\qquad |V_s|\le48
\]

## 19.4 Nyström Worker

\[
O(m^3+qn),\qquad m\le48,\ q=4
\]

## 19.5 APPR Worker

\[
O(P)
\]

其中`P`由`ppr_max_pushes`硬限制。

## 19.6 最坏情况控制

主线程从不等待Worker，因此Worker即使达到较高P99，也不会让UAV停止。

真正需要验收的是：

```text
main-thread P99
目标切换频率
当前轨迹中断次数
```

而不是只看Worker单次求解耗时。

---

# 21. 防振荡回归测试

## 20.1 双目标交替测试

两个候选成本在噪声下交替：

```text
A: 10.0 ± 0.3
B: 10.1 ± 0.3
```

要求：在承诺期和切换裕量内，目标不得每周期切换。

## 20.2 分区符号翻转测试

人为将Fiedler向量整体乘以`-1`。

要求：

- 区域ID通过最大重叠保持；
- 当前目标不变；
- 区域商图拓扑不变。

## 20.3 后端切换测试

同一个图分别用：

```text
Incremental Fiedler
Nyström
APPR
```

要求：输出被统一映射到同一个父区域，后端切换不能直接改变飞行目标。

## 20.4 Worker延迟测试

让Worker人为sleep 1000 ms。

要求：

- UAV继续当前轨迹；
- 主规划线程没有1000 ms尖峰；
- 不调用wait/get。

## 20.5 旧结果测试

提交版本100的任务，Worker完成前图更新到103。

要求：

- 超过允许版本差则拒绝；
- 不清空当前稳定区域；
- 不切换当前目标。

## 20.6 frontier抖动测试

一个frontier的expected gain在阈值上下波动。

要求：

- 只改变排序；
- 不立即删除frontier；
- 不触发区域DONE；
- 不引起UAV往返。

---

## 21.7 EDEN基线纯净性测试

配置：

```yaml
MR_DTG/GlobalPlanner/mode: "eden"
```

要求：

- `SpectralWorker`未创建；
- Fiedler、Nyström和APPR调用次数均为0；
- 不读取`RegionStateSnapshot`；
- 不调用V4目标承诺逻辑；
- 路线结果与冻结的原始EDEN基线一致；
- 日志明确显示`mode=EDEN_BASELINE`。

## 21.8 V4独立性测试

配置：

```yaml
MR_DTG/GlobalPlanner/mode: "spectral_v4"
```

要求：

- 不调用`BuildEdenActiveDistanceMatrix()`；
- 不调用完整`BuildOriginalEohdtRoute()`；
- 不构造EDEN/V4两条路线进行比较；
- Worker延迟不阻塞主线程；
- 日志明确显示`mode=SPECTRAL_V4`和当前后端。

## 21.9 启动切换复现实验

使用相同地图、随机种子和公共参数，分别重新启动运行：

```text
EDEN_BASELINE
SPECTRAL_V4
```

要求：

- 两次运行各自只初始化对应模式；
- 上一次运行的Worker、区域树和目标状态不会进入下一次运行；
- 统计文件必须记录`planner_mode`；
- 不通过运行中修改参数完成实验切换。

## 21.10 错误配置测试

将模式设为：

```yaml
MR_DTG/GlobalPlanner/mode: "invalid_mode"
```

要求节点启动失败并输出清晰错误，不得静默回退。

---

# 22. 性能验收指标

## 21.1 规划延迟

| 指标 | 目标 |
|---|---:|
| 图更新/GraphDelta主线程P50 | ≤1 ms |
| 图更新/GraphDelta主线程P99 | ≤2 ms |
| 全局单目标选择P99 | ≤10 ms |
| 完整规划P99 | 不高于EDEN基线15% |
| UAV等待Worker次数 | 0 |

## 21.2 振荡指标

新增：

```text
target_switch_count
region_switch_count
direction_reversal_count
same_two_targets_pingpong_count
trajectory_abort_due_to_retarget
```

目标：

| 指标 | 目标 |
|---|---:|
| 连续A↔B目标ping-pong | 0 |
| 非失效原因目标切换间隔 | ≥min_target_commit_sec |
| 每100 m方向反转次数 | 不高于EDEN |
| 因新谱结果中止安全轨迹 | 0 |

## 21.3 路径和稳定性

| 指标 | 目标 |
|---|---:|
| 路径相对EDEN增长 | ≤2% |
| 平均速度相对EDEN下降 | ≤5% |
| 同地图探索时间Std | ≤EDEN的1.2倍 |
| City P99规划尖峰 | 不出现>1 s |

---

# 23. 分阶段实施顺序

## 阶段0：统一源码并建立EDEN/V4双模式入口

1. 只保留一个源码树；
2. 清理`build/devel`；
3. 冻结原始EDEN行为；
4. 新建`mr_dtg_plus_eden_baseline.cpp`；
5. 新建`mr_dtg_plus_v4_global.cpp`；
6. 增加`GlobalPlannerMode`；
7. 增加`MR_DTG/GlobalPlanner/mode`；
8. `PlanGlobalRoute()`改为纯分发器；
9. Worker只在V4模式创建；
10. 确认CMake同时编译两种实现，但运行时只进入一条路径；
11. 建立固定commit和干净构建脚本；
12. 验证EDEN模式结果与冻结基线一致。

## 阶段1：先解决V4目标振荡

1. 实现`CommittedTargetManager`；
2. 删除完整V4 candidate route；
3. V4只选择一个anchor；
4. 目标发布成功后再commit；
5. 禁用soft mode、lock debt和random recovery；
6. 保持当前安全轨迹；
7. 验证A↔B目标来回切换消失。

## 阶段2：从V4路径删除主要计算冗余

1. V4不调用`BuildEdenActiveDistanceMatrix()`；
2. V4不调用每周期完整EOHDT tour；
3. 原始EDEN基线中的这两项保持不变；
4. 从V4删除edge betweenness；
5. V4候选集固定为TOP-K；
6. V4只搜索到单一目标。

## 阶段3：实现真正Worker和增量骨架

1. 长期Worker线程；
2. latest-only mailbox；
3. Worker私有稀疏图；
4. GraphDelta；
5. 增量chain contraction；
6. 主线程只提交轻量delta。

## 阶段4：接入Incremental Fiedler

1. 小骨架求`u2/u3`；
2. warm start；
3. 残差和Ncut；
4. 统一proposal；
5. 区域树每次只接受一个切割。

## 阶段5：接入真正Nyström

1. 持久化拓扑地标；
2. 构造`W/C`矩阵；
3. 固定小矩阵求谱；
4. Nyström扩展；
5. 与Fiedler后端统一输出。

## 阶段6：接入真正APPR

1. residual push；
2. conductance sweep；
3. 固定push预算；
4. 作为全局方法不可用时的局部后端。

## 阶段7：双模式完整验收

1. EDEN模式10次重复实验；
2. V4模式10次重复实验；
3. 两种模式分别确认初始化日志；
4. EDEN模式确认谱调用次数为0；
5. V4模式确认完整EOHDT调用次数为0；
6. 对比路径、时间、P50/P95/P99和标准差。

---

# 24. 最终架构与当前代码的根本区别

| 当前代码 | 双模式统一架构 |
|---|---|
| 两套源码树 | 一个唯一源码树 |
| 模式由多个`enabled/fallback`隐式决定 | `GlobalPlannerMode`启动时明确二选一 |
| V4配置旁挂在V2/V3主流程 | 顶层分发到纯EDEN或纯V4 |
| 同周期计算EOHDT完整路线和谱完整路线 | EDEN模式只算EOHDT；V4模式只选一个宏观目标 |
| V4失败就回完整EOHDT | V4继续当前目标或局部候选，不触发完整EOHDT |
| EDEN也可能维护谱状态 | EDEN模式不创建Worker、不维护区域树 |
| 全对anchor距离矩阵 | 仅EDEN基线保留；V4使用固定小候选精确路径 |
| kNN + 第二棵MST | V4使用真实DTG增量骨架 |
| 每次全量走廊压缩 | V4局部chain增量维护 |
| 在线edge betweenness | V4删除 |
| 全局稠密谱求解 | V4小骨架增量Fiedler |
| FPS子采样冒充Nyström | V4真实固定地标Nyström |
| TOP-K启发式冒充APPR | V4真实residual push + sweep |
| soft-lock/lock debt/cooldown | V4目标承诺 + 确定性迟滞 |
| 单次Ncut失败清空分区 | V4保留最后稳定区域状态 |
| 每周期重排区域和目标 | V4只在硬事件或显著收益时切换 |
| 每任务`std::async` | V4长期Worker + latest-only mailbox |
| 目标选择即计为执行 | V4轨迹发布成功后才commit |
| 难以确认实验运行哪种算法 | 日志强制记录`planner_mode`和`v4_backend` |

---

# 25. 最终建议

整个仓库应保留两个可独立运行的全局规划模式：

\[
\boxed{
\text{EDEN\_BASELINE}
\quad\text{或}\quad
\text{SPECTRAL\_V4}
}
\]

但两者不能在同一个规划周期内同时计算。

正确结构是：

\[
\boxed{
\begin{aligned}
&\text{GlobalPlanner启动时明确选择EDEN\_BASELINE或SPECTRAL\_V4；}\\
&\text{原始EDEN完整保留，用于基线和对照，但不参与V4周期内的双路线计算；}\\
&\text{V4.1提供唯一的持久化骨架和区域商图框架；}\\
&\text{V4.3成为大骨架下的固定规模二分求解后端；}\\
&\text{V4.2成为全局方法不可用时的局部二分后端；}\\
&\text{三种V4后端输出同一个BinaryRegionProposal；}\\
&\text{V4规划层永远只维护一个CommittedTarget。}
\end{aligned}
}
\]

最终运行方式：

```text
planner_mode=eden
    → 原始EDEN/EOHDT
    → 不启动V4 Worker

planner_mode=spectral_v4
    → 统一Spectral-EDEN V4
    → Fiedler/Nyström/APPR三选一
    → 不计算完整EOHDT基线路线
```

优先级固定为：

\[
\boxed{
\text{完整探索}
>
\text{轨迹连续和无振荡}
>
\text{主线程实时性}
>
\text{路径效率}
>
\text{谱近似精度}
}
\]

当V4谱结果未完成、过期或失败时，系统继续当前安全目标或使用局部EDEN视点能力，不等待Worker、不进入随机cooldown，也不临时重新生成完整EOHDT tour。

第一版应只支持启动时通过Launch/YAML切换。待两种模式分别稳定通过回归测试后，再单独设计运行时热切换；热切换不属于当前V4首版的必要功能。

