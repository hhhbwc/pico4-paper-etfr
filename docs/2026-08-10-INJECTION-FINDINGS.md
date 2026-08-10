# 注入通道验证 & 喂图可行性结论 (2026-08-10 会话)

> 本文件记录一次深度逆向会话的关键实证结果与技术结论。
> 相关设备: PICO 4 标准版 (A8110),外区固件,已 root。
> 工具源码见 `tools/`,本次会话新增多款 ptrace 注入/patch 工具与反汇编脚本。

---

## 一、已实证: 能注入眼追数据并被系统真实消费 ✅

在渲染消费端进程 `openxr_runtime` (com.pico.xr.openxr_runtime, PID 2009) 内
patch `TrackingClient::GetEyeTrackingData` 填充假注视点:

```text
patch 期间:  sys.pxr.trackingservice.alg_confidence = 1   (有数据)
restore 后:  sys.pxr.trackingservice.alg_confidence = -1  (无数据)
```

`alg_confidence` 随注入状态**实时 ±1 联动**,证明:
- 系统确实消费了我们塞进去的眼追数据
- **ETFR/眼动追踪的数据链路核心已经打通**
- 这是硬证据,不是属性自欺(注意: `persist.pvr.foveation.level` setprop 成 6 是自欺,渲染层不读它)

**关键教训**: 注入点必须在**真正消费眼追数据的进程**。日志 `EyeTrackingClient: binderDied`
来自 openxr_runtime(2009),而 mrservice(2265) 虽加载了 client 库但不主动消费。

---

## 二、当前激活算法 = TobiiAlgorithm (专有数据流)

读运行中 `pxreyetrackingservice`(PID 9614) 的 `getAlgType` 缓存:

```text
libpxreyetrackingservice.so 基址 0x7480308000, algType 缓存 @ +0x61000+0xc
读得: 0x0000000200000002  ->  algType = 2 = TobiiAlgorithm
```

### Tobii 不消费"图像帧"的证据

`TobiiAlgorithm::onFrameAvailable`(0x53750) 完整反汇编:
```
ldrb [x0+0x2f70]   ; 启动标志
cbz  -> return      ; 未启动直接丢弃(当前必为 0,整库无任何写入者)
adrp [0x66000+0x48]; 取全局状态, <1 也丢弃
... 之后仅读 frame 的时间戳/尺寸打日志, 不处理图像内容!
```

真正算注视点的是 `onTobiiConsumerDataAvailable` / `onTobiiFoveatedDataAvailable`,
吃的是 **Tobii 硬件专有数据 (`tobii_etp_consumer_data_t`)**,不是普通 JPEG 图像。

### 结论

**"劫持 Paper 图像喂给官方算法"在当前设备物理不可行**:
- 活跃的 Tobii 算法不消费图像帧,吃专有硬件数据流
- 即使切到 StandardAlgorithm(图像型),标准版缺整套硬件基础(相机/标定/CameraInfo)
- Paper 的可见光 JPEG 与 Tobii 专有数据格式完全不匹配

---

## 三、喂图主线的完整攻坚链路 (已逆向清楚)

```text
hook hasEyeCamera=true        ✅ 已做 (@ 0x2ecdc, TrackingService::hasEyeCamera)
  └─ 让系统认为有眼追相机
hook getAlgType=1 (Standard)  ✅ 已做 (@ 0x3ac8c)   // Tobii→Standard
  └─ 尝试用图像型算法
→ 客户端调 OpenCamera+StartAlgorithm   ❌ 标准版无此调用者(核心卡点)
→ 打开眼追相机设备                      ❌ 无真设备
→ StandardAlgorithm 吃图算注视点        ❓ Standard图像链路+算法认Paper图未验证
→ ETFR/眼动追踪
```

关键函数地址 (libpxreyetrackingservice.so):
- `TrackingService::hasEyeCamera` @ 0x2ecdc (hook 成恒 true)
- `AlgorithmManager::getAlgType` @ 0x3ac8c (hook 成恒 1)
- `StandardAlgorithm::onFrameAvailable` @ 0x4b7ac (空: `ret`)
- `TobiiAlgorithm::onFrameAvailable` @ 0x53750 (专有数据, 不处理图像)
- `CameraManager::CameraImageListener::onFrameAvailable` @ 0x36c1c (喂图入口,多前置检查)
- `CameraManager::addImageListener` @ 0x3492c
- `CameraManager::openCamera` @ 0x34b18

---

## 四、实用工具链 (本次实战验证可用, 源码在 tools/)

| 工具 | 作用 | 说明 |
|---|---|---|
| `ptrace_rw.c` | attach 读写任意进程内存 | 动态链接版可直接跑, 避开静态 TLS 对齐坑 |
| `patch_func.c` | patch 函数入口为 `mov w8,#1;strb w8,[x1];ret` | 恒 true hook + 恢复 |
| `patch_ret1.c` | 通用 `*out=1` hook(X0/X1, strb/str) | apply/restore |
| `inject_gaze.c` | baseline gaze 注入(已验证 shellcode) | 16 词 patch GetEyeTrackingData |
| `inject_foveation.c` | 强制 SetFoveationLevel(9)+gaze | 22 词, 调 Pxr_SetFoveationLevel |
| `probe_calls.c` | 调用计数器探针(未验证使用) | 设计稿 |
| `ptrace_dlopen.c` | ptrace 远程 dlopen 注入(未完成) | 设计稿 |

### 编译方法 (WSL Ubuntu + NDK r27c)
```bash
/opt/android-ndk-r27c/toolchains/llvm/prebuilt/linux-x86_64/bin/\
  aarch64-linux-android24-clang -static -O2 \
  -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384 -o prog prog.c
# 注意: 静态链接 aarch64 需要 TLS 对齐参数, 否则 Bionic 报
# "executable's TLS segment is underaligned"
```

### 运行时基址定位 (ASLR)
```bash
su -c "cat /proc/<pid>/maps | grep libxxxx.so | head -1 | cut -d'-' -f1"
# 运行时函数地址 = 基址 + ELF 内偏移 (libpxreyetrackingservice.so 偏移见符号表)
```

---

## 五、设备当前注入状态 (会话结束时)

进程 `pxreyetrackingservice`(9614) 仍持有:
- `hasEyeCamera` hook → 恒 true (可随时恢复)
- `getAlgType` hook → 恒 1 / Standard (可随时恢复)

系统稳定, 未崩溃。恢复方式见各工具 `restore` 命令。

---

## 六、后续建议

1. **务实路** (成功率高): 基于已实证的注入通道, 做 Zygisk 模块 hook
   渲染消费端 `GetEyeTrackingData` 返回真实注视点(源可用 Paper 设备端解析)。
2. **正统路** (失败率高): 继续攻 StandardAlgorithm 启动 + 用 Zygisk/dlopen 注入
   驱动喂图, 但需接受"官方算法可能不认 Paper 图"的物理风险。
3. 若要继续, 首选 **ptrace dlopen 注入 hook.so** 而非手工逐字节 patch
   (Zygisk 注不进 root 原生服务; dlopen 注入更稳, 且是最终 Mod 的技术雏形)。
