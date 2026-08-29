# Paper Pico Bridge 进度

## 当前目标

在标准版 PICO 4 上让 Paper 设备的图像采集、瞳孔检测、校准和 gaze 样本发布完全在一体机端完成。原生 OpenXR 兼容和 ETFR 是后续验证阶段，不是当前已实现的功能。

可维护实现位于仓库根目录的 [`paper-pico-bridge/`](../paper-pico-bridge/)。

## 已验证硬件与传输

- Paper 以三个 USB 设备动态枚举：
  - Face：`0425:0001`
  - Left eye：`0425:0002`
  - Right eye：`0425:0003`
- 每个眼设备动态发现 CDC control/data interface 和 bulk-IN endpoint；当前实测配置为 115200 8N1 与 DTR。
- 当前设备输出为连续 raw JPEG (`FF D8 ... FF D9`)；bridge 同时保留 APK 中 `JPG0`/`UVC0` 记录格式的解析支持。
- Pico 端已实测左右眼并行 claim、读取、解码、瞳孔候选检测和接口释放。

## 已实现一体机端数据链路

```text
Paper USB
→ native daemon
→ JPEG parser
→ JPEG decoder
→ PupilDetector
→ left/right observation pairing
→ GazeEstimator
→ versioned SharedSample
```

- `GazeSample` 有 magic、版本、结构大小、sequence、单调时间戳、有效标志与置信度字段。
- 共享样本使用序列锁发布；消费者拒绝版本不匹配、daemon 不健康、未校准、过期或非有限 gaze。
- `--dual` 保持诊断用途，只打印/统计瞳孔检测结果，不发布有效 gaze。
- `--dual-live <seconds> <calibration-file>` 仅在左右眼有效观测位于 50 ms 配对窗口内时发布融合样本。
- live 模式结束、USB 采集失败或校准加载失败时，只发布健康 heartbeat，使消费者回退而不继续使用旧 gaze。

## 校准边界

- 当前模型是左右眼各自的二维 affine 映射，输出归一化平面坐标；它不是最终的 Pico 3D gaze ray 模型。
- `--dual-record` 输出的是未标记的瞳孔观测 CSV，不是校准文件。
- live 模式只接受 `CalibrationStore::Save` 写入的固定二进制九点校准格式：九个左眼输入点、九个右眼输入点和九个目标点。
- 文件必须恰好 216 bytes，所有 float 必须有限，并且左右眼拟合必须可逆；CSV、缺失、截断或损坏文件均拒绝。
- 尚未实现用户可操作的 Pico 端九点校准 UI，因此尚无真实用户校准文件可用于产品化 live gaze。

## OpenXR Runtime 结论与安全状态

- 已定位消费端 C++ 方法：
  ```text
  pvr::TrackingClient::GetEyeTrackingData(long, int, pxr_eyepose*)
  ```
- 已验证当前构建的库签名、函数前导和 AArch64 调用 ABI。
- bridge 包含只读 Zygisk Runtime 探针、共享样本 freshness gate、输出边界适配器及隔离的 ARM64 trampoline 实验。
- `pxr_eyepose` 的完整字段语义、生命周期和坐标空间尚未证明。
- **active Hook 默认关闭**。没有写系统分区、覆盖系统库、固定 PID、固定 ASLR 地址或持久化 Runtime 代码补丁。
- 样本无效/过期时的设计行为是调用 Pico 原始函数。

## 后续里程碑

1. 实现 Pico 端九点校准交互并生成真实二进制 calibration 文件。
2. 用真实校准连续运行 `--dual-live`，评估有效率、抖动、延迟和断连回退。
3. 将归一化平面 gaze 映射到经验证的 Pico Runtime 坐标空间。
4. 验证完整 `pxr_eyepose` 输出契约。
5. 在完整恢复与稳定性测试后，评估短时 Runtime 兼容测试。
6. 使用原生眼动游戏验证消费结果；之后才评估 ETFR。
