### 2026/6/3  19:58 更新
本次更新引入了html，需要一个文件系统，当构建时，需要:
> platformioIO插件 -> quick access -> miscellaneous -> platformio core cli

进入命令行后执行：
```bash
pio run --target buildfs
```
以编译文件系统，之后再进行编译主体。

文件系统仅编译一次就可以，**前提是你没有向data中添加内容。**

### 2026/6/4 9:13 更新
本次更新，引入了连接 wokwi仿真器 虚拟wifi的功能，配置外部的桥架，实现连接互联网能力
同时，规范了代码的风格

各位成员可以去绘制页面了，可以先阅读 main.cpp。
页面的重绘逻辑一定要写到你负责页面 update 函数下，保证项目清晰。

### 2026/6/5 更新

**CLOCK 模块：**

- 闹钟保存语义修正：`_saveAndApply()` 中盲翻逻辑改为"调过数值则必定开启，未调才是 toggle"，消除调好时间点保存结果关了闹钟的困惑。
- 保存成功闪烁确认：开启闹钟后屏幕闪 3 次（明暗交替），防止用户以为没生效而二次长按导致关闭。
- 列优先渲染收拢：新增 `LedMatrix::drawColumnMajorBitmap()` 方法，bell/xmark 的手动双循环封装进 HAL 层；`MonoBitmap` 结构体标注列优先格式注释；修正 `IMG_XMARK.height` (7→5)。

**TIME 模块：**

- 确认现有 `update()` 驱动 `_drawTime()` 的模式正确，与 CLOCK 页面（事件驱动绘制）各适其用，CLOCK 不需要强行套用 `_dirty` 标志。