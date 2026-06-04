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