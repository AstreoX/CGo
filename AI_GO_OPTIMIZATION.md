# 围棋 AI 优化总结

## ⚠️ 重要说明

本次优化专门针对**围棋（Go/Baduk）**规则，不是五子棋！

围棋的胜负判断基于：
- **提子数**
- **地盘（目数）**
- **气数**

## 优化概述

提升围棋 AI 的智能水平和决策质量，主要优化以下方面：

---

## 1. 参数优化

### 修改前：
- `DEFAULT_SIMULATION_COUNT`: 200
- `DEFAULT_EXPLORATION_PARAM`: 3.2 (非标准值)
- `MCTS_RANGE_SMALL`: 2 (5×5 范围)

### 修改后：
- `DEFAULT_SIMULATION_COUNT`: **1000** (提升 400%)
- `DEFAULT_EXPLORATION_PARAM`: **1.41** (标准值 √2)
- `MCTS_RANGE_SMALL`: **3** (7×7 范围)

**影响**：
- 更多模拟次数 → 更准确的局势判断
- 标准探索参数 → 更好的探索/利用平衡
- 更大搜索范围 → 考虑更多可能性

---

## 2. UCT 公式标准化

### 修改前：
```c
double visitFactor = sqrt(2.0 - (node->visits / (double)(parentVisits + 1)));
double exploration = explorationParam * visitFactor * sqrt(log(parentVisits) / node->visits);
```
非标准实现，额外的 visitFactor 调整。

### 修改后：
```c
// 标准 UCT 公式: exploitation + C * sqrt(ln(N) / n)
double exploitation = node->wins / (double)node->visits;
double exploration = explorationParam * sqrt(log((double)parentVisits) / (double)node->visits);
```

**优势**：
- 符合 MCTS 学术标准
- 更好的理论保证
- 便于调参和对比

---

## 3. 改进评估函数（核心优化）

### 修改前：
```c
// 仅考虑提子数
int blackScore = tempBoard.blackCaptures;
int whiteScore = tempBoard.whiteCaptures;
score = (blackScore > whiteScore) ? 1.0 : 0.0;  // 简单二值判断
```

### 修改后：
```c
// 综合评估：40% 提子 + 30% 气数 + 30% 地盘
double captureScore = ...;   // 提子数比例
double libertyScore = ...;   // 气数比例
double territoryScore = ...; // 地盘估算
finalScore = 0.4 * captureScore + 0.3 * libertyScore + 0.3 * territoryScore;
```

**三维评估系统**：

#### 1. 提子数评分（权重 40%）
```c
captureScore = blackCaptures / (blackCaptures + whiteCaptures)
```
- 提子是围棋中的重要得分手段
- 每提一子相当于获得 1 目

#### 2. 气数评分（权重 30%）
```c
libertyScore = blackLiberties / (blackLiberties + whiteLiberties)
```
- 气数多 → 棋子安全，不易被提
- 气数少 → 棋子危险，可能被吃

#### 3. 地盘估算（权重 30%）
```c
// 简化算法：统计每个空点周围的棋子
for (每个空点) {
    检查周围 3×3 范围
    if (黑棋多) → 黑方地盘
    else if (白棋多) → 白方地盘
}
```
- 虽然不如专业的目数计算，但在模拟中足够高效
- 能大致反映双方的势力范围

---

## 4. 性能对比

| 指标 | 优化前 | 优化后 | 提升 |
|------|--------|--------|------|
| 模拟次数 | 200 | 1000 | +400% |
| 探索参数 | 3.2 | 1.41 | 标准化 |
| 搜索范围 | 5×5 | 7×7 | +96% |
| 评估维度 | 1 (提子) | 3 (提子+气+地盘) | 质的提升 |
| UCT 公式 | 非标准 | 标准 | 更科学 |

---

## 5. 预期效果

### 战术层面：
- ✅ 更重视保护气数（不易被提子）
- ✅ 更好的地盘意识（围地能力提升）
- ✅ 提子判断更准确（知道何时该提子）

### 策略层面：
- ✅ 局势判断更全面（不只看提子）
- ✅ 模拟质量更高（评估更接近真实围棋）
- ✅ 决策更稳定（更多模拟次数）

---

## 6. 与围棋规则的对应

| 围棋规则 | AI 实现 |
|---------|---------|
| 提子得分 | ✅ 40% 权重评估 |
| 目数计算 | ✅ 30% 权重（简化地盘估算）|
| 气的重要性 | ✅ 30% 权重评估 |
| 死活判断 | ⚠️ 间接通过气数反映 |
| 打劫规则 | ✅ 已在 board.c 中实现 |

---

## 7. 文件修改

**src/ai.c**:
- 第 19-23 行：参数优化
- 第 95-107 行：UCT 公式标准化
- 第 442-510 行：评估函数改进（新增 68 行代码）

---

## 8. 后续建议

如需进一步提升围棋 AI 水平，可考虑：

### 短期优化：
1. **增加模拟次数**至 2000-3000（需要更多计算时间）
2. **优化地盘计算**：使用更精确的算法（如 BFS 连通域分析）
3. **征子检测**：识别并评估征子情况

### 中期优化：
4. **死活判断**：简单的死活判断（如双眼做活）
5. **定式库**：常见开局定式
6. **形势判断**：更准确的优势判断

### 长期优化：
7. **神经网络评估**：参考 AlphaGo 的价值网络
8. **策略网络**：指导落子选择
9. **深度学习**：完全的深度强化学习方案

---

## 9. 测试建议

1. 对比优化前后的对局质量
2. 测试 AI 在不同局面的表现：
   - 开局：布局是否合理
   - 中盘：战斗能力
   - 官子：收官效率
3. 检查 AI 响应时间（应在 3 秒内）
4. 观察提子、围地、护气的平衡

---

**优化完成时间**: 2025-11-25
**优化目标**: 围棋（Go/Baduk）
**优化者**: Claude AI Assistant
