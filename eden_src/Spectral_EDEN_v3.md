# Spectral-EDEN v2 源码核查后的纠正方案

> **核查对象**：用户上传的 `Spectral_EDEN_code.zip`  
> **核心源码目录**：
>
> - `Mapping_mr_dtg_plus`
> - `Exploration_eden`
>
> **结论**：前一版建议中有几项需要纠正。当前代码并非“谱求解完全同步”“仍然使用硬区域锁”或“按照 Fiedler 坐标直接对全部 frontier 排序”。真正导致当前故障的核心问题主要集中在：轨迹时间参数错误、轨迹输入输出缺少有限数检查、完成判断调用位置错误、低 expected gain 被当作 frontier 失效条件、重复规划被错误解释为重复探索，以及主线程仍承担谱快照构建工作。

---

# 1. 先纠正此前不准确的判断

## 1.1 `async_solve` 不是完全没有异步

实际代码：

```cpp
// Mapping_mr_dtg_plus/src/mr_dtg_plus_spectral.cpp
spectral_future_ = std::async(std::launch::async, ...);
```

位置：`mr_dtg_plus_spectral.cpp:980-992`

结果读取使用：

```cpp
spectral_future_.wait_for(std::chrono::milliseconds(0));
```

只有结果已经 ready 时才调用 `get()`。

位置：`mr_dtg_plus_spectral.cpp:1047-1055`

因此：

> `SpectralRouter::Solve()` 确实在异步线程中执行，不能再把当前问题简单归因于 `future.get()` 阻塞主线程。

但异步仍然只完成了一半，因为：

```cpp
BuildSpectralGraphSnapshot(context, snapshot, reason)
```

是在调用 `std::async` 之前、主规划线程中执行的。

位置：`mr_dtg_plus_spectral.cpp:935-947`

所以 City 中较高的规划开销更可能来自：

- 活跃区域收集；
- DTG 距离维护；
- 谱支撑子图构建；
- 支撑路径扩展；
- 走廊压缩；
- EOHDT 基线构造；
- 区域感知完整路线构造和评估；

而不只是异步线程中的特征值求解。

### 额外确认的配置问题

参数：

```yaml
MR_DTG/SpectralV2/async_solve: true
```

虽然在 `mr_dtg_plus_initialization.cpp:77` 被读取，但之后没有任何代码使用 `spectral_exec_config_.async_solve`。

也就是说：

> 当前 `async_solve` 是一个无效配置项。无论设置为 `true` 还是 `false`，代码都会调用 `std::async(std::launch::async, ...)`。

应当删除该参数，或者真正实现同步/异步分支，避免配置文件产生误导。

---

## 1.2 当前 V2 没有使用传统硬区域锁

初始化代码明确执行：

```cpp
// V2 never uses the legacy hard regional filter.
spectral_exec_config_.region_lock_enabled = false;
```

位置：`mr_dtg_plus_initialization.cpp:262-263`

因此，不能继续将 V2 的路径增长描述为“硬锁仍在强制阻止跨区”。

V2 当前实际采用的是：

- active region；
- 动态 switch penalty；
- 区域感知贪心路线；
- 路径 regret 上限；
- lock debt 事后恢复；
- 局部视点评分中的区域惩罚。

它仍可能产生区域偏置，但不是原 V1 的绝对硬锁。

---

## 1.3 当前路线不是按 Fiedler 数值全局排序

`BuildRegionAwareRoute()` 的真实逻辑是：

```text
从当前机器人或上一节点出发
→ 遍历剩余所有 anchor
→ 计算 DTG 距离 + 跨区域 switch penalty
→ 选择得分最低的节点
→ 重复直到全部节点排序完成
```

位置：`mr_dtg_plus_spectral.cpp:1291-1382`

因此，正确表述是：

> V2 使用“带区域切换惩罚的最近邻贪心路线”，而不是直接按照 Fiedler 坐标升序访问全部 frontier。

不过，这个完整贪心重排仍会改变原 EOHDT 的访问顺序，所以路径增长问题依然成立，只是根因需要改正。

---

## 1.4 谱求解器内部已经有较完整的数值防线

`spectral_router.cpp` 已经包含：

- 配置有限性检查；
- 边长度、净空和介数检查；
- degree 有限性和最小度检查；
- normalized Laplacian 有限性检查；
- `SelfAdjointEigenSolver::info()`；
- 特征值和特征向量 `allFinite()`；
- 迭代求解残差检查；
- Fiedler 残差检查；
- Ncut 分母和结果检查。

因此，前一版建议中“在谱求解器各处补充所有 finite guard”已经不是最高优先级。

当前 `inf` 更需要优先排查：

1. EDEN轨迹时间输入；
2. 运动增益公式；
3. 视点和走廊输入；
4. 轨迹优化器返回结果；
5. 轨迹发布。

---

# 2. P0：已经确认的轨迹时间 Bug

## 2.1 `TrajCheck()` 使用了错误的时间坐标

文件：

```text
Exploration_eden/src/eden.cpp
```

问题代码：

```cpp
double cur_t = max(ros::WallTime::now().toSec(), traj_start_t_);
...
Eigen::Vector3d last_p = exc_traj_.getPos(cur_t).head(3);
```

位置：`eden.cpp:206-217`

`cur_t` 是绝对 wall time，例如十亿秒量级；但 `exc_traj_.getPos()` 需要的是相对于轨迹起点的时间。

同一函数后面的循环使用的却是：

```cpp
exc_traj_.getPos(t - traj_start_t_)
```

位置：`eden.cpp:221-222`

这证明第217行应当使用相对时间。

### 必须修改为

```cpp
const double total_duration = exc_traj_.getTotalDuration();
const double rel_cur_t = std::clamp(
    cur_t - traj_start_t_,
    0.0,
    std::max(0.0, total_duration - 1.0e-6));

Eigen::Vector4d last_state = exc_traj_.getPos(rel_cur_t);

if(!last_state.allFinite()){
    ROS_ERROR("[EDEN] non-finite trajectory state in TrajCheck");
    dtg_flag_ = false;
    return false;
}

Eigen::Vector3d last_p = last_state.head<3>();
```

这个 Bug 与“RViz中UAV消失、规划器仍继续运行”的现象高度相关，应作为第一修复项，而不是先继续调整谱参数。

---

# 3. P0：轨迹优化成功后没有验证结果是否合法

## 3.1 当前调用链

分支轨迹：

```cpp
if(TrajOpt_.AggressiveBranchTrajOptimize(toi)){
    exc_traj_ = TrajOpt_.stem_traj_;
    ...
    PublishTraj();
    return EdenPlanStatus::SUCCESS;
}
```

位置：`eden.cpp:595-660`

普通轨迹：

```cpp
if(TrajOpt_.NormTrajOptimize(toi)){
    exc_traj_ = TrajOpt_.norm_traj_;
    ...
    PublishTraj();
    return EdenPlanStatus::SUCCESS;
}
```

位置：`eden.cpp:676-708`

`PublishTraj()` 又直接复制：

- 每段 duration；
- 位置多项式系数；
- yaw多项式系数；

没有任何 `std::isfinite()` 或 `allFinite()` 检查。

位置：`eden.cpp:1015-1042`

因此，只要轨迹优化器错误地返回 `true`，即使内部系数包含 `inf`，系统也会：

```text
覆盖当前轨迹
→ 更新时间戳
→ 发布非法轨迹
→ 返回SUCCESS
```

这与你观察到的故障链完全一致。

---

## 3.2 增加统一轨迹验证函数

建议在 `eden.h` 增加：

```cpp
bool ValidateTrajectory(
    const Trajectory<4> &trajectory,
    std::string &reason) const;
```

根据项目实际轨迹类型调整模板名。

实现至少检查：

```cpp
bool SingleExp::ValidateTrajectory(
    const Trajectory<4> &traj,
    std::string &reason) const {

    const double total_duration = traj.getTotalDuration();

    if(!std::isfinite(total_duration) || total_duration <= 1.0e-6){
        reason = "invalid total duration";
        return false;
    }

    if(traj.getPieceNum() <= 0){
        reason = "trajectory contains no piece";
        return false;
    }

    for(int i = 0; i < traj.getPieceNum(); ++i){
        const auto &piece = traj[i];
        const double duration = piece.getDuration();
        const Eigen::MatrixXd coeff = piece.getCoeffMat();

        if(!std::isfinite(duration) || duration <= 1.0e-6){
            reason = "invalid piece duration";
            return false;
        }

        if(!coeff.allFinite()){
            reason = "trajectory coefficient contains NaN or Inf";
            return false;
        }
    }

    for(double t = 0.0; t <= total_duration; t += 0.05){
        if(!traj.getPos(t).allFinite() ||
           !traj.getVel(t).allFinite() ||
           !traj.getAcc(t).allFinite()){
            reason = "sampled trajectory state contains NaN or Inf";
            return false;
        }
    }

    return true;
}
```

---

## 3.3 发布之前再验证一次

分支轨迹：

```cpp
if(TrajOpt_.AggressiveBranchTrajOptimize(toi)){
    const auto candidate = TrajOpt_.stem_traj_;

    std::string reason;
    if(!ValidateTrajectory(candidate, reason)){
        ROS_ERROR_STREAM(
            "[EDEN] rejected invalid branch trajectory: " << reason);
        return EdenPlanStatus::TRAJECTORY_FAILED;
    }

    exc_traj_ = candidate;
    ...
}
```

普通轨迹同样处理。

`PublishTraj()` 内部还应再执行轻量检查，构成最后一道防线。

---

## 3.4 不要先覆盖旧轨迹再验证

必须采用：

```text
candidate trajectory
→ 校验
→ 校验成功后赋值给exc_traj_
```

不能：

```text
先覆盖exc_traj_
→ 再发现非法
```

否则旧的安全轨迹会被破坏。

---

# 4. P0：轨迹优化输入也缺少有限数检查

`EdenPlan()` 会从当前轨迹采样：

```cpp
p4s = exc_traj_.getPos(...)
v4s = exc_traj_.getVel(...)
a4s = exc_traj_.getAcc(...)
```

位置：`eden.cpp:303-348`

随后直接写入：

```cpp
toi.initState
```

位置：`eden.cpp:364-371`

但代码没有确认：

- `p4s`有限；
- `v4s`有限；
- `a4s`有限；
- yaw有限；
- corridor矩阵有限；
- endState有限。

如果上一周期轨迹已经被污染，下一周期会继续将非法状态送入优化器。

### 必须增加

```cpp
if(!ps.allFinite() ||
   !vs.allFinite() ||
   !as.allFinite() ||
   !std::isfinite(ys) ||
   !std::isfinite(yds) ||
   !std::isfinite(ydds)){
    ROS_ERROR("[EDEN] invalid initial state for trajectory optimization");
    return EdenPlanStatus::TRAJECTORY_FAILED;
}
```

同时在调用优化器前检查：

```text
toi.initState
toi.endState_stem
toi.endState_main
toi.endState_sub
toi.endState_norm
所有corridor矩阵
```

---

# 5. P0：运动增益公式存在实际 NaN/Inf 风险

文件：

```text
Mapping_mr_dtg_plus/include/mr_dtg_plus/mr_dtg_plus.h
```

函数：

```cpp
GetGainExp1()
GetGainExp2()
```

位置：`mr_dtg_plus.h:1509-1590`

## 5.1 风险一：零向量归一化

```cpp
(vp.head(3) - ps.head(3)).normalized()
```

当目标位置和起点重合时会产生非有限数。

## 5.2 风险二：`acos()` 输入未夹紧

```cpp
acos(dp.dot(vs.normalized()))
```

浮点误差可能使点积略大于1或略小于-1，从而产生NaN。

必须：

```cpp
const double cosine = std::clamp(
    dp.dot(velocity_direction), -1.0, 1.0);
```

## 5.3 风险三：除以 `sin(det_theta/2)`

```cpp
r = distance / sin(det_theta * 0.5)
```

当方向几乎相同或完全相反时，分母可能接近0。

## 5.4 风险四：后续连锁除零

```cpp
sqrt(a_max_ * lambda_a_ / r)
det_theta / omiga
dist / v1_max
yaw_diff / yv_max_
```

任何参数或中间量接近0都可能产生NaN/Inf。

---

## 5.5 建议处理原则

不要用“发现非有限值后简单设为0”的方式掩盖错误。

应当：

```text
输入非法
→ 返回gain=0，拒绝该候选

方向差很小
→ 使用直线运动极限，a_cost=0，v1_max=v_max

曲率半径或omega非法
→ 使用保守直线时间估计，或拒绝候选
```

每次返回前：

```cpp
if(!std::isfinite(t) || t < 0.0){
    return 0.0;
}

const double exponent = -lambda_e_ * t;

if(!std::isfinite(exponent)){
    return 0.0;
}

return std::exp(std::clamp(exponent, -700.0, 0.0));
```

这部分比继续加强 `spectral_router.cpp` 的有限数检查更重要，因为谱求解器本身已经有较完整检查。

---

# 6. P0：体积覆盖终止位置确认错误

## 6.1 当前代码只在全局规划失败后检查完成

位置：`eden.cpp:392-446`

当前结构：

```cpp
if(last_global_plan_status_ != SUCCESS){
    if(last_global_plan_status_ == NO_ACTIVE_FRONTIER){
        // 才计算volume_covered
    }
}
```

所以只要 `PlanGlobalRoute()` 持续返回 `SUCCESS`，即使地图覆盖率已经超过99%，终止检查也不会执行。

这确认了 Maze 99.7% 无法结束的根因。

---

## 6.2 City与Maze必须分开解释

Maze：

```text
3160 / 3170 ≈ 99.7%
```

超过99%，属于终止调用路径错误。

City：

```text
10660 / 11200 ≈ 95.2%
```

没有达到99%，因此不能用体积阈值直接终止。City属于“仍未完成但目标被过滤，导致停滞”。

---

## 6.3 将已知体积完成检查放在规划入口

在调用 `TargetPlanning()` 之前执行：

```cpp
bool SingleExp::VolumeCoverageReached() {
    if(!volume_coverage_enabled_ ||
       !stat_ ||
       total_explorable_volume_ <= 0.0){
        volume_finish_stable_since_ = -1.0;
        return false;
    }

    const double volume = CS_.GetVolume(0);
    const double ratio = volume / total_explorable_volume_;

    if(!std::isfinite(ratio)){
        volume_finish_stable_since_ = -1.0;
        return false;
    }

    const double now = ros::WallTime::now().toSec();

    if(ratio >= volume_coverage_threshold_){
        if(volume_finish_stable_since_ < 0.0){
            volume_finish_stable_since_ = now;
        }

        return now - volume_finish_stable_since_
            >= finish_stable_duration_;
    }

    volume_finish_stable_since_ = -1.0;
    return false;
}
```

在 `EdenPlan()` 中：

```cpp
if(VolumeCoverageReached()){
    ROS_WARN("Known-volume coverage threshold reached");
    return EdenPlanStatus::FINISHED;
}
```

---

## 6.4 不要让体积完成依赖 `low_total_gain`

当前：

```cpp
finish_candidate = sensor_fresh && low_total_gain &&
    ((no_effective_frontier && no_pending_region) || volume_covered);
```

即便 `volume_covered=true`，仍要求 `low_total_gain=true`。

如果存在陈旧frontier，Maze仍可能被阻塞。

对于仿真中已知准确总体积的终止条件，建议：

```text
volume_covered并稳定
→ 独立终止
```

而未知地图才使用：

```text
无可达frontier
+ 无pending region
+ 低剩余信息增益
+ 传感器新鲜
```

---

## 6.5 当前确认计数还有一个隐患

当前 `finish_confirm_count_` 只有在：

```cpp
frontier_epoch != last_finish_frontier_epoch_
```

时才增加。

位置：`eden.cpp:414-418`

如果地图已经稳定、frontier epoch不再变化，计数可能永远不会达到阈值。

体积覆盖应使用独立的时间稳定器或每规划周期确认计数，而不是依赖frontier epoch变化。

---

# 7. P1：当前异步性能问题的准确修复方向

## 7.1 不要删除现有 `std::launch::async`

当前求解部分确实异步，保留即可。

首先修复无效配置：

```cpp
if(spectral_exec_config_.async_solve){
    // 当前std::launch::async路径
}
else{
    // 明确同步求解，或直接删除async_solve参数
}
```

更推荐删除该参数并始终异步，减少配置分支。

---

## 7.2 不要直接把整个 `BuildSpectralGraphSnapshot()` 丢进worker

该函数会读取：

- `H_list_`；
- `h_ptr`；
- DTG边；
- LRM和地图相关数据。

如果直接在后台读取这些可变对象，可能引入数据竞争。

正确拆分：

### 主线程

只复制最小不可变原始数据：

```cpp
struct RawSpectralSnapshot {
    uint64_t dtg_version;
    uint64_t frontier_version;
    std::vector<RawNode> nodes;
    std::vector<RawEdge> edges;
    std::unordered_set<uint32_t> active_anchor_ids;
};
```

该步骤只做线性复制，不进行：

- 走廊压缩；
- Ncut；
- confidence；
- 特征求解；
- 路线构造。

### worker线程

处理：

```text
support graph构建
走廊压缩
边权计算
特征求解
Ncut sweep
confidence evidence
```

### 主线程消费

只执行：

```text
版本检查
结果合法性检查
模式状态更新
```

---

## 7.3 当前5ms预算不能减少已经发生的计算

代码在快照构建完成后才判断：

```cpp
if(build_ms > spectral_time_budget_ms)
```

位置：`mr_dtg_plus_spectral.cpp:959-971`

异步求解完成后又判断：

```cpp
if(snapshot_build_ms + solve_ms > budget)
```

位置：`mr_dtg_plus_spectral.cpp:1077-1091`

这只能：

```text
拒绝超预算结果
```

不能追回已经耗费的CPU时间。

因此，5ms预算目前是“结果接纳预算”，不是“真实执行上限”。

建议拆成：

```yaml
main_thread_snapshot_budget_ms: 3.0
worker_warning_budget_ms: 20.0
over_budget_limit: 3
cooldown_duration: 10.0
```

连续超预算时暂停提交新任务，而不是每0.5秒继续算完再丢弃。

---

## 7.4 先测量各阶段，而不是只记录总 solve_ms

新增：

```text
collect_active_ms
distance_matrix_ms
raw_snapshot_copy_ms
support_graph_ms
compression_ms
solver_ms
ncut_ms
confidence_ms
baseline_route_ms
candidate_route_ms
path_to_first_ms
local_target_ms
trajectory_ms
```

这样才能确认 City 的227ms究竟耗在哪个函数中。

---

# 8. P1：路径增长的真实根因和修复

## 8.1 `max_route_regret: 0.05` 本身允许5%路径增长

City和Maze配置均为：

```yaml
MR_DTG/SpectralV2/max_route_regret: 0.05
```

因此，算法设计上就允许预测路径比EOHDT长5%。

Maze实测增长5.2%并不意外，因为：

- 预测距离不是实际飞行距离；
- 轨迹曲率和yaw没有进入全局路径估计；
- 距离矩阵缺失时使用近似上界；
- 路线在后续规划周期会再次变化。

---

## 8.2 距离缺失时使用root-star上界

位置：`mr_dtg_plus_spectral.cpp:1206-1212`

```cpp
return root_distance(from) + root_distance(to);
```

这个值是有效上界，但不一定接近真实两anchor最短距离。

它会影响：

- EOHDT路线评估；
- 区域候选路线评估；
- route regret；
- switch/revisit判断。

所以5%的“预测后悔”不能保证最终实际路径只增加5%。

---

## 8.3 lock debt只能事后恢复，不能阻止第一次绕路

代码先接受候选路线，再更新lock debt：

```cpp
UpdateLockDebt(...)
```

位置：`mr_dtg_plus_global.cpp:692-705`

即使最终进入recovery，之前接受的绕路已经发生。

因此，当前阶段建议：

```yaml
max_route_regret: 0.00
lock_debt_max: 0.0
```

并禁用完整区域感知路线替换。

---

## 8.4 推荐改为TOP-K首目标建议

完整路线继续使用：

```cpp
baseline_route = RunEohdtFallback(...)
```

谱模块只从EOHDT前3个节点中选择首目标：

```text
EOHDT前3个候选
→ 使用精确robot-to-anchor路径
→ 加入很小的区域切换惩罚
→ 只改变首目标
→ 其余顺序保持EOHDT
```

接受条件：

```text
候选首目标距离
≤
EOHDT原首目标距离 × 1.02
```

第一轮实验甚至可以设置：

```yaml
top_k: 1
```

也就是谱模块只观察、不改变路线，确认计算和终止恢复后再逐步开放。

---

# 9. P2：City停滞的源码根因已经确认

## 9.1 低 expected gain 被当作“frontier无效”

以下位置都会直接排除：

```cpp
frontier.unknown_num_ <= frontier_expected_gain_eps
```

### 位置一

`EffectiveFrontierCount()`：

```text
mr_dtg_plus_spectral.cpp:1385-1408
```

### 位置二

`HasEffectiveFrontier()`：

```text
mr_dtg_plus_spectral.cpp:1431-1455
```

### 位置三

`ReassignFrontierOwners()`：

```text
mr_dtg_plus_spectral.cpp:1544-1559
```

### 位置四

`FindFastExpTarget()`中的 `frontier_is_effective`：

```text
mr_dtg_plus_path_search.cpp:812-821
```

所以，只要探索末期所有frontier的 `unknown_num_ <= 1`：

```text
有效frontier数量变成0
→ anchor被移除
→ 本地目标也被移除
→ 探索停止
```

---

## 9.2 expected gain还会直接触发quarantine

位置：`mr_dtg_plus_spectral.cpp:1526-1538`

当前逻辑：

```text
unknown_num <= 1
→ low_gain_streak增加
→ SUSPECT
→ 两个周期后QUARANTINED
```

这没有要求：

- UAV实际到达；
- 真实信息增益为0；
- 强视点检查失败；
- 路径不可达。

这正是City停滞的最直接代码原因。

---

## 9.3 必须修改

`frontier_expected_gain_eps` 不再作为有效性判断。

删除或取消以下函数中的该阈值过滤：

```text
EffectiveFrontierCount
HasEffectiveFrontier
ReassignFrontierOwners
FindFastExpTarget/frontier_is_effective
EstimateReturnProbability
```

删除 `UpdateFrontierRuntimeStates()` 中仅凭expected gain quarantine的代码块：

```cpp
if(frontier.unknown_num_ <= frontier_expected_gain_eps){
    ...
    runtime.state = QUARANTINED;
}
```

expected gain只用于排序，例如：

```cpp
const double gain_scale =
    frontier.unknown_num_ <= frontier_expected_gain_eps
        ? low_expected_gain_scale
        : 1.0;

candidate_gain *= gain_scale;
```

建议：

```yaml
low_expected_gain_scale: 0.25
```

低收益目标仍可被选中，只是优先级更低。

---

# 10. P2：重复目标判断也存在逻辑错误

## 10.1 当前按“重复被规划”计数，而不是“重复到达”

位置：`mr_dtg_plus_spectral.cpp:1591-1614`

只要同一个frontier每隔0.5秒再次被选中：

```cpp
++runtime.repeat_count;
```

达到3次就quarantine。

但EDEN会在UAV飞向远处目标期间持续重规划。同一个合理目标可能在尚未到达前被连续选中3次。

于是：

```text
正常持续追踪同一目标
→ 被判断为重复目标
→ quarantine
→ 进入recovery
```

---

## 10.2 更严重的是记录发生在轨迹成功之前

`RecordSelectedFrontier(best_tar1.first)` 位于：

```text
mr_dtg_plus_path_search.cpp:1378
```

此时：

- corridor还没有完成验证；
- 轨迹优化还没有执行；
- 轨迹可能失败；
- 目标可能最终没有被发布。

因此，这是“计划选择次数”，不是“实际探索次数”。

---

## 10.3 修复方式

第一阶段直接禁用：

```text
repeat_count触发quarantine
```

保留repeat count仅用于日志。

将“目标执行开始”记录移动到：

```text
轨迹优化成功
+ ValidateTrajectory成功
+ PublishTraj之前
```

同时，quarantine只能由以下证据触发：

```text
已经执行该目标
AND 经过足够飞行时间或到达目标附近
AND actual unknown reduction连续过低
AND StrongCheckViewpoint失败或多次路径失败
```

---

# 11. P2：late-stage判断存在自我强化

`IsLateStage()` 使用：

```cpp
EffectiveFrontierCount()
```

位置：`mr_dtg_plus_spectral.cpp:1412-1428`

而 `EffectiveFrontierCount()` 已经排除了：

- 低expected gain；
- quarantined frontier。

所以会形成：

```text
低gain被过滤
→ effective count下降
→ late stage激活
→ 谱模块关闭
→ 但frontier仍然被过滤
→ 没有目标可以继续探索
```

late-stage关闭谱计算是正确方向，但它不能修复被过滤的frontier。

---

## 11.1 应拆成两个计数

### RawReachableFrontierCount

只检查：

```text
f_state有效
至少一个有效视点
存在有限可达路径
```

不检查：

- expected gain；
- region owner；
- quarantine。

### ActionableFrontierCount

用于正常目标选择，可以考虑状态和优先级。

late-stage必须使用：

```text
RawReachableFrontierCount
+
raw total unknown
```

而不能使用过滤后的actionable数量。

---

# 12. P2：增加fail-open恢复

当：

```text
ActionableFrontierCount == 0
RawReachableFrontierCount > 0
地图未达到体积完成阈值
```

执行一次恢复：

```text
1. spectral mode进入RECOVERY
2. active_region_id设为-1
3. 暂停区域惩罚
4. 将最接近的SUSPECT/QUARANTINED frontier临时恢复
5. 使用原EOHDT重新规划
```

不能继续：

```text
NO_ACTIVE_FRONTIER
→ 完成条件不满足
→ RETRY
→ 再次NO_ACTIVE_FRONTIER
```

这就是当前City可能永久停滞的循环。

---

# 13. 日志问题需要纠正

V2并不是完全没有诊断信息。

`PublishGlobalPlanDiagnostics()` 已经发布：

```text
/<node_name>/MR_DTG/SpectralDiagnostics
```

包含：

- lambda2/lambda3；
- eigengap；
- ncut；
- solve_ms；
- confidence；
- route regret；
- lock debt；
- frontier数量；
- timeout/stale次数；
- solver类型等。

位置：`mr_dtg_plus_global.cpp:42-117`

事件话题：

```text
/<node_name>/MR_DTG/SpectralEvents
```

但终端没有每epoch的 `ROS_INFO`，所以只看控制台时会显得“完全静默”。

建议保留话题，并增加：

```cpp
ROS_INFO_THROTTLE(
    1.0,
    "[SpectralV2] mode=%d nodes=%zu rawF=%zu actionF=%zu "
    "snapshot=%.2fms solve=%.2fms ncut=%.3f conf=%.3f "
    "route=%d regret=%.3f volume=%.1f fallback=%s",
    ...);
```

---

# 14. 其他必须清理的问题

## 14.1 运行路径中的 `getchar()`

`eden.cpp` 中仍有多个实际执行的 `getchar()`，例如：

```text
eden.cpp:501-567
```

一旦路径或corridor异常，ROS节点会等待键盘输入，造成无人机和规划器看似“卡死”。

所有运行时 `getchar()` 必须替换成：

```text
ROS_ERROR
→ 返回明确错误状态
→ 保留旧安全轨迹或执行hold
```

---

## 14.2 `SpectralGainMultiplier()`增加最后保护

位置：`mr_dtg_plus_spectral.cpp:1699-1753`

该函数通常只会返回0到1，但仍应检查：

```cpp
if(!std::isfinite(travel_distance) ||
   !std::isfinite(lambda_e_) ||
   lambda_e_ < 0.0){
    return 1.0;
}
```

结果非有限时返回1.0，表示“不让谱附加项污染原EDEN评分”。

---

# 15. 正确的修改顺序

## 第一阶段：只修P0，不改变谱策略

1. 修复 `TrajCheck()` 第217行绝对时间；
2. 增加轨迹优化输入检查；
3. 增加候选轨迹有限性验证；
4. `PublishTraj()`增加最后检查；
5. 将体积完成判断移到规划入口；
6. 体积完成不依赖low gain；
7. 移除运行时`getchar()`；
8. 修复 `GetGainExp1/2()` 数值边界。

验收：

```text
0次NaN/Inf进入traj_pub
0次SUCCESS但没有合法轨迹
Maze达到99%后按稳定时间结束
```

---

## 第二阶段：修复City停滞

1. expected gain不再决定frontier有效性；
2. 删除expected gain直接quarantine；
3. repeat count不再直接quarantine；
4. 目标执行记录移动到轨迹成功之后；
5. late stage使用raw reachable count；
6. 增加fail-open。

验收：

```text
City不得因全部unknown_num<=1而失去所有目标
连续20次体积不变时必须恢复或显式报错
```

---

## 第三阶段：恢复实时性

1. 增加各阶段计时；
2. 保留异步solver；
3. 将快照重计算拆到worker；
4. 主线程只复制不可变原始图；
5. 连续超预算进入cooldown；
6. 删除或实现`async_solve`配置。

验收：

```text
City主线程规划时间接近EDEN
worker故意延迟500ms时主线程不等待
无TSan data race
```

---

## 第四阶段：重新开放谱路线影响

第一轮：

```yaml
max_route_regret: 0.00
region_lock: false
lock_debt: false
```

谱模块只观察。

第二轮：

```text
只在EOHDT前3个候选中建议首目标
首目标路径后悔≤2%
```

不再替换整条EOHDT路线。

---

# 16. 修订后的推荐参数

在完成源码修改前，临时使用最保守配置：

```yaml
MR_DTG/SpectralV2/enabled: true

# 保留后台求解，但暂时不让谱路线覆盖EOHDT
MR_DTG/SpectralV2/max_route_regret: 0.0
MR_DTG/SpectralV2/switch_penalty_base: 0.0
MR_DTG/SpectralV2/revisit_penalty_weight: 0.0
MR_DTG/SpectralV2/lock_debt_max: 0.0

# 在修复代码前，避免低gain自动隔离
MR_DTG/SpectralV2/frontier_expected_gain_eps: 0.0
MR_DTG/SpectralV2/repeat_target_limit: 1000000

# 降低谱任务频率
MR_DTG/SpectralV2/spectral_min_update_interval: 1.0
MR_DTG/SpectralV2/spectral_max_update_interval: 5.0
MR_DTG/SpectralV2/max_spectral_nodes: 60
```

注意：

> 这些参数只能临时绕开部分故障，不能代替源码修复。尤其是轨迹时间Bug和完成检查Bug必须改代码。

---

# 17. 必须新增的回归测试

## 17.1 绝对时间测试

给定：

```text
traj_start_t = 1000
wall time = 1001
轨迹时长 = 5
```

确认 `TrajCheck()` 调用：

```text
getPos(1)
```

而不是：

```text
getPos(1001)
```

---

## 17.2 非有限轨迹测试

构造：

- 一个系数为Inf的轨迹；
- 一个duration为NaN的轨迹；
- 一个采样状态为Inf的轨迹。

必须全部拒绝发布。

---

## 17.3 完成判断测试

让：

```text
PlanGlobalRoute始终SUCCESS
coverage=99.7%
```

仍必须返回FINISHED。

---

## 17.4 低expected gain测试

构造一个：

```text
unknown_num=1
可达
StrongCheck有效
```

的frontier。

它可以降权，但不得被自动quarantine或从raw reachable集合删除。

---

## 17.5 重复规划测试

UAV需要8秒飞到同一frontier，期间每0.5秒重规划一次。

该frontier不得因为重复被选中而quarantine。

---

## 17.6 fail-open测试

所有actionable frontier被过滤，但raw reachable仍有3个。

系统必须恢复至少1个目标，不能无限返回RETRY。

---

# 18. 最终纠正后的判断

根据实际代码，当前最重要的修复不是继续重写谱特征求解，而是：

```text
修正TrajCheck时间坐标
+
阻止非法轨迹发布
+
独立体积终止
+
取消expected gain硬过滤
+
取消基于重复规划的quarantine
+
将谱快照重计算移出主线程
+
用EOHDT保持完整路线主导权
```

最终保留的总体架构应是：

```text
EDEN/EOHDT主路线
        ↓
谱模块后台计算区域结构
        ↓
谱结果通过版本、数值和时间检查
        ↓
只对少量近邻候选提供建议
        ↓
轨迹输入/输出双重finite验证
        ↓
任何异常立即回退原EDEN或安全轨迹
```

这份方案已经针对你上传的V2源码纠正了此前关于异步、硬锁、Fiedler排序和谱求解数值检查的错误判断。
