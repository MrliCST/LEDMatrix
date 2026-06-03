# EasyMatrix TIME & CLOCK 完整实现链路

## 项目概述

- **硬件**: ESP32-C3 + 4×8×8 WS2812B LED 矩阵 (32×8 像素)
- **框架**: PlatformIO + Arduino + FreeRTOS
- **核心库**: Adafruit_NeoMatrix, Adafruit_NeoPixel, Adafruit_GFX, Ticker, OneButton

---

## 文件索引

| 文件 | 职责 |
|------|------|
| `src/main.cpp` | 主循环调度、`setup()` 启动流程 |
| `src/common.h` | 全局常量、枚举、引脚定义、HTML 页面模板 |
| `src/light.cpp` / `.h` | 全部 LED 绘制函数 (`drawTime`, `drawClock`, `drawAnim`, ...) |
| `src/task.cpp` / `.h` | FreeRTOS 任务、Ticker 定时器、按键逻辑 |
| `src/net.cpp` / `.h` | WiFi 连接、NTP 对时、SoftAP 配网 |
| `src/preferencesUtil.cpp` / `.h` | NVS 非易失性存储读写 |
| `src/buzzer.cpp` / `.h` | 蜂鸣器音乐播放 |
| `src/songs.h` | 5 首曲谱数据（音符 + 时值） |
| `src/img/timeAnim0.h` ~ `timeAnim8.h` | 9 帧 11×8 动画位图 |
| `src/img/bell.h` | 铃铛图标 8×6 位图 |
| `src/img/checkingTime.h` | "对时中" 提示 32×8 位图 |

---

# 一、TIME 页面完整实现链路

## 1.1 数据结构与状态变量

### 1.1.1 全局常量 (`common.h`)

```cpp
#define MATRIX_SIDE 8              // 单块矩阵边长
#define MATRIX_WIDTH 32            // 矩阵总宽度 (4×8)
#define MATRIX_COUNT 4             // 矩阵数量
#define ANIM_INTERVAL 200          // 时钟动画帧间隔 (ms)
#define TIME_CHECK_INTERVAL 18000  // NTP 对时间隔 (s) = 5 小时
#define BRIGHTNESS 45              // 默认亮度
#define BRIGHTNESS_SPACING 20      // 亮度调节步长

// TIME 子页面常量
const int TIME_H_M_S  = 1;  // 显示 时:分:秒
const int TIME_H_M    = 2;  // 显示 时:分 + 左侧动画
const int TIME_DATE   = 3;  // 显示 月-日 + 左侧动画

// 页面枚举
enum CurrentPage { SETTING, TIME, RHYTHM, ANIM, CLOCK, BRIGHT };
```

### 1.1.2 全局状态变量

```cpp
// ===== light.cpp / light.h =====
int timePage;                  // TIME 当前子页面 (存 NVS)
uint16_t mainColor;            // 主题色 = matrix.Color(R, G, B)
uint16_t weekColor;            // 星期次要色 (取反色算法)
int brightness;                // 当前亮度 5~145
int clockColor[3];             // RGB 颜色分量 {R, G, B}
Adafruit_NeoMatrix matrix;     // LED 矩阵对象 (32×8)

// ===== main.cpp =====
enum CurrentPage currentPage;  // 当前页面
unsigned int prevDisplay;      // 上次渲染的 time_t (用于每秒刷新)

// ===== task.cpp =====
Ticker tickerAnim;             // 动画帧定时器
Ticker tickerCheckTime;        // NTP 对时定时器
Ticker tickerClock;            // 闹钟一次性定时器
int animIndex;                 // 动画帧编号 0~9
bool isCheckingTime;           // NTP 对时中标志
bool belling;                  // 闹钟响铃中标志
TaskHandle_t showTextTask;     // 启动文字动画任务句柄
```

---

## 1.2 进入 TIME 页面的四条入口

### 入口 A: 系统启动 `setup()` → WiFi → NTP → TIME

`main.cpp:25-83`

```
setup()
  │
  ├─ Serial.begin(115200)
  ├─ getInfos()                          // preferencesUtil.cpp
  │   ├─ prefs.begin("matrix")
  │   ├─ 读 ssid, pass, R, G, B, apConfig
  │   ├─ 读 timePage, rhythmPage, animPage (恢复上次子页面)
  │   ├─ 读 clockH, clockM, clockBellNum, clockOpen
  │   ├─ 算 mainColor = matrix.Color(R, G, B)
  │   ├─ 算 weekColor (取反色: max(R,G,B)分量保留, 其余用 255-c)
  │   ├─ 读 brightness (默认 45)
  │   └─ prefs.end()
  │
  ├─ initMatrix()                        // light.cpp
  │   ├─ matrix.begin()                  // Adafruit_NeoMatrix 初始化
  │   ├─ matrix.setTextWrap(false)
  │   ├─ sampling_period_us = round(1e6 / SAMPLING_FREQ)
  │   └─ pinMode(AUDIO_IN_PIN, INPUT)   // 拾音器
  │
  ├─ createShowTextTask("START")         // task.cpp
  │   └─ xTaskCreate(startShowText, 8192)
  │       └─ 循环: "START." → "START.." → "START..." 每秒交替
  │
  ├─ btnInit()                           // task.cpp
  │   ├─ button1(GPIO3): click→btn1click, longPress(1.2s)→btn1LongClick
  │   ├─ button2(GPIO4): click→btn2click, longPress(1.2s)→btn2LongClick
  │   └─ button3(GPIO7): click→btn3click, longPress(1.5s)→btn3LongClick
  │
  ├─ if (apConfig) → SETTING 配网 (不走 TIME)
  │
  └─ else ──────────────────────────────────────────────────────
      ├─ connectWiFi(30)                 // net.cpp
      │   ├─ WiFi.begin(ssid, pass)
      │   └─ 循环等待 WL_CONNECTED
      │       ├─ 成功 → wifiConnected = true
      │       └─ 超时:
      │           vTaskDelete(showTextTask)
      │           drawFailed(4, 24, "WIFI")   // 显示 "WIFI X"
      │           currentPage = RHYTHM
      │           return
      │
      ├─ if (wifiConnected)
      │   ├─ checkTime(30)               // net.cpp
      │   │   ├─ configTime(8*3600, 0, 3个NTP服务器)
      │   │   └─ 循环 getLocalTime()
      │   │       ├─ 成功 → RTCSuccess = true
      │   │       └─ 超时:
      │   │           vTaskDelete(showTextTask)
      │   │           drawFailed(4, 24, "TIME")  // 显示 "TIME X"
      │   │           currentPage = RHYTHM
      │   │           return
      │   │
      │   └─ if (RTCSuccess)
      │       ├─ startTickerCheckTime()   // task.cpp
      │       │   └─ tickerCheckTime.attach(18000, checkTime)
      │       │
      │       ├─ if (clockOpen)
      │       │   └─ startTickerClock(getClockRemainSeconds())
      │       │       // task.cpp, 计算闹钟剩余秒数
      │       │       // clockTime > nowTime → clockTime - nowTime
      │       │       // clockTime ≤ nowTime → clockTime + 86400 - nowTime
      │       │       // tickerClock.once(seconds, ringingBell)
      │       │
      │       ├─ currentPage = TIME              // ★ 进入 TIME
      │       ├─ vTaskDelete(showTextTask)       // 停止启动动画
      │       └─ clearMatrix()                   // 全屏清黑
      │
      └─ disConnectWifi()                // WiFi.disconnect()
```

### 入口 B: 从 BRIGHT 页按键切换

`task.cpp:619-628`

```
btn3click() → case BRIGHT:
  if (!RTCSuccess)
    currentPage = RHYTHM
  else
    currentPage = TIME    // ★ 无清屏/无动画停止，依赖下次 drawTime() 的 fillScreen(0)
```

### 入口 C: 闹钟响铃强制切 TIME

`task.cpp:206-228`

```
ringingBell():
  if (currentPage == SETTING || currentPage == CLOCK)
    return                        // 这两个页面静默不响
  if (currentPage == RHYTHM)
    memset(matrixArray, 0, ...)   // 清理节奏灯状态
    lightedCount = 0
  currentPage = TIME              // ★ 强制切 TIME
  createBellTask()                // FreeRTOS 响铃任务
  belling = true
  tickerClock.detach()
  startTickerClock(getClockRemainSeconds())  // 重置到明天同时间
```

### 入口 D: 主循环 TIME 分支 (每秒渲染)

`main.cpp:141-149`

```
loop():
  watchBtn()                      // button1/2/3.tick() 扫描按键
  if (isCheckingTime) ...         // 对时中 → 走对时分支
  else switch (currentPage)
    case TIME:
      time_t now; time(&now);     // 读 ESP32 RTC
      if (now != prevDisplay)     // ★ 每秒只执行一次
        prevDisplay = now
        drawTime()                // light.cpp
```

---

## 1.3 `drawTime()` 绘制链路

`light.cpp:119-228`

```
drawTime():
  struct tm timeinfo;
  getLocalTime(&timeinfo)          // 读 ESP32 RTC
  matrix.setBrightness(brightness)
  matrix.setTextColor(mainColor)
  weekNum = timeinfo.tm_wday
  if (weekNum == 0) weekNum = 7    // 周日=0 → 7

  ┌── timePage == TIME_H_M_S ─────────────────────────────────────
  │  matrix.fillScreen(0)          // 全屏清黑
  │  // 格式化 HH:MM:SS (不足两位补0)
  │  matrix.setCursor(2, 5)
  │  matrix.print("14:30:25")
  │  // 底部 7 条星期横线
  │  for i=1..7:
  │    if i==weekNum: continue
  │    matrix.drawFastHLine(2 + (i-1)*4, 7, 3, weekColor)
  │  matrix.drawFastHLine(2 + (weekNum-1)*4, 7, 3, mainColor)
  │
  ├── timePage == TIME_H_M ───────────────────────────────────────
  │  if (!tickerAnim.active())
  │    startTickerAnim()           // tickerAnim.attach_ms(200, showAnim)
  │  matrix.fillRect(12, 0, 19, 8, 0)  // 只清右侧 20 列
  │  // 格式化 HH:MM
  │  matrix.setCursor(14, 5)
  │  matrix.print("14:30")
  │  // 星期横线 (紧凑版: 线宽2, 间距3, 起点12)
  │  for i=1..7:
  │    if i==weekNum: continue
  │    matrix.drawFastHLine(12 + (i-1)*3, 7, 2, weekColor)
  │  matrix.drawFastHLine(12 + (weekNum-1)*3, 7, 2, mainColor)
  │
  ├── timePage == TIME_DATE ──────────────────────────────────────
  │  if (!tickerAnim.active())
  │    startTickerAnim()
  │  matrix.fillRect(12, 0, 19, 8, 0)
  │  // 格式化 MM-DD
  │  matrix.setCursor(12, 5)
  │  matrix.print("06-03")
  │  // 星期横线 (同 TIME_H_M)
  │  ...
  │
  matrix.show()                    // ★ 一次性刷新到 LED 矩阵
```

### 星期条取反色算法

`preferencesUtil.cpp:35-56`

```cpp
// weekColor 计算规则:
//   找出 R,G,B 中的最大值 maxRGB
//   最大值分量 → 保留原值
//   非最大值分量 → 用 255-c (取反色)
// 例: R=0, G=250, B=250 → max=G(250)
//     weekR = 255-0 = 255
//     weekG = 250 (保留)
//     weekB = 255-250 = 5
// 结果: 主题青色(0,250,250) → 星期条粉色(255,250,5)
```

---

## 1.4 动画帧链路

### 1.4.1 定时器启动

`task.cpp:235-238`

```cpp
void startTickerAnim() {
    tickerAnim.attach_ms(ANIM_INTERVAL, showAnim);  // 200ms 触发一次
}
```

### 1.4.2 动画帧切换

`task.cpp:139-183`

```cpp
void showAnim() {
    switch (animIndex) {
        case 0: matrix.drawRGBBitmap(0, 0, timeAnim0, 11, 8); break;
        case 1: matrix.drawRGBBitmap(0, 0, timeAnim1, 11, 8); break;
        case 2: matrix.drawRGBBitmap(0, 0, timeAnim2, 11, 8); break;
        case 3: matrix.drawRGBBitmap(0, 0, timeAnim3, 11, 8); break;
        case 4: matrix.drawRGBBitmap(0, 0, timeAnim4, 11, 8); break;
        case 5: matrix.drawRGBBitmap(0, 0, timeAnim5, 11, 8); break;
        case 6: matrix.drawRGBBitmap(0, 0, timeAnim6, 11, 8); break;
        case 7: matrix.drawRGBBitmap(0, 0, timeAnim7, 11, 8); break;
        case 8: matrix.drawRGBBitmap(0, 0, timeAnim8, 11, 8); break;
        case 9: matrix.drawRGBBitmap(0, 0, timeAnim0, 11, 8); break;  // 兜底
        default: animIndex = 9; break;
    }
    matrix.show();
    animIndex++;
    if (animIndex == 10) animIndex = 0;
}
```

位图数据: `src/img/timeAnim0.h` ~ `timeAnim8.h`，每帧 11×8 像素 RGB565 格式。

---

## 1.5 NTP 定时对时链路

`main.cpp:89-133` + `net.cpp:267-294`

```
tickerCheckTime.attach(18000, checkTime)   // 每 5 小时触发
  ↓
checkTime():                               // task.cpp:133
  isCheckingTime = true                    // ★ 只设标志位，不直接执行
  ↓
主循环 loop() 检测:                        // main.cpp:89
  if (isCheckingTime):
    // 1. 暂停动画
    bool stopAnim = false
    if (tickerAnim.active())
      tickerAnim.detach()
      stopAnim = true

    // 2. 显示 "对时中" 提示
    drawCheckTimeText()                    // light.cpp:76
      matrix.fillScreen(0)
      matrix.setBrightness(brightness)
      matrix.drawBitmap(0, 0, checkingTime, 32, 8, mainColor)
      matrix.show()

    // 3. 执行对时
    checkTimeTicker()                      // net.cpp:267
      WiFi.begin(ssid, pass)
      循环连接 (10s 超时)
        ├─ 超时 → wifiConnected=false → return
        └─ 成功:
            getNTPTime()                   // configTime(8*3600, 0, NTP1, NTP2, NTP3)
            if (clockOpen)                 // 重置闹钟
              tickerClock.detach()
              startTickerClock(getClockRemainSeconds())
            disConnectWifi()

    // 4. 清除标志
    isCheckingTime = false

    // 5. 强制等待至少 4 秒 (避免提示一闪而过)
    while ((millis() - start) < 4000)
      delay(200)

    // 6. 恢复动画
    if (stopAnim) startTickerAnim()

    // 7. 清屏并重绘当前页面
    clearMatrix()
    if (currentPage == TIME) drawTime()
```

---

## 1.6 TIME 页面按键链路

### BTN1 (GPIO3) 短按 — 子页面前向轮转

`task.cpp:280-309`

```
btn1click() → case TIME:
  TIME_H_M_S → timePage = TIME_H_M
  TIME_H_M   → timePage = TIME_DATE
  TIME_DATE  → tickerAnim.detach()
               timePage = TIME_H_M_S

  clearMatrix()
  recordExtensionPage()     // 写入 NVS
  drawTime()
```

### BTN2 (GPIO4) 短按 — 子页面反向轮转

`task.cpp:404-433`

```
btn2click() → case TIME:
  TIME_H_M_S → timePage = TIME_DATE
  TIME_H_M   → tickerAnim.detach()
               timePage = TIME_H_M_S
  TIME_DATE  → timePage = TIME_H_M

  clearMatrix()
  recordExtensionPage()
  drawTime()
```

### BTN3 (GPIO7) 短按 — 切换到节奏灯页面

`task.cpp:563-565`

```
btn3click() → case TIME:
  if (tickerAnim.active())
    tickerAnim.detach()     // 停止动画
  clearMatrix()
  currentPage = RHYTHM      // ★ 退出 TIME
```

### 任意按键长按 (非SETTING) — 重启配网

`task.cpp:735-740`

```
btn3LongClick() → 非 SETTING:
  setApConfigWhenStart(true)   // NVS 写 apConfig=true
  ESP.restart()                // ★ 重启进入配网流程
```

### 响铃期间 — 任意按键取消响铃

```
btn1click/btn2click/btn3click() 第一行:
  if (belling):
    cancelBell()               // vTaskDelete(bellTask) + belling=false
    return                     // 不执行原按键逻辑
```

---

# 二、CLOCK 页面完整实现链路

## 2.1 数据结构与状态变量

```cpp
// ===== light.h / light.cpp =====
bool clockOpen;              // 闹钟开关 (存 NVS)
int clockH, clockM;          // 闹钟实际设定值 (存 NVS)
int clockBellNum;            // 闹铃编号 0~4 (存 NVS)
int tmpClockH, tmpClockM;    // 编辑中的临时值
int tmpClockBellNum;         // 编辑中的临时铃声编号
int clockChoosed;            // 当前选中项: CLOCK_H=1, CLOCK_M=2, CLOCK_BELL=3
bool playingMusic;           // 是否正在预览铃声

// ===== common.h =====
const int CLOCK_H    = 1;
const int CLOCK_M    = 2;
const int CLOCK_BELL = 3;

// ===== task.h / task.cpp =====
TaskHandle_t bellTask;       // 响铃 FreeRTOS 任务句柄
TaskHandle_t playSongsTask;  // 预览播放任务句柄

// ===== buzzer.h / buzzer.cpp =====
int songCount = 5;           // 可选铃声数量
```

---

## 2.2 进入 CLOCK 页面

`task.cpp:582-594`

```
btn3click() → case ANIM:
  if (!RTCSuccess)
    currentPage = BRIGHT       // 无 RTC 则跳到亮度页
  else
    currentPage = CLOCK        // ★ 进入 CLOCK
    resetTmpClockData()        // light.cpp:602
      tmpClockH = clockH
      tmpClockM = clockM
      tmpClockBellNum = clockBellNum
      clockChoosed = CLOCK_H   // 默认选中 "时"
    drawClock()                // light.cpp:610
```

---

## 2.3 `drawClock()` 绘制链路

`light.cpp:610-647`

```
drawClock():
  matrix.fillScreen(0)

  ┌── clockOpen == true (闹钟开启) ──────────────────────────────
  │  matrix.setTextColor(mainColor)
  │  matrix.setCursor(2, 5)
  │  // 格式化 tmpClockH:tmpClockM (不足两位补 0)
  │  matrix.print("07:30")
  │  // 铃铛图标 8×6 @ (22, 1)
  │  matrix.drawBitmap(22, 1, bell, 8, 6, mainColor)
  │  // 底部选中指示线
  │  if (clockChoosed == CLOCK_H)   → drawFastHLine(2,  7, 7, weekColor)
  │  if (clockChoosed == CLOCK_M)   → drawFastHLine(12, 7, 7, weekColor)
  │  if (clockChoosed == CLOCK_BELL)→ drawFastHLine(22, 7, 8, weekColor)
  │
  └── clockOpen == false (闹钟关闭) ─────────────────────────────
     matrix.drawBitmap(8, 1, bell, 8, 6, mainColor)   // 铃铛居中
     matrix.setCursor(20, 6)
     matrix.setTextColor(matrix.Color(255, 0, 0))      // 红色
     matrix.print("X")                                  // 显示 X

  matrix.show()
```

---

## 2.4 CLOCK 页面按键链路

### BTN1 短按 — 数值增加 / 切换铃声

`task.cpp:350-386`

```
btn1click() → case CLOCK:
  if (!clockOpen) return       // 闹钟关闭时不操作

  if (clockChoosed == CLOCK_H):
    tmpClockH++
    if (tmpClockH == 24) tmpClockH = 0
    drawClock()

  if (clockChoosed == CLOCK_M):
    tmpClockM++
    if (tmpClockM == 60) tmpClockM = 0
    drawClock()

  if (clockChoosed == CLOCK_BELL):
    tmpClockBellNum++
    if (tmpClockBellNum == songCount) tmpClockBellNum = 0
    vTaskDelete(playSongsTask)     // 停止旧预览
    delay(300)
    createPlaySongsTask()          // 启动新课铃声预览
```

### BTN2 短按 — 数值减少 / 切换铃声

`task.cpp:474-509`

```
btn2click() → case CLOCK:
  if (!clockOpen) return

  if (clockChoosed == CLOCK_H):
    tmpClockH--
    if (tmpClockH == -1) tmpClockH = 23
    drawClock()

  if (clockChoosed == CLOCK_M):
    tmpClockM--
    if (tmpClockM == -1) tmpClockM = 59
    drawClock()

  if (clockChoosed == CLOCK_BELL):
    tmpClockBellNum--
    if (tmpClockBellNum == -1) tmpClockBellNum = songCount - 1
    vTaskDelete(playSongsTask)
    delay(300)
    createPlaySongsTask()
```

### BTN1 长按 — 选中项正向切换

`task.cpp:634-667`

```
btn1LongClick() → case CLOCK:
  if (!clockOpen) return
  if (clockChoosed == CLOCK_H):
    clockChoosed = CLOCK_M
    drawClock()
  if (clockChoosed == CLOCK_M):
    clockChoosed = CLOCK_BELL
    drawClock()
    createPlaySongsTask()      // 开始预览
    playingMusic = true
  if (clockChoosed == CLOCK_BELL):
    vTaskDelete(playSongsTask) // 停止预览
    delay(300)
    playingMusic = false
    clockChoosed = CLOCK_H
    drawClock()
```

### BTN2 长按 — 选中项反向切换

`task.cpp:668-701`

```
btn2LongClick() → case CLOCK:
  // 同上但方向相反
  CLOCK_H → CLOCK_BELL (开始预览)
  CLOCK_M → CLOCK_H
  CLOCK_BELL → CLOCK_M (停止预览)
```

### BTN3 短按 — 保存并退出到亮度页

`task.cpp:596-618`

```
btn3click() → case CLOCK:
  // 1. 确认保存
  clockH = tmpClockH
  clockM = tmpClockM
  clockBellNum = tmpClockBellNum
  recordClockPage()            // 写入 NVS
    prefs.putInt("clockH", clockH)
    prefs.putInt("clockM", clockM)
    prefs.putInt("clockBellNum", clockBellNum)
    prefs.putBool("clockOpen", clockOpen)

  // 2. 重置闹钟定时器
  tickerClock.detach()
  if (clockOpen)
    startTickerClock(getClockRemainSeconds())

  // 3. 停止铃声预览
  if (playingMusic)
    vTaskDelete(playSongsTask)
    delay(300)
    playingMusic = false

  // 4. 进入亮度页
  currentPage = BRIGHT
  drawBright()
```

### BTN3 长按 — 开关闹钟 / 切换铃声预览

`task.cpp:709-733`

```
btn3LongClick() → case CLOCK:
  if (clockChoosed == CLOCK_BELL):
    // Toggle 铃声预览
    if (playingMusic):
      vTaskDelete(playSongsTask)
      playingMusic = false
    else:
      createPlaySongsTask()
      playingMusic = true

  clockOpen = !clockOpen       // ★ 切换闹钟开关
  drawClock()
  recordClockPage()            // 立即存入 NVS
```

---

## 2.5 铃声播放链路

### 2.5.1 预览播放 (闹钟设置页)

`task.cpp:50-52` + `task.cpp:107-114`

```
createPlaySongsTask():
  xTaskCreate(startPlaySongs, 2048)
    ↓
startPlaySongs(void*):
  while (true):
    playSong(false)            // bell=false → 用 tmpClockBellNum
    vTaskDelay(3000)           // 间隔 3 秒循环
```

### 2.5.2 闹钟响铃 (到时间触发)

`task.cpp:45-48` + `task.cpp:117-125`

```
createBellTask():
  xTaskCreate(startPlayBell, 2048)
    ↓
startPlayBell(void*):
  while (true):
    playSong(true)             // bell=true → 用 clockBellNum
    vTaskDelay(3000)
```

### 2.5.3 音频播放核心

`buzzer.cpp:18-44`

```
playSong(bool bell):
  songNum = bell ? clockBellNum : tmpClockBellNum
  switch (songNum):
    case 0: playSong(SuperMario_note, SuperMario_duration, len, 2.1)
    case 1: playSong(AlwaysWithMe_note, AlwaysWithMe_duration, len, 3.0)
    case 2: playSong(DreamWedding_note, DreamWedding_duration, len, 2.7)
    case 3: playSong(CastleInTheSky_note, CastleInTheSky_duration, len, 2.8)
    case 4: playSong(Canon_note, Canon_duration, len, 1.8)
```

`buzzer.cpp:8-16`

```
playSong(note[], duration[], length, timeCoefficient):
  for thisNote in 0..length:
    noteDuration = 1000 / duration[thisNote]   // ms
    tone(BUZZER, note[thisNote], noteDuration) // Arduino PWM 输出到 GPIO10
    pauseBetweenNotes = noteDuration * timeCoefficient
    delay(pauseBetweenNotes)
    noTone(BUZZER)
```

5首曲谱定义在 `src/songs.h`:
- `0` — 超级马里奥 (SuperMario)
- `1` — Always With Me (千与千寻)
- `2` — 梦中的婚礼 (DreamWedding)
- `3` — 天空之城 (CastleInTheSky)
- `4` — 卡农 (Canon)

---

## 2.6 闹钟倒计时 → 触发链路

### 2.6.1 倒计时计算

`task.cpp:185-203`

```cpp
int32_t getClockRemainSeconds() {
    struct tm timeinfo;
    getLocalTime(&timeinfo);                              // RTC 当前时间

    int32_t nowTime   = (timeinfo.tm_hour * 60 + timeinfo.tm_min) * 60 + timeinfo.tm_sec;
    int32_t clockTime = (clockH * 60 + clockM) * 60;     // 闹钟设定时间 (秒)

    if (clockTime > nowTime)
        return clockTime - nowTime;                       // 当天触发
    else
        return clockTime + 86400 - nowTime;               // 明天触发
}
```

### 2.6.2 定时器设置

`task.cpp:230-233`

```cpp
void startTickerClock(int32_t seconds) {
    tickerClock.once(seconds, ringingBell);   // seconds 秒后执行一次回调
}
```

### 2.6.3 闹钟触发回调

`task.cpp:206-228`

```
ringingBell():
  // 1. 静默判断
  if (currentPage == SETTING || currentPage == CLOCK)
    return

  // 2. 清理当前状态
  if (currentPage == RHYTHM):
    memset(matrixArray, 0, sizeof(matrixArray))
    lightedCount = 0

  // 3. 强制切到 TIME
  currentPage = TIME

  // 4. 启动响铃
  createBellTask()                  // FreeRTOS 任务 → startPlayBell()
  belling = true

  // 5. 重置到明天同时间
  tickerClock.detach()
  startTickerClock(getClockRemainSeconds())
```

### 2.6.4 取消响铃

`task.cpp:246-251`

```
cancelBell():
  vTaskDelete(bellTask)             // 杀死响铃 FreeRTOS 任务
  delay(300)
  belling = false
```

所有按键处理函数第一行都会检查 `belling`，若为 `true` 则优先调用 `cancelBell()` 并 `return`。

---

## 2.7 闹钟重置的触发时机

闹钟定时器在以下场景中会被 `detach()` + `startTickerClock()` 重置：

| 触发场景 | 代码位置 | 说明 |
|----------|----------|------|
| 系统启动 NTP 成功后 | `main.cpp:67-69` | 首次设置闹钟 |
| NTP 定时对时成功后 | `net.cpp:287-290` | 防止时间漂移 |
| 用户在 CLOCK 页保存后 | `task.cpp:604-607` | 用户修改了闹钟时间 |
| 闹钟响铃触发后 | `task.cpp:226-227` | 重置到明天同时间 |

---

# 三、TIME ↔ CLOCK 关联关系

## 3.1 状态机流转图

```
                              ┌─────────┐
                    ┌────────►│ SETTING │◄────────┐
                    │         └────┬─────┘         │
                    │              │ btn3(配网成功)  │
                    │              ▼                 │
                    │         ┌─────────┐   长按重启  │
     btn3(切页)      │  NTP成功 │  TIME   │◄─────────┤
   ┌──────┴──────┐   ├────────►│         │          │
   │             │   │         └────┬────┘          │
   ▼             ▼   │              │ btn3          │
┌───────┐   ┌───────┐│              ▼               │
│RHYTHM │◄──│  ANIM ││    ┌─────────┐              │
└───────┘   └───┬───┘│    │  CLOCK  │              │
                │    │    └────┬────┘              │
                │    │         │ btn3              │
                │    │         ▼                   │
                │    │    ┌─────────┐              │
                │    └───►│ BRIGHT  │──────────────┘
                │         └─────────┘   btn3
                │
                └── btn3 ──► (无RTC时跳过CLOCK)
```

## 3.2 TIME ↔ CLOCK 耦合点

| 耦合点 | 方向 | 说明 |
|--------|------|------|
| `ringingBell()` | CLOCK → TIME | 闹钟到时强制切到 TIME 并响铃 |
| `checkTimeTicker()` | TIME → CLOCK | NTP 对时后重置闹钟定时器 |
| `NVS "matrix"` | 双向 | `timePage` 和 `clockH/M/BellNum` 共享 Preferences namespace |
| `cancelBell()` | CLOCK → TIME | 响铃期间任意按键停止铃声但不退出 TIME |
| `getClockRemainSeconds()` | CLOCK ← TIME | 依赖 ESP32 RTC 时间计算倒计时 |

## 3.3 页面切换时的初始化/清理

```
进入 TIME 需要:
  □ 停止 tickerAnim (如有)
  □ clearMatrix()
  □ prevDisplay = 0 (强制秒刷新)

退出 TIME 需要:
  □ 停止 tickerAnim (tickerAnim.detach())
  □ clearMatrix()

进入 CLOCK 需要:
  □ resetTmpClockData() (当前值 → 临时值)
  □ drawClock()

退出 CLOCK 需要:
  □ 保存 clockH, clockM, clockBellNum (NVS)
  □ 重置 tickerClock
  □ 停止 playSongsTask (如有)
```

---

# 四、NVS 存储结构

Preferences namespace: `"matrix"`

| Key | 类型 | 默认值 | 读写函数 |
|-----|------|--------|----------|
| `ssid` | String | `""` | `recordInfos()` / `getInfos()` |
| `pass` | String | `""` | `recordInfos()` / `getInfos()` |
| `red` | Int | `0` | `recordInfos()` / `getInfos()` |
| `green` | Int | `250` | `recordInfos()` / `getInfos()` |
| `blue` | Int | `250` | `recordInfos()` / `getInfos()` |
| `apConfig` | Bool | `true` | `setApConfigWhenStart()` / `getInfos()` |
| `brightness` | Int | `45` | `recordBrightness()` / `getInfos()` |
| `timePage` | Int | `TIME_H_M_S` | `recordExtensionPage()` / `getInfos()` |
| `rhythmPage` | Int | `RHYTHM_MODEL1` | `recordExtensionPage()` / `getInfos()` |
| `animPage` | Int | `ANIM_MODEL1` | `recordExtensionPage()` / `getInfos()` |
| `clockH` | Int | `0` | `recordClockPage()` / `getInfos()` |
| `clockM` | Int | `0` | `recordClockPage()` / `getInfos()` |
| `clockBellNum` | Int | `0` | `recordClockPage()` / `getInfos()` |
| `clockOpen` | Bool | `false` | `recordClockPage()` / `getInfos()` |

---

# 五、NTP 服务器配置

`common.h:30-32`

```cpp
#define NTP1 "ntp2.aliyun.com"    // 首选
#define NTP2 "ntp3.ict.ac.cn"     // 备用1
#define NTP3 "ntp4.ntsc.ac.cn"    // 备用2
```

时区: 东八区 `configTime(8 * 3600, 0, NTP1, NTP2, NTP3)`

---

# 六、引脚分配

`common.h:17-23`

| 引脚 | 功能 | 说明 |
|------|------|------|
| GPIO6 | LED 矩阵数据 | DATAPIN, WS2812B |
| GPIO3 | 按键1 | BTN1, 安全引脚避开 Strapping |
| GPIO4 | 按键2 | BTN2 |
| GPIO7 | 按键3 | BTN3, 安全引脚避开 Strapping |
| GPIO10 | 蜂鸣器 | BUZZER, 消除开机刺耳杂音 |
| GPIO1 | 拾音器 | AUDIO_IN_PIN, ADC |
| GPIO2 | 随机种子 | RANDOM_SEED_PIN, analogRead |
