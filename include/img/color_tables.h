// 律动页渐变色表
// 从原 light.cpp 的 model2ColorArray / model4ColorArrar 提取
#ifndef COLOR_TABLES_H
#define COLOR_TABLES_H

#include <Arduino.h>

// ---- Mode 2: 青→紫渐变色 (32列, 左青右紫) ----
// 用于 _drawGradientBars() 的柱体颜色映射
static const uint16_t model2ColorArray[] PROGMEM = {
    0x07FF, 0x06FF, 0x05FF, 0x04FF, // 列 0~3   青
    0x03FF, 0x02FF, 0x01FF, 0x00FF, // 列 4~7   青→蓝
    0x007F, 0x009F, 0x00BF, 0x00DF, // 列 8~11  蓝
    0x01DF, 0x03BF, 0x059F, 0x077F, // 列 12~15 蓝→紫
    0x0F5F, 0x1F3F, 0x2F1F, 0x3F0F, // 列 16~19 紫
    0x4F07, 0x5F03, 0x6F01, 0x7F00, // 列 20~23 紫→品红
    0x7E00, 0x7C00, 0x7A00, 0x7800, // 列 24~27 品红
    0x7801, 0x7803, 0x7805, 0x7807  // 列 28~31 品红→粉
};

// ---- Mode 4: 峰值点彩虹色 (12色循环) ----
// 用于 _drawPeaksOnly() 的峰值圆点颜色, 从低到高渐变
static const uint16_t model4ColorArrar[] PROGMEM = {
    0xF800, // 红
    0xFA00, // 橙
    0xFC00, // 金
    0xFF00, // 黄
    0x87E0, // 黄绿
    0x07E0, // 绿
    0x07FC, // 青
    0x00FC, // 蓝绿
    0x001F, // 蓝
    0x481F, // 蓝紫
    0x801F, // 紫
    0xF81F  // 粉
};

#define MODEL2_COLOR_COUNT  32
#define MODEL4_COLOR_COUNT  12

#endif
