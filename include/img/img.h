#ifndef IMG_H
#define IMG_H

#include <pgmspace.h>
#include <Arduino.h>

// ---- 逐个引入位图数据 ----
#include "elems/animHack.h"        // const uint16_t animHack[]  — 数字雨色板 (6色)
#include "elems/bell.h"            // const uint8_t  bell[]      — 闹钟图标 (6×8 单色)
#include "elems/xmark.h"           // const uint8_t  xmark[]     — X 图标 (5×5 单色)
#include "elems/checkingTime.h"    // const uint8_t  checkingTime[] — 对时中提示 (32×8 单色)
#include "elems/timeAnim0.h"       // const uint16_t timeAnim0[] — 时间翻页帧0
#include "elems/timeAnim1.h"       // const uint16_t timeAnim1[] — 时间翻页帧1
#include "elems/timeAnim2.h"       // const uint16_t timeAnim2[] — 时间翻页帧2
#include "elems/timeAnim3.h"       // const uint16_t timeAnim3[] — 时间翻页帧3
#include "elems/timeAnim4.h"       // const uint16_t timeAnim4[] — 时间翻页帧4
#include "elems/timeAnim5.h"       // const uint16_t timeAnim5[] — 时间翻页帧5
#include "elems/timeAnim6.h"       // const uint16_t timeAnim6[] — 时间翻页帧6
#include "elems/timeAnim7.h"       // const uint16_t timeAnim7[] — 时间翻页帧7
#include "elems/timeAnim8.h"       // const uint16_t timeAnim8[] — 时间翻页帧8

// ---- 位图元信息 ----

// 注意：此数据为列优先格式，务必使用 drawColumnMajorBitmap 渲染
struct MonoBitmap {
    const uint8_t* data;
    int width;
    int height;
};

struct RgbBitmap {
    const uint16_t* data;
    int width;
    int height;
};

// 图标
static const MonoBitmap IMG_BELL = {
    bell, 6, 8
};

static const MonoBitmap IMG_XMARK = {
    xmark, 5, 5
};

static const MonoBitmap IMG_CHECKING_TIME = {
    checkingTime, 32, 8
};

// 数字雨色板（非位图，是 6 色调色板）
#define ANIM_HACK_COLOR_COUNT 6

// 时间翻页动画帧数
#define TIME_ANIM_FRAME_COUNT 9

// 时间翻页动画帧数组（按索引 0~8 访问）
static const uint16_t* const TIME_ANIM_FRAMES[TIME_ANIM_FRAME_COUNT] = {
    timeAnim0, timeAnim1, timeAnim2,
    timeAnim3, timeAnim4, timeAnim5,
    timeAnim6, timeAnim7, timeAnim8
};

#endif
