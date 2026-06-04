#ifndef RHYTHM_PAGE_H
#define RHYTHM_PAGE_H
#include "pages/page_base.h"

/**
 * 节奏灯页面 (RHYTHM)
 *
 * 麦克风实时采集环境声音，FFT 分频后以柱状图显示频谱。
 * 4 种可视化模式可切换。
 *
 * 按键行为：
 *   短按 按键1: 模式 +1 (模式1→模式2→模式3→模式4→循环)
 *   短按 按键2: 模式 -1 (反向循环)
 *   短按 按键3: 前往 ANIM 页面 (动画)
 *   长按 按键3: 重启进入配网模式
 *   其他长按: 无
 *
 * 4 种模式说明：
 *   模式1 RHYTHM_MODE_1: 彩虹柱 + 白色峰值点
 *   模式2 RHYTHM_MODE_2: 青→紫渐变柱 + 白色峰值点
 *   模式3 RHYTHM_MODE_3: 垂直渐变柱，无峰值点
 *   模式4 RHYTHM_MODE_4: 仅显示彩色峰值点，无柱体
 */
class RhythmPage : public PageBase {
public:
    void enter(Page from) override;       // 初始化
    void update() override;               // 采样 → FFT → 清屏 → 画柱 → show
    void onBtn1Short() override;          // 模式 +1
    void onBtn2Short() override;          // 模式 -1
    void onBtn3Short() override;          // 前往 ANIM 页面

private:
    int   _barHeights[32];                // 当前柱高度 (平滑后)
    float _peak[32];                      // 峰值顶点高度 (用于下落动画)
    unsigned long _lastPeakDecay;         // 上次峰值下落时间
    unsigned long _lastColorShift;        // 上次颜色偏移时间

    void _drawRainbowBars();              // 模式1: 彩虹柱
    void _drawGradientBars();             // 模式2: 青→紫渐变柱
    void _drawVerticalGradient();         // 模式3: 垂直渐变柱
    void _drawPeaksOnly();                // 模式4: 仅彩色峰值点
};

void _cycleRhythm(int step);              // 辅助函数: 循环切换子模式 (+1 或 -1)

#endif
