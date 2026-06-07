# 🟢 EasyMatrix — 多功能 LED 点阵屏

> 基于 **ESP32 + Arduino (PlatformIO)** 的 32×8 RGB LED 点阵屏桌面终端。  
> 集时钟、频谱律动、动画、闹钟、音乐播放于一体，支持 WiFi 配网与 NTP 自动校时。

---

## 📸 效果预览

> TODO: 插入实物照片或 Wokwi 仿真截图

```
┌──────────────────────────────────┐
│   🕐  12:34                         │
│   MON  2026-06-07                  │
└──────────────────────────────────┘
```

---

## 🧱 硬件清单

| 组件               | 型号 / 说明                         | 数量 |
| ------------------ | ----------------------------------- | ---- |
| 主控               | ESP32 DevKit (ESP-WROOM-32)         | 1    |
| LED 点阵屏         | 8×8 RGB LED Matrix (共 4 块级联)    | 4    |
| 麦克风模块         | MAX4466 / MAX9814 (模拟音频输入)     | 1    |
| 无源蜂鸣器         | 3.3V 无源蜂鸣器 (PWM 驱动)          | 1    |
| 按键               | 轻触按键 (10kΩ 上拉)               | 3    |
| 连接线             | 杜邦线 / 排线                       | 若干  |

### 引脚连接

| ESP32 引脚 | 外设         |
| ---------- | ------------ |
| GPIO 23    | LED Matrix DIN |
| GPIO 34    | 麦克风音频输入 |
| GPIO 13    | 按键 1        |
| GPIO 14    | 按键 2        |
| GPIO 27    | 按键 3        |
| GPIO 25    | 蜂鸣器 PWM    |
| GPIO 35    | randomSeed 悬空引脚 |

---

## ✨ 功能特性

- **🕐 时钟显示** — 多种数字动画主题，NTP 自动校时，自动星期推算
- **🌐 WiFi 配网** — ESP32 启动为 SoftAP 热点，浏览器配置 SSID/密码
- **⏰ 闹钟功能** — 可设置时/分/铃声，到点响铃，3 按钮直观交互
- **🎵 音乐播放** — 蜂鸣器演奏内置曲目（超级玛丽、千与千寻、天空之城、卡农、梦中的婚礼）
- **🎶 音频律动** — 麦克风 FFT 实时频谱可视化，4 种显示模式：
  - 彩虹柱 + 白色峰值点
  - 青→紫渐变柱 + 白色峰值点
  - 垂直渐变柱
  - 仅彩色峰值点
- **🎨 动画主题** — 多种内置像素动画循环播放
- **💡 LED 亮度调节** — 5~145 范围可调，NVS 持久化
- **🔘 按钮菜单** — 3 个按键完成全部页面切换与设置

---

## 🏗️ 软件架构

项目遵循 **单一职责、依赖倒置、开闭原则、最少知识** 四大设计原则，整体分为 4 层：

```
┌─────────────────────────────────────────────┐
│                 Pages (页面层)                 │
│  ClockPage  RhythPage  AnimPage  TimePage    │
│  BrightPage  SettingPage                      │
├─────────────────────────────────────────────┤
│             StateMachine (状态机)              │
│              页面生命周期 & 事件分发             │
├───────────────┬─────────────────────────────┤
│  Core (数据层)  │    Net (网络层)              │
│  Store (NVS)    │  WifiManager  Ntp  WebServer │
│  Config (配置)  │                              │
├───────────────┴─────────────────────────────┤
│          HAL (硬件抽象层)                      │
│  LedMatrix  Buzzer  Buttons  Microphone      │
└─────────────────────────────────────────────┘
```

### 核心模块

| 模块                | 职责                                          |
| ------------------- | --------------------------------------------- |
| `LedMatrix`         | 封装 Adafruit_NeoMatrix，提供画屏 HAL 接口      |
| `Buzzer`            | PWM 发声，不关心曲目逻辑                        |
| `Microphone`        | ADC 采样 + FFT 分频，输出频段数据               |
| `Buttons`           | 按键轮询与事件分发 (短按 / 长按)                |
| `Store`             | 全局配置 NVS 持久化 (WiFi/颜色/模式/闹钟/亮度) |
| `StateMachine`      | 页面状态机，管理生命周期与页面跳转               |
| `PageBase`          | 页面接口基类，定义 enter/exit/update/onBtn*    |
| `WifiManager`       | WiFi 连接 / SoftAP 热点                        |
| `Ntp`               | NTP 校时 + 定时同步                            |
| `WebServerWrapper`  | 配网 Web 配置页面                               |

### 设计亮点

- **单例模式** — 所有模块通过 `static Xxx& instance()` 懒汉式安全单例
- **页面解耦** — 页面只依赖 `PageBase` 接口和 HAL 封装，不接触第三方库 API
- **可扩展** — 新增页面只需继承 `PageBase` 并在 `StateMachine::init()` 注册

---

## 🚀 快速开始

### 1. 环境准备

- 安装 [VS Code](https://code.visualstudio.com/) + [PlatformIO 插件](https://platformio.org/install/ide?install=vscode)
- 或安装 [PlatformIO CLI](https://docs.platformio.org/en/latest/core/installation.html)

### 2. 克隆项目

```bash
git clone git@github.com:MrliCST/LEDMatrix.git
cd LEDMatrix
```

### 3. 编译 & 烧录

```bash
# 编译
pio run

# 烧录固件
pio run --target upload

# 查看串口输出
pio device monitor
```

> 首次使用需要编译 SPIFFS 文件系统（存放 Web 配网页面的 HTML）：
> ```bash
> pio run --target buildfs
> ```
> 文件系统只需编译一次，除非 `data/` 目录内容有变更。

### 4. 配网

1. 上电后 ESP32 启动为 **SoftAP 热点**，SSID 为 `EasyMatrix`
2. 手机/电脑连接该热点
3. 浏览器访问 `http://192.168.1.1`
4. 配置 WiFi SSID 和密码，保存后自动重启连接

> 也可以使用 Wokwi 仿真器联调（参见 `wokwi.toml`）

---

## 📁 目录结构

```
LEDMatrix/
├── include/
│   ├── core/
│   │   ├── config.h         # 引脚定义、枚举、常量
│   │   ├── state_machine.h  # 页面状态机
│   │   └── store.h          # 全局配置 NVS 存储
│   ├── hal/
│   │   ├── led_matrix.h     # LED 点阵 HAL
│   │   ├── buzzer.h         # 蜂鸣器 HAL
│   │   ├── buttons.h        # 按键 HAL
│   │   └── microphone.h     # 麦克风 FFT HAL
│   ├── img/
│   │   ├── elems/           # 数字、图标等像素位图
│   │   └── img.h
│   ├── net/
│   │   ├── wifi_manager.h   # WiFi 连接 & SoftAP
│   │   ├── ntp.h            # NTP 校时
│   │   └── web_server.h     # 配网 Web 页面
│   ├── pages/
│   │   ├── page_base.h      # 页面基类
│   │   ├── time_page.h      # 时钟显示页面
│   │   ├── clock_page.h     # 闹钟设置页面
│   │   ├── rhythm_page.h    # 频谱律动页面
│   │   ├── anim_page.h      # 动画页面
│   │   ├── bright_page.h    # 亮度调节页面
│   │   └── setting_page.h   # WiFi 配网页面
│   └── songs/
│       ├── elems/            # 曲谱数据
│       └── songs.h           # 歌曲管理器
├── src/
│   ├── main.cpp             # 入口程序
│   ├── core/
│   ├── hal/
│   ├── net/
│   └── pages/
├── data/                    # SPIFFS 文件系统 (Web 页面)
├── partitions_spiffs.csv    # 分区表
├── wokwi.toml               # Wokwi 仿真配置
├── platformio.ini           # PlatformIO 工程配置
├── CLAUDE.md                # AI 辅助开发文档
└── README.md
```

---

## 🔧 配置说明

所有用户可调配置均通过 **3 个物理按键 + 点阵屏菜单** 完成：

| 页面       | 按键 1 (短按)    | 按键 2 (短按)    | 按键 3 (短按)    | 按键 3 (长按)     |
| ---------- | ---------------- | ---------------- | ---------------- | ----------------- |
| TIME (时钟) | 切换数字动画主题  | 切换时间格式     | 进入 RHYTHM      | 重启进入配网      |
| RHYTHM (律动) | 频谱模式 +1    | 频谱模式 -1      | 进入 ANIM        | 重启进入配网      |
| ANIM (动画) | 动画模式 +1      | 动画模式 -1      | 进入 CLOCK       | —                 |
| CLOCK (闹钟) | 切换选中位      | 调整数值         | 确认 / 保存      | —                 |
| BRIGHT (亮度) | 亮度 +5         | 亮度 -5          | 返回 TIME        | —                 |

> 长按任意页面下 **按键 3** 均可重启进入 WiFi 配网模式。

---

## 🎵 内置曲目

| 曲目                | 来源                   |
| ------------------- | ---------------------- |
| 🎮 Super Mario      | 超级玛丽主题曲         |
| 🎬 Always With Me   | 千与千寻 — 永远同在    |
| 💒 Dream Wedding    | 梦中的婚礼             |
| 🏰 Castle in the Sky | 天空之城              |
| 🎹 Canon            | 卡农 (Pachelbel)       |

---

## 🔌 硬件接线参考

```
ESP32                   LED Matrix 级联 (4×8×8)
─────                   ────────────────────
GPIO23 ───────────────→ DIN (第1块)
                         DOUT → DIN (第2块)
                         DOUT → DIN (第3块)
                         DOUT → DIN (第4块)
5V ──────────────────── VCC (所有块并联)
GND ─────────────────── GND (所有块并联)

ESP32                   麦克风 (MAX4466)
─────                   ─────────────
GPIO34 ←─────────────── OUT
3.3V ────────────────── VCC
GND ─────────────────── GND

ESP32                   蜂鸣器
─────                   ──────
GPIO25 ──────────────── PWM (正极)
GND ─────────────────── GND (负极)

ESP32                   按键 (10kΩ 上拉至 3.3V)
─────                   ──────────────────────
GPIO13 ←─────────────── 按键 1 → GND
GPIO14 ←─────────────── 按键 2 → GND
GPIO27 ←─────────────── 按键 3 → GND
```

---

## 🧪 Wokwi 仿真

项目支持 [Wokwi](https://wokwi.com/) 在线仿真调试：

```bash
# 安装 wokwi CLI
pip install wokwi

# 启动仿真
wokwi start -c wokwi.toml
```

访问 `http://localhost:8180` 可查看仿真器的 Web 配网页面。

---

## 🤝 贡献指南

1. Fork 本仓库
2. 创建新分支 (`git checkout -b feature/xxx`)
3. 提交改动 (`git commit -m 'feat: add xxx'`)
4. 推送到分支 (`git push origin feature/xxx`)
5. 发起 Pull Request

### 代码风格

- 尽早退出，避免嵌套 `if`
- 函数大括号与 `()` 对齐
- 新增文件不写注释，除非 WHY 不显而易见
- 三个相似的写法 > 一个过早的抽象

---

## 📄 许可证

本项目基于 **MIT License** 开源 — 详见 [LICENSE](LICENSE) 文件。

---

<p align="center">
  Made with ❤️ by <a href="https://github.com/MrliCST">MrliCST</a> &amp; Contributors
</p>
