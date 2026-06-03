### 2026/6/3  19:58 更新
本次更新引入了html，需要一个文件系统，当构建时，需要:
> platformioIO插件 -> quick access -> miscellaneous -> platformio core cli

进入命令行后执行：
```bash
pio run --target buildfs
```
以编译文件系统，之后再进行编译主体。

文件系统仅编译一次就可以，**前提是你没有向data中添加内容。**