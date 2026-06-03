#ifndef BRIGHT_PAGE_H
#define BRIGHT_PAGE_H
#include "pages/page_base.h"

/**
 * 亮度调节页面 (BRIGHT)
 *
 * 静态页面，显示当前亮度值，支持步进调节。
 *
 * 按键行为：
 *   短按 按键1: 亮度 +5 (上限 BRIGHTNESS_MAX = 145)
 *   短按 按键2: 亮度 -5 (下限 BRIGHTNESS_MIN = 5)
 *   短按 按键3: 前往 TIME 页面 (时间)
 *   长按 按键3: 重启进入配网模式
 *   其他长按: 无
 *
 * 每次调节后立即保存到 NVS 并应用到 LED 矩阵。
 */
class BrightPage : public PageBase {
public:
    void enter(Page from) override;       // 进入时显示当前亮度值
    void update() override {}             // 静态页面，不需每帧刷新
    void onBtn1Short() override;          // 亮度 +5 (上限 145)
    void onBtn2Short() override;          // 亮度 -5 (下限 5)
    void onBtn3Short() override;          // 前往 TIME 页面

private:
    void _drawBright();                   // 显示当前亮度值 (数字或进度条)
};

#endif
