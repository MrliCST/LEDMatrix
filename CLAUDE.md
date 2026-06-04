# EasyMatrix 设计原则

## 代码风格
* **尽早退出**: 避免写成 if嵌套if，那样的面条代码，可读性太差
* **函数风格**: 第一个大括号和函数的 ()对齐，而不是另起一行

## 架构模式

### 单一职责
每个类只做一件事：
- `Store` — 只管全局配置的读写和 NVS 持久化
- `LedMatrix` — 只管画屏，封装 Adafruit_NeoMatrix
- `Buzzer` — 只管发声，不关心曲目逻辑
- `Microphone` — 只管采样和 FFT，输出频段数据
- `Buttons` — 只管按键轮询和事件触发
- `StateMachine` — 只管页面生命周期和事件分发
- 各 `XxxPage` — 只管本页面的绘制和交互逻辑

### 依赖倒置
- 页面只依赖 `PageBase` 接口和 HAL 层封装，不直接碰硬件寄存器
- 页面开发者不接触 `Adafruit_NeoMatrix`、`arduinoFFT`、`OneButton` 等第三方库 API，只调 HAL 层封装

### 开闭原则
- 新增页面只需新建一个 `PageBase` 子类并在 `StateMachine::init()` 注册
- 新增可视化模式只需在对应 Page 内部添加 `case` 分支

### 最少知识（迪米特法则）
- 页面只通过 `Store::instance()` 读写配置，不自己操作 NVS
- 页面只通过 `LedMatrix::instance()` 画屏，不拿 `raw()` 直接操作底层库
- 每个模块对其他模块的内部实现细节零依赖

## 实现约束

### 单例模式
所有模块通过**函数内 static 局部变量**实现线程安全的懒汉式单例：
```cpp
static Xxx& instance() { static Xxx m; return m; }
```
构造函数为 `private`，通过 `= default` 声明。

### 页面状态机
- 按键逻辑不在状态机里，`StateMachine` 只做分发
- 每个页面对按键的响应在各自类的 `onBtn1Short()` / `onBtn1Long()` 等方法中实现，天然解耦
- 页面间跳转通过 `StateMachine::instance().gotoPage(Page::XXX)` 完成

### 硬件抽象层 (HAL)
- 所有硬件操作封装在 `hal/` 目录下的单例类中
- 页面代码不 `#include` 任何第三方硬件库头文件
- HAL 层提供最简接口：初始化 (`begin`)、数据获取、输出执行

### 全局数据仓库
- 所有需要持久化的配置集中存储在 `Store` 单例中：
  - WiFi 配置（SSID、密码）
  - 颜色配置（RGB 三通道）
  - 模式配置（时间格式、律动模式、动画模式）
  - 闹钟配置（时、分、铃铛索引、启用状态）
  - 亮度值
- `Store` 底层使用 NVS 持久化，外部无感

## 代码风格

- 新文件不写注释，除非逻辑的 WHY 不显而易见
- 不写多行 docstring 或注释块，函数签名 + 命名即文档
- 编辑现有文件优先用 Edit（不重写整个文件）
- 不引入项目不需要的抽象或过度设计
- 三个相似的写法 > 一个过早的抽象