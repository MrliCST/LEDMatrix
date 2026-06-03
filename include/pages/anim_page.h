#ifndef ANIM_PAGE_H
#define ANIM_PAGE_H
#include "pages/page_base.h"

/**
 * 动画页面 (ANIM)
 *
 * 脱离网络后的纯装饰动画，3 种动画可切换。
 *
 * 按键行为：
 *   短按 按键1: 动画 +1 (动画1→动画2→动画3→循环)
 *   短按 按键2: 动画 -1 (反向循环)
 *   短按 按键3: 前往 CLOCK 页面 (闹钟)
 *   长按 按键3: 重启进入配网模式
 *   其他长按: 无
 *
 * 3 种动画说明：
 *   动画1 ANIM_MODE_1: 黑客帝国风格数字雨滴下落 (使用 animHack 色板)
 *   动画2 ANIM_MODE_2: 全屏颜色渐变擦拭 (红→黄→绿→蓝→紫→循环)
 *   动画3 ANIM_MODE_3: 随机像素闪烁 (亮→灭→亮，数量渐变)
 *
 * 帧率：动画1 60ms/帧 ｜ 动画2 80ms/帧 ｜ 动画3 100ms/帧
 */
class AnimPage : public PageBase {
public:
    void enter(Page from) override;       // 进入时清屏 + 重置所有动画状态
    void update() override;               // 根据间隔控制帧率 → 调当前动画的 _draw
    void onBtn1Short() override;          // 动画 +1
    void onBtn2Short() override;          // 动画 -1
    void onBtn3Short() override;          // 前往 CLOCK 页面

private:
    unsigned long _lastFrame;             // 上一帧时间

    // ---- 动画1: 雨滴 ----
    uint8_t _raindropLines[32][13];       // 每列 13 个位置，存储该位置的颜色索引 (0=不亮)
    void _drawRaindrops();                // 画一帧雨滴
    void _resetRaindrops();               // 清空雨滴状态

    // ---- 动画2: 渐变擦拭 ----
    int _wipeStep;                        // 大步骤 0~4 (对应 红→黄→绿→蓝→紫)
    int _wipeSubStep;                     // 小步骤 0~49 (每次推进一列)
    int _wipeR, _wipeG, _wipeB;           // 当前过渡色 RGB
    void _drawColorWipe();                // 画一帧渐变擦拭
    void _resetColorWipe();               // 重置到初始状态

    // ---- 动画3: 闪烁 ----
    uint8_t _twinkleGrid[8][32];          // 每点是否点亮 (0/1)
    int _twinkleLitCount;                 // 当前点亮数量
    bool _twinkleIncreasing;              // 是否在增加中 (true=增加, false=减少)
    void _drawTwinkle();                  // 画一帧闪烁
    void _resetTwinkle();                 // 重置闪烁状态
};

void _cycleAnim(int step);                // 辅助函数: 循环切换动画子模式 (+1 或 -1)

#endif
