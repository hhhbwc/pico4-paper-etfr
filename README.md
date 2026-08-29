# PICO 4 + Paper Eye Tracker

将第三方 **Paper Tracker** 接入标准版 **PICO 4 (A8110)** 的研究与开发仓库。当前优先目标是让所有采集、视觉处理和 gaze 样本发布均在 Pico 一体机端完成；在坐标空间和 Runtime 输出结构验证完成后，再评估原生眼动游戏兼容与 ETFR。

> English: [README.en-US.md](README.en-US.md) · Русский: [README.ru-RU.md](README.ru-RU.md)

## 当前状态

已在 root 的 PICO 4 上验证以下一体机端链路：

```text
Paper USB 双眼设备
→ CDC 初始化 / bulk-IN 读取
→ raw JPEG 重组
→ Pico 本地 JPEG 解码
→ 瞳孔候选检测
→ 双眼时间配对
→ 已校准 gaze 样本共享内存
```

- 动态发现三个 Paper USB 设备：Face `0425:0001`、Left `0425:0002`、Right `0425:0003`。
- 当前设备实际输出是连续 raw JPEG，而非只有 APK 中存在的 `JPG0` 记录格式；bridge 同时保留两种解析器。
- Pico 本地已验证左右眼并行读取、JPEG 解码和瞳孔候选检测。
- `--dual` 是只读诊断采集模式；`--dual-live <seconds> <calibration-file>` 只在有效二进制九点校准文件存在时发布融合 gaze 样本。
- 缺失、CSV、截断或非有限数值校准文件均会被拒绝，只留下健康 heartbeat。

可维护的实现位于 [paper-pico-bridge/](paper-pico-bridge/)。其中包含 CMake 原生工程、设备端 daemon、共享样本 ABI、校准、视觉基础实现、测试和默认关闭的 Zygisk Runtime 探针。

## 已确认的架构结论

### Paper 提供图像流，不是可直接消费的 gaze

Paper 设备经 USB 输出眼部 JPEG 图像。原始 Paper PC 流程在桌面端从图像计算结果；当前 bridge 改为在 Pico 设备端直接读取和处理图像，不依赖 PC 或局域网坐标转发。

### 不再以“把 JPEG 交给 Pico 官方算法”为主线

已验证当前激活的 Pico Tobii 算法路径使用专有数据，并不接受普通 Paper JPEG；将外挂相机图像喂给原生服务不是可用的主线。

当前主线是：

```text
Paper 图像
→ Pico 本地视觉 / 校准
→ 版本化共享 GazeSample
→ 经验证的 OpenXR Runtime 消费端兼容层
```

### OpenXR 消费点已定位，但 Hook 默认关闭

当前固件中已定位到：

```text
pvr::TrackingClient::GetEyeTrackingData(long, int, pxr_eyepose*)
```

bridge 的 Zygisk 组件仅匹配 OpenXR Runtime，并进行库映射、函数前导和共享样本新鲜度的只读探针。未知固件、无效/过期样本或未校准样本都 fail-closed，保持 Pico 原始行为。

**没有启用 Runtime inline Hook、没有覆盖系统库、没有宣称原生眼动游戏或 ETFR 已经可用。**

## 下一步

1. 实现用户可操作的 Pico 端九点校准，生成受校验的二进制校准文件。
2. 用真实校准验证 `--dual-live` 连续发布的 gaze 样本、稳定性和延迟。
3. 建立 Paper gaze 平面坐标与 Pico Runtime 所需坐标空间之间的可验证映射。
4. 验证 `pxr_eyepose` 的完整输出语义和生命周期。
5. 在完整 fallback 与可恢复测试通过后，评估短时 Runtime 兼容测试与原生游戏验证。
6. 最后评估 ETFR；它不是当前已完成的功能。

历史逆向证据、协议资料和早期实验仍保留在 [docs/](docs/) 与 [tools/](tools/) 中；一次性 ptrace 实验工具不属于 `paper-pico-bridge` 的默认运行路径。

## 构建与部署边界

`paper-pico-bridge` 需要 Android NDK、CMake 和 Ninja。其 `tools/build.ps1` 从 `ANDROID_NDK_HOME` 读取 NDK 路径。安全模块的默认 daemon 是空跑模式，不会自动 claim USB；USB 读取及 live gaze 发布必须通过显式 daemon 命令运行。

## 许可证

MIT
