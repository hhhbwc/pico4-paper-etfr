# Paper ETFR 逆向项目 - 进度

## 目标
用第三方眼追(Paper)的注视点数据,注入 PICO 4 系统,驱动**系统级注视点渲染(ETFR/动态FFR)**。

## 核心架构(已确认)
- **硬件**: 3 个独立 ESP32-S3 设备,通过 USB 枚举:
  - `/dev/bus/usb/001/003` = Left Eye Tracker (0425:0002)
  - `/dev/bus/usb/001/004` = Right Eye Tracker (0425:0003)
  - `/dev/bus/usb/001/005` = Face Tracker (0425:0001)
- **接口**: 每个设备 class=02/02(CDC-ACM 串口) + class=0a/00(CDC 数据)
- **Paper app** (`com.bridge.papertracker`): 用 `libusbhost.so` + `com.hoho.android.usbserial` 库,直接 bulk 读写这些设备,不走内核 FTDI 驱动(无 /dev/ttyUSB)
- **固件**: app 内 `assets/firmware/3.0/` 有 p4/q3/q3s 眼追固件 + face 固件(ESP32-S3,可刷写)
- **传输**: Paper 收集眼部数据 → 局域网 → PC 端 (PaperTracker.exe)

## USB 读取机制
- 端口: 左眼 1-1.1, 右眼 1-1.2, 面追 1-1.3
- 波特率可变(changeBaudRate via serialPort.setParameters)
- 读写双线程(LinkedBlockingDeque<Byte> 缓冲)

## 逆向进展
- ✅ APK 拉取 + jadx 反编译: `C:\Users\wzy\.openclaw\workspace\paper-etfr\`
- ✅ 确认 USB 拓扑和设备枚举
- ✅ 确认 Paper app 直接 libusb 读设备
- ⏳ 抓取 USB 串口原始数据帧(待: 停 Paper + 自建读取器)
- ❓ 注视点解析算法在固件还是 App(未定,关键)

## 待攻克链条
1. [ ] 抓 USB 原始数据 → 确认帧格式(固件算坐标?还是输出原始值?)
2. [ ] 解析注视点坐标(如果在固件则简单;在App则需逆向混淆 x.* 类)
3. [ ] 注入 PICO 系统渲染(最大瓶颈: pxreyetrackingservice/openxr 是否消费外部数据)
4. [ ] 延迟评估(串口直读无局域网延迟,可行)

## 关键文件
- APK: `C:\Users\wzy\.openclaw\workspace\pico4\mt\paper_base.apk`
- 反编译: `C:\Users\wzy\.openclaw\workspace\paper-etfr\jadx\`
- 固件: APK 内 assets/firmware/3.0/
- USB 信息脚本: paper-etfr/usb_info.sh
- 设备 IP (无线adb): 192.168.1.119:5555

## 风险/难点
- App 用混淆 x.* 类组织核心逻辑,反编译难
- ESC32 固件是机器码,协议常量需逆向
- 系统渲染层是否接受外部注视点数据是最大未知
