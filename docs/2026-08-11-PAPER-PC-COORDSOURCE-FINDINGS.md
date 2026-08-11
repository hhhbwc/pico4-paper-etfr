# PC 端 Paper 坐标源抓取结论 (2026-08-11 会话)

> 本文件记录对 **PC 端 PaperTracker (Windows)** 坐标源的网络层探测结论。
> 结论: Paper 眼追坐标**不通过 OSC / UDP 网络包输出**, 走进程内直接消费。
> 相关文档: [2026-08-10-INJECTION-FINDINGS.md](./2026-08-10-INJECTION-FINDINGS.md)
> (该文档已实证"系统消费端 GetEyeTrackingData 注入通道打通"; 本文件从**数据源侧**交叉验证)

---

## 背景与目标

PICO 4 标准版无眼追硬件, 但系统保留完整软件链路。方案: 用第三方眼追 Paper 的坐标,
注入系统消费端启用 ETFR。本会话尝试**在 PC 端抓取 Paper 已经算好的注视点坐标**,
以便转发给 PICO 端注入。

Paper 架构 (Windows PC):
- 3 个 ESP32-S3 摄像头 (Left/Right/Face) 通过 WiFi/UDP 把 **JPEG 图像**发给 PC 端 PaperTracker.exe
- PC 端直接读图像 (`libusb` / UDP), 用 onnxruntime + DirectML (RTX 4060) 算注视点
- 算完坐标后**在进程内直接驱动 VRChat 面捕插件** (VrcBridgeHelper)

## 已完成的探测 (全部实证, 非推测)

### 1. Paper 的 OSC 通道 = 9001 (不是 9000)

`PaperTracker` 启动日志:
```
bridge active: 9009 -> 9001
VrcBridgeHelper: avatar 参数刷新(initial-sync), 数量 = 313
```

- 端口 9000 被 VRChat (PID) 独占监听
- Paper 实际向 **127.0.0.1:9001** 持续发送 OSC avatar 参数
- `SO_REUSEADDR` 并行绑定 9001 抓包: **成功捕获大量 OSC 明文**

### 2. 抓到的 OSC 全是身体/移动参数, 无眼追坐标

捕获到的完整路径清单 (去重后):
```
/avatar/parameters/VelocityX / VelocityZ / VelocityY / VelocityMagnitude
/avatar/parameters/AngularY / Grounded
/avatar/parameters/ParameterMerge_Cloth* 
/avatar/parameters/LightLimitChanger/System/ParameterSyncer*
```

**没有任何 eye / gaze / pupil / tracking / viseme / blink 相关路径。**

→ 即使 VRChat 模型装了面捕插件且有变化, Paper 的**眼追坐标也不走 OSC**。
  (符合 Paper 架构: 坐标是进程内算完直接用, OSC 只是给面捕参数用的子集)

### 3. Paper 到 PICO 的 UDP 通道 (9101) 绑定监听 = 零包

`netstat` 显示 Paper 持有发往 `192.168.1.119:9101` (PICO IP) 的 socket:
```
UDP  0.0.0.0:62355  192.168.1.119:9101  34684  (PaperTracker)
```
但 bind 9101 监听 (含清理残留进程后干净的 SO_REUSEADDR 绑定):
**持续 >= 20 秒零包**。→ Paper 建立了 socket 但当前**不在 9101 上持续发坐标**。

### 4. 9100/9101/20425/20426 全部绑定监听 → 均无坐标流量

对 Paper 所有对外 UDP socket 目标端口逐一绑定监听, 均无眼追坐标数据。
唯一有流量的是 9001 (OSC avatar 参数, 无 eye)。

---

## 核心结论

**Paper 的注视点坐标不通过 UDP/OSC 网络包输出 —— 在进程内直接消费。**

这与 `INJECTION-FINDINGS.md` 的判断交汇:
- 想从 PC 端抓 Paper 坐标再转发 PICO, **网络层路径在此版本 Paper 上不可行**
- 注入口必须换到**系统消费端 `GetEyeTrackingData`** (已实证能注入), 
  数据源用 **Paper 设备端解析** (esp32 固件/USB CDC) 或**自行算坐标**

## 对项目方向的影响

1. **放弃**: 抓 PC PaperTracker 的 OSC/UDP 坐标出口 (此版本堵死)
2. **保留**: `INJECTION-FINDINGS.md` 的务实路 —— Zygisk 模块 hook 消费端
   `GetEyeTrackingData`, 数据源来自 Paper **设备端** (固件/USB) 或自算
3. **下一步候选**:
   - 进 Paper 进程内部 hook (Windows API hook `sendto`/OSC 发送函数) 截坐标 — 重, 且非最终形态
   - 直接攻 Paper 设备端 (ESP32 固件/USB CDC) 拿原始数据 — 更贴近最终 Mod
   - 或自建坐标源 (读摄像头图像+OpenCV) 完全绕开 Paper — 最干净但工作量最大

## 关键端口速查 (PaperTracker Windows 版)

| 端口 | 方向 | 内容 |
|---|---|---|
| 9000 | VRChat 监听 | VRChat OSC 入口 (Paper 不直接发这) |
| 9001 | Paper→ 本地 | **OSC avatar 参数** (有流量, 无 eye) |
| 9009 | Paper 接收 | bridge 输入 (VRChat→Paper) |
| 9100/9101/20425/20426 | Paper 持有 socket | 绑定监听零包, 无坐标 |
| 45454 | Paper 监听 | UDP 诊断 listener |
| 5353 | Paper | mDNS |

## 会话遗留

- 多个测试监听 Python 后台进程/孤儿进程, 均已终止 (若残留可 `taskkill /PID` 清理)
- 测试脚本: `paper-osn/` (osc_listen.py / grab_9100.py / listen_9100.py / listen_osc.py / listen_eye.py)
