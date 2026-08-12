# PICO 4 + Paper Eye Tracker → 系统级注视点渲染 (ETFR/Foveation)

用第三方眼追 **Paper Tracker** 的数据，驱动 **PICO 4 (A8110)** 系统级**注视点渲染（Eye-Tracked Foveated Rendering, ETFR）** 的逆向研究项目。

> English: [README.en-US.md](README.en-US.md) · Русский: [README.ru-RU.md](README.ru-RU.md)

## 目标

PICO 4 **标准版没有眼追硬件**，但系统（与 Pro 版同一系统）**完整保留了眼追 + 注视点渲染的软件链路**。本项目的目标是：**注入第三方眼追(Paper)的数据，让系统以为自己有原生眼追，从而启用 ETFR（渲染跟随眼球注视点，节省 GPU）。**

## 核心结论（逆向已确认）

### 1. Paper 传输的是"图像"，不是"坐标"
- 传感器是 **3 个 ESP32-S3 USB 设备**：Left Eye (0425:0002) / Right Eye (0425:0003) / Face (0425:0001)
- PICO app(`com.bridge.papertracker`)用 **libusb 直接读**，不走内核 FTDI 驱动
- **PICO → PC 只发眼睛图像(240×320 灰度 JPEG)**，UDP 单播 `192.168.1.119:35506 → PC:45454`
- **注视点坐标 = PC 端从图像计算**（Paper 的"PC 处理数据"本质）
- 帧协议：`PT` 头(5054 0101) + 帧计数 + 分片序号 + 长度 + JPEG

### 2. PICO 渲染层有完整 ETFR API（链路是通的）
`libopenxr_api.so` 导出（前 40 个）:
- `Pxr_GetEyeTrackingData` / `Pxr_GetEyeTrackingData1`
- `Pxr_StartEyeTracking` / `Pxr_StopEyeTracking`
- `Pxr_GetEyeTrackingSupported` / `Pxr_GetEyeTrackingState`
- `Pxr_SetFoveationLevel` / `Pxr_SetFoveationParams` / `Pxr_GetLayerFoveationImage`

**标准版与 Pro 同一系统 → 渲染消费链路存在，只差数据源。**

### 3. 系统眼追服务完整但"空转"（关键状态）
`pxreyetrackingservice`（注册 binder `pvr.IEyeTrackingService`）:
```
The status of eye camera: Nonexistent   <- 标准版无眼追相机
ET algorithm is not started.
isSupportEyetracking = 0
```
- 服务完整、可 binder 调用，只是**检测到没硬件 → 不自检启动**
- **`isSupportEyetracking` 是软件判定（可 hook）**

### 4. 系统眼追内部架构
`libpxreyetrackingservice.so` / `libpxreyetracking.phoenix.so`:
```
CameraManager::openCamera/addImageListener → onFrameAvailable  ← 相机图像
AlgorithmBase::setResultsListener / getTrackingDataSharedMemory  ← 算法
TrackingService::GetData / SetData / GetTrackingDataSharedMemory  ← 数据输出
```

## 注入路径（已明确，待实现）

**方案（劫持系统，复用原生算法+渲染）：**
1. **hook `isSupportEyetracking` → 返回 true**（总开关）
2. **让 CameraManager 认为有相机**：把 Paper 的图像喂给系统
3. 系统算法跑起来 → 算注视点 → 渲染消费 → **ETFR 启用**

**或备选：hook `Pxr_GetEyeTrackingData` / `SetData` 直接注入坐标。**

## 工具（`tools/`）
- `usbcdc_read.c` — USB CDC 读取器(NDK 编译)
- `ptrace_probe.c` — ptrace 注入探测（已确认可 attach）
- `list_9100.ps1` / `cap_osc.ps1` — PC 端 OSC/网络分析

## 待办 / 卡点
- [ ] ptrace 注入 `pxreyetrackingservice`（系统 root 服务，需专门攻坚）
- [ ] hook `isSupportEyetracking` + 数据源注入
- [ ] 系统算法数据结构逆向（DataBufferParcelable 注视点格式）
- [ ] VR 实测渲染是否响应（foveation 跟随）

## 许可证
MIT
