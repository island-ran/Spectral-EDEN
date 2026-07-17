# Spectral-EDEN 改进方案：针对路径增长、分区震荡、计算开销与末期退化

> **文档用途**：用于下一轮 Spectral-EDEN 算法重构、源代码修改和论文消融实验设计。  
> **核心原则**：地图完整探索是硬约束；总探索时间、飞行路径长度和规划计算量是在完整探索前提下共同优化的性能目标。  
> **关键修正**：谱图只负责提供“区域结构证据”，不能直接充当强制路径约束；所有谱策略必须受到路径代价、稳定性和计算预算约束。

---

# 1. 六个问题的统一根因

当前 Spectral-EDEN 把谱图同时用于了三件事：

1. 判断当前已知地图是否存在拓扑分区；
2. 决定 UAV 应先访问哪个区域；
3. 强制 UAV 留在当前区域，直到该区域所有 frontier 被清空。

其中，谱图最适合解决的是第 1 项，而不是第 2、3 项。

谱切割优化的是：

$$
\min \operatorname{Ncut}(C_1,C_2)
$$

它描述“哪些节点组内连接强、组间连接弱”。

路径规划优化的是：

$$
\min_R
\left[
d(p_t,R_1)
+
\sum_{k=1}^{m-1}d(R_k,R_{k+1})
\right]
$$

它描述“以什么顺序访问目标能够走得更短”。

因此：

$$
\boxed{
\text{谱分区合理}
\not\Rightarrow
\text{按区域严格穷尽时路径更短}
}
$$

当前六个问题，本质上可以归结为：

```text
谱分区被过度使用
+
分区状态缺少时间稳定性
+
谱计算没有预算控制
+
固定阈值不具备尺度适应性
+
探索末期没有退出机制
```

---

# 2. 改进后的总体目标

Spectral-EDEN 不应以“尽可能多地触发谱分区”为目标，而应满足：

$$
\boxed{
\text{只在谱分区能够降低未来重复访问，}
\text{且不会明显增加当前路径和计算量时启用}
}
$$

建议将整体问题写成：

$$
\begin{aligned}
\min_{\pi}\quad
J(\pi)
&=
w_T T_{\mathrm{exp}}
+
w_L L_{\mathrm{path}}
+
w_C C_{\mathrm{plan}}
+
w_B N_{\mathrm{BFM}}\\
\text{s.t.}\quad
&\operatorname{CompletionRate}=1,\\
&\operatorname{CollisionCount}=0,\\
&T_{\mathrm{plan,P95}}
\le
T_{\mathrm{budget}}.
\end{aligned}
$$

其中：

- $T_{\mathrm{exp}}$：完整探索时间；
- $L_{\mathrm{path}}$：总飞行距离；
- $C_{\mathrm{plan}}$：累计规划计算开销；
- $N_{\mathrm{BFM}}$：区域间无效往返次数。

目标优先级为：

$$
\boxed{
\text{完整探索}
>
\text{探索时间}
+
\text{路径长度}
+
\text{计算量}
}
$$

---

# 3. 新的总体架构

建议把 Spectral-EDEN 改成“谱证据驱动的混合规划器”：

```text
地图 / DTG 更新
        ↓
有效 frontier 清理与动态归属
        ↓
判断是否值得进行谱计算
  ├─ 否：直接使用原始 EDEN
  └─ 是：计算候选谱分区
                ↓
      分区置信度与时间稳定器
        ├─ 不稳定：保留旧稳定结果或关闭谱影响
        └─ 稳定：输出区域结构先验
                ↓
    同时生成两条候选全局路线
    ├─ 原始 EOHDT 路线
    └─ 区域感知路线
                ↓
  路径后悔上限 + 综合代价比较
                ↓
      软区域偏好，而非硬锁
                ↓
末期关闭 / 停滞恢复 / 残留 frontier 处理
                ↓
原 EDEN 局部视点评分与 ASEO 轨迹规划
```

---

# 4. 问题 1：区域锁定强制次优路径

## 4.1 问题分析

当前逻辑是：

```text
partition_active = true
→ active_region = 0
→ 只允许选择 active_region 内的 frontier
→ 当前区域不清空，禁止跨区
```

这相当于把区域标签变成了硬约束。

当邻区 frontier 仅 2 m，而当前区域剩余 frontier 需要绕行 10 m 时，硬锁仍会要求 UAV 先完成当前区域，从而直接增加飞行距离。

这与实验现象一致：

- Large Maze：Traveled Distance 增加 6.4%；
- City：Traveled Distance 增加 16.6%。

---

## 4.2 改进一：硬锁改为软区域偏好

删除：

```cpp
if (!active_region_finished) {
  only_search_active_region = true;
}
```

改为在候选目标代价中加入软切换惩罚：

$$
J(v)
=
T(v)
-
\beta_I I(v)
+
\lambda_{\mathrm{switch}}(t)
\mathbf 1[r(v)\neq r_{\mathrm{active}}]
$$

其中：

- $T(v)$：到候选视点的预计飞行时间；
- $I(v)$：候选视点的信息增益；
- $r(v)$：候选视点所属区域；
- $\lambda_{\mathrm{switch}}$：跨区软惩罚。

动态切换惩罚：

$$
\lambda_{\mathrm{switch}}(t)
=
\lambda_0
C_{\mathrm{part}}(t)
P_{\mathrm{return}}(t)
$$

其中：

- $C_{\mathrm{part}}\in[0,1]$：当前分区置信度；
- $P_{\mathrm{return}}\in[0,1]$：离开当前区域后仍需返回的估计概率。

当前区域仍有大量有效 frontier 时，$P_{\mathrm{return}}$ 较大；只剩少量低收益 frontier 时，跨区惩罚自动减小。

---

## 4.3 改进二：设置邻区近距离例外

若邻区候选目标非常近，则允许直接跨区。

条件：

$$
d(p_t,v_{\mathrm{out}})
\le
d_{\mathrm{neighbor}}
$$

且：

$$
J(v_{\mathrm{out}})
+
\delta_{\mathrm{margin}}
<
J(v_{\mathrm{in}})
$$

建议初值：

```yaml
neighbor_override_distance: 2.0
neighbor_override_margin: 0.5
```

这样可直接处理：

> 邻区 frontier 仅 2 m，但当前区域剩余目标明显更远。

---

## 4.4 改进三：同时生成原 EDEN 路线和区域感知路线

每次规划同时生成：

- $R_E$：原始 EOHDT 路线；
- $R_S$：区域感知路线。

路线长度估计：

$$
C_{\mathrm{path}}(R)
=
d(p_t,R_1)
+
\sum_{k=1}^{m-1}d(R_k,R_{k+1})
$$

综合路线代价：

$$
J_{\mathrm{route}}(R)
=
C_{\mathrm{path}}(R)
+
\lambda_{\mathrm{sw}}N_{\mathrm{switch}}(R)
+
\lambda_{\mathrm{rv}}D_{\mathrm{revisit}}(R)
$$

只有同时满足：

$$
J_{\mathrm{route}}(R_S)
<
J_{\mathrm{route}}(R_E)
$$

以及：

$$
C_{\mathrm{path}}(R_S)
\le
(1+\varepsilon_R)
C_{\mathrm{path}}(R_E)
$$

才采用区域感知路线。

建议：

```yaml
max_route_regret: 0.05
```

即谱策略相对于原 EDEN 的预计路径增长最多允许 5%。

---

## 4.5 改进四：累计绕路债务

即使单次绕路只增加少量距离，多次累积也会明显增加总路径。

维护：

$$
B_{\mathrm{lock}}(t+1)
=
\eta_B B_{\mathrm{lock}}(t)
+
\max
\left(
0,
C_{\mathrm{selected}}-C_{\mathrm{EDEN}}
\right)
$$

当：

$$
B_{\mathrm{lock}}>B_{\max}
$$

则：

```text
进入 RECOVERY
→ 暂时关闭区域偏好
→ 使用原始 EOHDT
```

建议：

```yaml
lock_debt_decay: 0.90
lock_debt_max: 8.0
recovery_duration: 5.0
```

---

# 5. 问题 2：分区不稳定导致回溯浪费

## 5.1 问题分析

City 中出现：

```text
epoch 534：ncut = 0.078，分区激活
epoch 572：eigengap = 0.008，分区消失
epoch 586：分区重新激活
```

如果分区状态直接由单次谱结果决定，就会出现：

```text
锁定
→ 解锁
→ 全局重规划
→ 再次锁定
```

这会造成路线首目标频繁变化和不必要的回溯。

---

## 5.2 改进一：使用双阈值滞回

定义连续分区置信度：

$$
C_{\mathrm{part}}(t)\in[0,1]
$$

采用不同激活和关闭阈值：

$$
C_{\mathrm{part}}(t)
\ge
\tau_{\mathrm{on}}
\Rightarrow
\text{允许激活}
$$

$$
C_{\mathrm{part}}(t)
\le
\tau_{\mathrm{off}}
\Rightarrow
\text{允许关闭}
$$

并满足：

$$
\tau_{\mathrm{on}}
>
\tau_{\mathrm{off}}
$$

建议：

```yaml
partition_confidence_on: 0.70
partition_confidence_off: 0.40
```

---

## 5.3 改进二：激活和关闭使用不同持续次数

```yaml
partition_on_persistence: 3
partition_off_persistence: 5
partition_grace_epochs: 8
```

逻辑：

```text
连续 3 次强信号
→ 激活分区

单次弱信号
→ 不立即关闭

连续 5 次弱信号
且超过 8 个 epoch 宽限期
→ 才关闭分区
```

分区的关闭应该比激活更保守。

---

## 5.4 改进三：持久区域 ID

不要每次重新将区域标为 0 和 1。

对新旧区域计算 Jaccard 重叠：

$$
J(C_a^{t-1},C_b^t)
=
\frac{
|C_a^{t-1}\cap C_b^t|
}{
|C_a^{t-1}\cup C_b^t|
}
$$

将新区域映射到重叠最大的旧区域 ID。

建议数据结构：

```cpp
struct PersistentRegion {
  int persistent_id;
  std::unordered_set<int> hnode_ids;
  double confidence;
  int age_epochs;
  int weak_epochs;
  RegionExecState state;
};
```

---

## 5.5 改进四：Fiedler 符号对齐与节点标签滞回

Fiedler 向量存在符号不唯一性：

$$
u_2
\quad\text{和}\quad
-u_2
$$

均是合法解。

必须做时间对齐：

$$
u_2(t)
\leftarrow
\operatorname{sign}
\left(
\widetilde u_2(t-1)^\top u_2(t)
\right)
u_2(t)
$$

对靠近分割阈值的节点使用标签滞回：

$$
c_i(t)
=
\begin{cases}
0,&f_i<\theta_t-\delta_f\\
1,&f_i>\theta_t+\delta_f\\
c_i(t-1),&|f_i-\theta_t|\le\delta_f
\end{cases}
$$

建议：

```yaml
label_hysteresis_ratio: 0.08
region_match_threshold: 0.50
```

---

# 6. 问题 3：计算成本非线性增长

## 6.1 问题分析

City 中：

- EDEN：约 49 ms；
- Spectral-EDEN：约 81 ms；
- 增长约 64%；
- 1,011 次谱求解累计约 80 s；
- 占总运行时间约 5%。

说明谱求解频率和图规模均没有受到控制。

---

## 6.2 改进一：事件驱动，而不是每轮都算

新增：

```cpp
bool NeedSpectralUpdate();
```

只在以下条件之一成立时重算：

- 活跃 frontier 锚点数量变化超过阈值；
- H-H 边发生显著增删；
- 发现新的跨区域连接；
- 当前稳定分区置信度持续下降；
- 距上次计算超过最大更新时间。

建议：

```yaml
spectral_min_update_interval: 0.5
spectral_max_update_interval: 3.0
dirty_node_changes: 3
dirty_edge_changes: 5
```

---

## 6.3 改进二：只对相关支撑子图求谱

不要将全部历史 H-node 永久加入谱图。

定义：

$$
V_t^{\mathrm{spec}}
=
A_t
\cup
S_t
\cup
\bigcup_{h_i,h_j\in A_t^{(k)}}P_{ij}
$$

其中：

- $A_t$：当前活跃边界 H-node；
- $S_t$：当前 UAV 附近种子 H-node；
- $A_t^{(k)}$：每个活跃节点的 $k$ 个相关近邻；
- $P_{ij}$：DTG 上连接这些节点的支撑路径。

建议：

```yaml
spectral_knn: 5
max_spectral_nodes: 80
```

---

## 6.4 改进三：走廊链压缩

若 H-node 满足：

$$
\deg(h)=2
$$

且没有 H-F edge，则将：

```text
h_a — h_1 — h_2 — ... — h_k — h_b
```

压缩为：

```text
h_a — h_b
```

新边长度：

$$
\ell_{ab}
=
\sum_{i=0}^{k}\ell_i
$$

净空：

$$
c_{ab}
=
\min_i c_i
$$

这能在保留走廊结构的同时，大幅降低谱矩阵维度。

---

## 6.5 改进四：小图和大图使用不同求解器

```text
n ≤ 40
→ Eigen::SelfAdjointEigenSolver

40 < n ≤ 80
→ 稀疏迭代求解器 + 热启动

n > 80
→ 先压缩；仍超限则跳过本次谱计算
```

设置硬时间预算：

```yaml
spectral_time_budget_ms: 5.0
```

若超时：

```text
丢弃本次候选谱结果
→ 保留最近一次稳定分区
→ 当前规划使用原始 EDEN
```

---

## 6.6 改进五：异步谱线程

主规划线程：

```text
读取最近一次已确认谱结果
→ 立即规划
```

谱线程：

```text
复制只读 DTG snapshot
→ 求解
→ 输出 candidate result
```

结果必须带：

```cpp
uint64_t graph_version;
```

若结果返回时版本过旧，则直接丢弃。

---

# 7. 问题 4：触发条件过于苛刻

## 7.1 不应把“不触发”一律视为失败

当前场景：

| 地图 | 最低 Ncut | 是否触发 | 结构 |
|---|---:|---:|---|
| DARPA Tunnel | 0.667 | 否 | 单走廊 |
| Classical Office | 0.658 | 否 | 多房间但紧密连接 |
| Large Maze | 0.078 | 是 | 多分支长廊 |
| City | 0.125 | 是 | 街区路网 |

正确解释应是：

- DARPA Tunnel 本身没有明显可分的两大区域；
- Classical Office 连接紧密，强制分区未必有益；
- Large Maze 和 City 才具有较明显的区域弱连接。

因此：

$$
\boxed{
\text{不触发谱分区不等于失败}
}
$$

系统应允许：

```text
没有明显区域结构
→ 直接运行原始 EDEN
```

真正需要评价的是：

- 该触发时是否触发；
- 触发后是否减少回访；
- 触发后是否满足路径后悔上限；
- 不该触发时是否保持关闭。

---

## 7.2 小图直接关闭谱分区

当：

$$
n<N_{\min}
$$

不进行分区。

建议：

```yaml
min_spectral_nodes: 10
```

小规模图中强行切割通常没有稳定统计意义。

---

# 8. 问题 5：固定 $\lambda_2$ 与 Ncut 阈值耦合

## 8.1 问题分析

固定条件：

```text
lambda2 < 0.15
AND
ncut < 0.30
AND
eigengap > 0.02
```

会产生两个问题：

1. 任一条件不满足，整个分区被否决；
2. $\lambda_2$ 对图规模、图密度和权重尺度敏感。

City 中 Ncut 已达到 0.15～0.25，但 $\lambda_2$ 保持在 0.2～0.5，导致迟迟不触发。

---

## 8.2 改进一：使用相对量而不是固定绝对量

### 相对 eigengap

$$
g_t
=
\frac{
\lambda_3-\lambda_2
}{
\lambda_3+\varepsilon
}
$$

### 相对代数连通度

$$
r_t
=
\frac{
\lambda_2(t)
}{
\bar\lambda_2(t)+\varepsilon
}
$$

其中：

$$
\bar\lambda_2(t)
=
\alpha\bar\lambda_2(t-1)
+
(1-\alpha)\lambda_2(t)
$$

这样判断的是：

> 当前图相对于自身历史是否出现明显弱连接。

而不是将所有地图都与固定 0.15 比较。

---

## 8.3 改进二：将多个指标合成为连续置信度

### Ncut 质量

$$
q_{\mathrm{ncut}}
=
1-
\operatorname{clip}
\left(
\frac{\operatorname{Ncut}}
{\tau_{\mathrm{ncut,soft}}},
0,1
\right)
$$

### eigengap 质量

$$
q_{\mathrm{gap}}
=
\operatorname{clip}
\left(
\frac{g_t}{\tau_{\mathrm{gap,soft}}},
0,1
\right)
$$

### 相对连通度下降

$$
q_{\lambda}
=
1-
\operatorname{clip}(r_t,0,1)
$$

### 区域平衡度

$$
q_{\mathrm{balance}}
=
\frac{
2\min(|C_0|,|C_1|)
}{
|C_0|+|C_1|
}
$$

### 瓶颈支持度

$$
q_{\mathrm{bottle}}
=
1-
\frac{
\operatorname{median}(c_{\mathrm{cut}})
}{
\operatorname{median}(c_{\mathrm{inside}})+\varepsilon
}
$$

最终分区置信度：

$$
C_{\mathrm{part}}
=
w_n q_{\mathrm{ncut}}
+
w_g q_{\mathrm{gap}}
+
w_\lambda q_\lambda
+
w_b q_{\mathrm{bottle}}
+
w_p q_{\mathrm{balance}}
$$

并满足：

$$
\sum w_i=1
$$

建议：

```yaml
confidence_weight_ncut: 0.30
confidence_weight_eigengap: 0.25
confidence_weight_relative_lambda2: 0.15
confidence_weight_bottleneck: 0.20
confidence_weight_balance: 0.10
```

这比多个硬阈值串联更适合跨场景。

---

# 9. 问题 6：探索末期谱图退化

## 9.1 问题分析

City：

| 覆盖率 | 分区率 |
|---|---:|
| 95% | 90% |
| 99% | 44% |
| 100% | 37% |

探索末期：

- 剩余 frontier 数量少；
- frontier 集中在单一连通区域；
- 不再存在明显的两个大分支；
- 谱分区收益降低；
- 继续求谱只增加计算量。

---

## 9.2 改进一：在线末期检测

在线运行时不知道真实覆盖率，因此使用：

- 有效 frontier 数量 $N_F$；
- frontier 总信息增益 $G_F$；
- 连续无可靠分区次数 $M_{\mathrm{nocut}}$；
- 谱支撑节点数量 $N_S$；
- 最近窗口新增未知体积减少率。

定义：

$$
\operatorname{LateStage}(t)
=
\mathbf 1
\left[
N_F
\le
N_{\mathrm{late}}
\lor
G_F
\le
G_{\mathrm{late}}
\lor
M_{\mathrm{nocut}}
\ge
M_{\mathrm{off}}
\right]
$$

建议：

```yaml
late_stage_frontier_count: 6
late_stage_total_gain: 50
late_stage_no_cut_epochs: 10
```

---

## 9.3 改进二：末期自动关闭谱模块

进入末期后：

```text
停止周期性谱求解
→ 清除 active_region 的路线约束
→ 使用原始 EOHDT
→ 加强残留 frontier 验证
→ 完成最后少量目标
```

只有当 frontier 数量重新明显增加时才重新启用：

```yaml
late_stage_reactivate_frontier_count: 12
```

采用双阈值避免反复进出末期状态。

---

# 10. 残留 frontier 与局部转圈的同步改进

虽然六个问题主要集中在谱策略，但区域锁路径变长往往会被残留 frontier 放大，因此应同步修改区域完成判据。

---

## 10.1 区域完成不能由 `hf_edges_` 是否为空决定

定义 frontier 对区域 $r$ 有效：

$$
\operatorname{Effective}(f,r,t)
=
\operatorname{Alive}(f)
\land
\operatorname{Reachable}(f,r)
\land
\operatorname{Owner}(f)=r
\land
I_t(f)>\varepsilon_I
\land
\neg\operatorname{Quarantined}(f)
$$

区域阻塞 frontier 集合：

$$
\mathcal F_r^{\mathrm{blocking}}(t)
=
\left\{
f:
\operatorname{Effective}(f,r,t)=1
\right\}
$$

区域完成：

$$
\operatorname{RegionDone}(r,t)=1
\iff
\mathcal F_r^{\mathrm{blocking}}(t-k)=\varnothing,
\quad
k=0,\ldots,K_r-1
$$

---

## 10.2 Frontier 生命周期

```cpp
enum class FrontierState {
  NEW,
  ACTIVE,
  SUSPECT,
  DEFERRED,
  QUARANTINED,
  DEAD
};
```

| 状态 | 含义 | 是否阻止当前区域完成 |
|---|---|---:|
| `NEW` | 新产生，等待验证 | 暂时阻止 |
| `ACTIVE` | 可达、有增益、归属明确 | 阻止 |
| `SUSPECT` | 多次低收益或路径失败 | 暂时阻止 |
| `DEFERRED` | 更适合从其他区域访问 | 不阻止 |
| `QUARANTINED` | 疑似伪 frontier，延迟复查 | 不阻止 |
| `DEAD` | 已探索或确认失效 | 不阻止 |

---

## 10.3 动态区域归属

对 frontier $f$，计算各区域到其有效视点的最小代价：

$$
c_r(f)
=
\min_{\substack{h\in C_r\\v\in\mathcal V_f^{\mathrm{valid}}}}
d(h,v)
$$

归属：

$$
\operatorname{Owner}(f)
=
\arg\min_r c_r(f)
$$

若当前区域不可达，而其他区域可达：

```text
f → DEFERRED
owner_region = 其他区域
```

此 frontier 不再阻止当前区域完成。

---

## 10.4 实际信息增益

执行前后记录目标邻域 unknown 体素变化：

$$
G_f^{\mathrm{actual}}
=
U_f^{\mathrm{before}}
-
U_f^{\mathrm{after}}
$$

若连续 $K_{\mathrm{low}}$ 次：

$$
G_f^{\mathrm{actual}}
<
\varepsilon_{\mathrm{actual}}
$$

则：

```text
ACTIVE → SUSPECT
强检查后仍无效 → QUARANTINED
```

建议：

```yaml
frontier_low_gain_cycles: 2
frontier_actual_gain_eps: 10
frontier_quarantine_time: 5.0
```

---

## 10.5 区域停滞看门狗

定义窗口 $W$ 内区域进度：

$$
P_r(t)
=
w_U\Delta U_r
+
w_F\Delta|\mathcal F_r^{\mathrm{blocking}}|
+
w_G
\sum_{\tau=t-W}^{t}
G_{f_\tau}^{\mathrm{actual}}
$$

若：

$$
P_r(t)<\varepsilon_P
$$

持续超过 $T_{\mathrm{stall}}$，执行：

```text
批量验证残留 frontier
→ 重新分配 owner
→ 隔离重复低收益 frontier
→ 若 blocking frontier 为空，结束区域
→ 否则进入 RECOVERY 并暂时关闭区域偏好
```

建议：

```yaml
region_stall_window: 8.0
region_stall_timeout: 12.0
repeat_target_limit: 3
```

---

# 11. 四种运行模式

建议新增：

```cpp
enum class SpectralMode {
  DISABLED,
  OBSERVING,
  ACTIVE_SOFT,
  RECOVERY
};
```

## `DISABLED`

条件：

- 图节点少于阈值；
- 已进入探索末期；
- 计算预算不足；
- 连续无可靠分区。

行为：完全使用原始 EDEN。

## `OBSERVING`

条件：

- 图规模足够；
- 存在潜在谱信号；
- 时间稳定性尚未确认。

行为：计算候选分区，但不改变 UAV 路线。

## `ACTIVE_SOFT`

条件：

- 分区置信度稳定；
- 区域感知路线满足路径后悔上限。

行为：使用软区域偏好。

## `RECOVERY`

条件：

- 路径债务超限；
- 当前区域停滞；
- 分区大幅变化；
- 谱结果连续超时。

行为：暂时关闭区域偏好，使用原始 EDEN 脱困。

---

# 12. 修订后的核心伪代码

```cpp
GlobalPlanStatus MultiDtgPlus::PlanSpectralEdenV2(
    const Eigen::Vector3d& robot_pos,
    std::vector<h_ptr>& route_h) {

  // 1. 更新 frontier 生命周期和区域归属
  UpdateFrontierRuntimeStates();
  ReassignFrontierOwners();
  DetectAndHandleRegionStall();

  // 2. 提取真正有效的活跃边界区域
  auto active = CollectEffectiveBoundaryRegions(robot_pos);

  if (active.empty()) {
    return CheckMapCompletion();
  }

  // 3. 探索末期关闭谱模块
  if (IsLateStage(active)) {
    spectral_mode_ = SpectralMode::DISABLED;
    return BuildEOHDTRoute(active, route_h);
  }

  // 4. 按事件和预算决定是否重算谱
  if (NeedSpectralUpdate(active)) {
    SubmitSpectralJobAsync(active);
  }

  // 5. 消费异步结果，更新稳定分区
  ConsumeSpectralResult();
  UpdatePartitionConfidence();
  UpdatePersistentRegions();

  // 6. 始终生成原始 EDEN 路线
  std::vector<h_ptr> eden_route;
  BuildEOHDTRoute(active, eden_route);

  // 7. 没有稳定分区时直接使用 EDEN
  if (spectral_mode_ != SpectralMode::ACTIVE_SOFT) {
    route_h = eden_route;
    return GlobalPlanStatus::SUCCESS;
  }

  // 8. 生成区域感知候选路线
  std::vector<h_ptr> region_route;
  BuildRegionAwareRoute(active, region_route);

  // 9. 路径后悔保护
  if (!AcceptRegionRoute(eden_route, region_route)) {
    route_h = eden_route;
    AccumulateLockDebt(region_route, eden_route);
    return GlobalPlanStatus::SUCCESS;
  }

  // 10. 应用软区域偏好，而不是过滤所有跨区目标
  route_h = ApplySoftRegionPreference(region_route, active);

  // 11. 绕路债务超限时恢复
  if (lock_debt_ > config_.lock_debt_max) {
    EnterRecovery();
    route_h = eden_route;
  }

  return GlobalPlanStatus::SUCCESS;
}
```

---

# 13. 推荐代码修改位置

## 13.1 `mr_dtg_plus_structures.h`

新增：

```cpp
struct SpectralConfig;
struct SpectralSnapshot;
struct SpectralResult;
struct PersistentRegion;
struct FrontierRuntimeState;

enum class SpectralMode;
enum class FrontierState;
enum class RegionExecState;
enum class RecoveryReason;
```

---

## 13.2 `mr_dtg_plus.h`

新增：

```cpp
bool NeedSpectralUpdate(
    const std::vector<h_ptr>& active_hnodes) const;

bool IsLateStage(
    const std::vector<h_ptr>& active_hnodes) const;

void SubmitSpectralJobAsync(
    const std::vector<h_ptr>& active_hnodes);

void ConsumeSpectralResult();

double ComputePartitionConfidence(
    const SpectralResult& result) const;

void UpdatePersistentRegions(
    const SpectralResult& result);

bool BuildRegionAwareRoute(
    const std::vector<h_ptr>& active_hnodes,
    std::vector<h_ptr>& route);

bool AcceptRegionRoute(
    const std::vector<h_ptr>& eden_route,
    const std::vector<h_ptr>& region_route);

void UpdateFrontierRuntimeStates();

void ReassignFrontierOwners();

bool DetectAndHandleRegionStall();

bool HasEffectiveFrontier(
    const h_ptr& h,
    int region_id = -1) const;
```

---

## 13.3 `mr_dtg_plus_path_search.cpp`

保留原始 EOHDT，封装为：

```cpp
BuildEOHDTRoute(...)
```

将：

```cpp
if (!hc->hf_edges_.empty())
```

修改为：

```cpp
if (HasEffectiveFrontier(hc, -1))
```

---

## 13.4 新增 `mr_dtg_plus_spectral.cpp`

实现：

```text
BuildSpectralSnapshot
CompressDegreeTwoChains
BuildWeightedSparseGraph
SolveFiedler
AlignFiedlerSign
SweepNcut
ComputePartitionConfidence
MatchPersistentRegions
BuildRegionAwareRoute
```

---

## 13.5 `eden.cpp / eden_fsm.cpp`

区分：

```cpp
SUCCESS
NO_ACTIVE_FRONTIER
MAP_FINISHED
FRONTIER_STALLED
SPECTRAL_TIMEOUT
SPECTRAL_UNSTABLE
ROUTE_REJECTED_BY_REGRET
PATH_FAILED
```

谱分区关闭、谱求解失败或候选路线被拒绝，都不能导致 UAV 停止，应回退原始 EDEN。

---

# 14. 推荐参数初值

```yaml
SpectralV2:
  enabled: true

  # 图规模与更新
  min_spectral_nodes: 10
  max_spectral_nodes: 80
  spectral_knn: 5
  spectral_min_update_interval: 0.5
  spectral_max_update_interval: 3.0
  dirty_node_changes: 3
  dirty_edge_changes: 5
  spectral_time_budget_ms: 5.0

  # 分区置信度
  confidence_weight_ncut: 0.30
  confidence_weight_eigengap: 0.25
  confidence_weight_relative_lambda2: 0.15
  confidence_weight_bottleneck: 0.20
  confidence_weight_balance: 0.10
  partition_confidence_on: 0.70
  partition_confidence_off: 0.40
  partition_on_persistence: 3
  partition_off_persistence: 5
  partition_grace_epochs: 8

  # 标签稳定
  label_hysteresis_ratio: 0.08
  region_match_threshold: 0.50

  # 路线保护
  max_route_regret: 0.05
  neighbor_override_distance: 2.0
  neighbor_override_margin: 0.5
  lock_debt_decay: 0.90
  lock_debt_max: 8.0
  recovery_duration: 5.0

  # 探索末期
  late_stage_frontier_count: 6
  late_stage_reactivate_frontier_count: 12
  late_stage_total_gain: 50
  late_stage_no_cut_epochs: 10

  # Frontier 清理
  frontier_low_gain_cycles: 2
  frontier_actual_gain_eps: 10
  frontier_quarantine_time: 5.0
  region_done_confirm_cycles: 3
  region_stall_window: 8.0
  region_stall_timeout: 12.0
  repeat_target_limit: 3
```

---

# 15. 必做消融实验

## 15.1 模块级消融

| 配置 | 内容 | 目的 |
|---|---|---|
| B0 | 原始 EDEN | 基线 |
| B1 | 原硬锁 Spectral-EDEN | 复现实验问题 |
| B2 | 谱分区 + 软区域偏好 | 验证硬锁是否为路径增长主因 |
| B3 | B2 + 路径后悔上限 | 验证路径增长是否受控 |
| B4 | B3 + 双阈值滞回和持久区域 ID | 验证分区震荡改善 |
| B5 | B4 + 事件驱动和图压缩 | 验证计算开销改善 |
| B6 | B5 + 末期自动关闭 | 验证探索末期收益 |
| B7 | B6 + 有效 frontier 和停滞恢复 | 验证局部转圈改善 |
| B8 | 完整 Spectral-EDEN v2 | 最终方案 |

---

## 15.2 参数级消融

### 路径保护

```text
max_route_regret ∈ {0, 0.03, 0.05, 0.08, 0.10}
lock_debt_max ∈ {3, 5, 8, 12, 20} m
neighbor_override_distance ∈ {1, 2, 3, 5} m
```

指标：

- 总路径长度；
- 区域提前切换次数；
- 回访距离；
- 总探索时间。

### 时间稳定

```text
on_persistence ∈ {1, 2, 3, 5}
off_persistence ∈ {2, 3, 5, 8}
grace_epochs ∈ {0, 3, 5, 8, 12}
```

指标：

- 分区启停次数；
- 区域 ID 切换次数；
- 首目标改变次数；
- BFM 数量。

### 计算控制

```text
max_spectral_nodes ∈ {40, 60, 80, 100}
update_interval ∈ {0.2, 0.5, 1.0, 2.0} s
time_budget ∈ {2, 5, 10, 20} ms
```

指标：

- 谱计算平均/P95/P99 时间；
- 累计谱计算开销；
- 总探索时间；
- fallback 次数。

### 末期关闭

```text
late_frontier_count ∈ {3, 6, 10, 15}
no_cut_epochs ∈ {5, 10, 20}
```

指标：

- 最后 10% 探索阶段耗时；
- 后期谱求解次数；
- 完成时间；
- 残留 frontier 处理成功率。

---

# 16. 新增评估指标

## 16.1 路线后悔率

$$
R_{\mathrm{path}}
=
\frac{
C_{\mathrm{selected}}-C_{\mathrm{EDEN}}
}{
C_{\mathrm{EDEN}}+\varepsilon
}
$$

报告平均值、P95 和超过阈值次数。

---

## 16.2 分区震荡次数

$$
N_{\mathrm{toggle}}
=
\sum_t
\mathbf 1[
\operatorname{active}(t)
\neq
\operatorname{active}(t-1)
]
$$

---

## 16.3 区域标签变化率

$$
R_{\mathrm{label}}
=
\frac{
\sum_i
\mathbf 1[r_i(t)\neq r_i(t-1)]
}{
|V_t\cap V_{t-1}|
}
$$

---

## 16.4 谱有效利用率

$$
U_{\mathrm{spectral}}
=
\frac{
N_{\mathrm{spectral\ route\ accepted}}
}{
N_{\mathrm{spectral\ solves}}
}
$$

若求解次数很多，但区域路线很少被接受，说明谱计算没有实际贡献。

---

## 16.5 计算收益比

$$
E_{\mathrm{compute}}
=
\frac{
T_{\mathrm{EDEN}}-T_{\mathrm{method}}
}{
C_{\mathrm{spectral,total}}
}
$$

表示每增加 1 秒谱计算，实际节省了多少探索时间。

---

## 16.6 Frontier 与停滞指标

- `region_stall_count`
- `frontier_quarantine_count`
- `frontier_reassignment_count`
- `repeat_target_count`
- `recovery_success_rate`

---

# 17. 六个问题与改进措施对应表

| 原问题 | 对应改进 |
|---|---|
| 区域硬锁导致路径更长 | 软区域偏好、邻区例外、EOHDT 双路线比较、5% 路径后悔上限、绕路债务 |
| 分区反复激活和消失 | 连续置信度、双阈值、不同 on/off persistence、宽限期、持久区域 ID |
| 计算开销非线性增长 | 事件驱动、异步求解、相关支撑子图、走廊链压缩、节点上限、时间预算 |
| 大部分场景不触发 | 不要求所有地图分区；无明显瓶颈时直接使用原 EDEN |
| 固定 $\lambda_2$ 与 Ncut 阈值不适配 | 相对 $\lambda_2$、相对 eigengap、多指标连续置信度 |
| 探索末期谱方法退化 | 在线末期检测、自动关闭谱求解、原 EDEN 接管残余目标 |
| 残留 frontier 造成局部转圈 | 有效 frontier、动态归属、实际信息增益、隔离机制、停滞恢复 |

---

# 18. 最终建议

下一版 Spectral-EDEN 不应继续采用：

```text
检测到分区
→ 强制锁定当前区域
→ 当前区域所有 frontier 清空后才切换
```

应改为：

```text
检测到可靠区域结构
→ 将区域关系作为软先验
→ 同时计算原 EDEN 与区域感知路线
→ 路径增量受后悔上限约束
→ 分区状态经过时间稳定器
→ 谱计算按事件、预算和探索阶段启停
→ 残留 frontier 不再无限阻止区域结束
```

最终算法的核心判断应是：

$$
\boxed{
\text{谱策略只有在完整探索不受影响、}
\text{预计总收益为正且实时预算允许时才介入}
}
$$

这比“分区后必须穷尽当前区域”更符合你的研究目标：

$$
\boxed{
\text{在完整探索地图的前提下，}
\text{尽可能缩短探索时间、路径长度并降低计算量}
}
$$
