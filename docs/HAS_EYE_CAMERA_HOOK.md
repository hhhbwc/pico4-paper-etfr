# hasEyeCamera Hook 分析（注入攻坚 - 关键发现）

## 位置
`libpxreyetrackingservice.so` → `_ZN3pvr15TrackingService12hasEyeCameraEPb` @ **0x2ecdc** (size 0x134)

## 反汇编逻辑（已解码）
```
AlgorithmManager::getAlgType() -> w8 (当前算法类型)
if   (w8 == 2): 返回对象字段 [x21+152]   // 内存标志
elif (w8 == 1): fopen(某路径) → 存在则 w8=1 否则 w8=0   // 文件检测
else          : w8 = 0                                  // 无相机
[x20] = w8 (写入返回 bool)
```

## hook 策略
**让 hasEyeCamera 总是返回 true**：
- 方案A：函数入口 patch 为 `mov w8,#1; strb w8,[x20]; <返回binder ok>` 
- 方案B：ptrace 改该函数指令 / 函数指针替换

## ⚠️ 关键限制（重要）
**只 hook hasEyeCamera 不够**。底层还有：
1. `TrackingService::OpenCamera` → `CameraManager::openCamera` 需能真打开视频设备（标准版无相机设备→可能失败）
2. 算法启动（`StartAlgorithm`）
3. **图像数据源**（系统算法要真实图像帧 `onFrameAvailable` → 需喂 Paper 图）
4. 注视点数据格式（DataBufferParcelable）
5. 渲染层最终响应（`Pxr_GetEyeTrackingData`/foveation 消费）

## 完整注入链路（全部待攻坚）
```
hook hasEyeCamera=true
  → hook/满足 openCamera(设备打开)
  → 喂 Paper 图像给 CameraManager(劫持图像源)
  → 算法跑 → 算注视点
  → 渲染消费 → ETFR 启用
```

## 后续攻坚建议
- 逐个环节验证：先确认 `openCamera` / `CameraManager::addImageListener` 在无硬件时行为
- 备选：hook `Pxr_GetEyeTrackingData` 直接注入坐标（绕开相机/算法）
- 需要专门会话 + 救机预案

## 待办
- [ ] ptrace 注入 pxreyetrackingservice（需完成注入器开发）
- [ ] hasEyeCamera hook 实现
- [ ] openCamera/图像源方案
- [ ] VR 实测
