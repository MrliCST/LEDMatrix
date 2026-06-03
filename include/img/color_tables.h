#ifndef COLOR_TABLES_H
#define COLOR_TABLES_H

#include <Arduino.h>

// 从旧项目 light.cpp 提取的渐变色表

// RHYTHM_MODE_2 用：青→紫 双色渐变
// 两行分别代表渐变的起点和终点 RGB
static const int model2ColorArray[2][3] PROGMEM = {
    {0,   220, 255},   // 青色
    {240, 45,  255}    // 紫色
};

// RHYTHM_MODE_4 用：8 色渐变（暖色循环）
// 8 行，每行是一个中间色 RGB
static const int model4ColorArray[8][3] PROGMEM = {
    {240, 45,  255},   // 紫红
    {253, 98,  248},   // 粉红
    {253, 169, 205},   // 浅粉
    {255, 196, 123},   // 暖橙
    {253, 214, 200},   // 浅橙
    {253, 192, 255},   // 淡紫
    {249, 175, 255},   // 浅紫
    {0,   220, 255}    // 青色
};

#endif
