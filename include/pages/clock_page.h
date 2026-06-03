#ifndef CLOCK_PAGE_H
#define CLOCK_PAGE_H
#include "pages/page_base.h"

/**
 * 闹钟页面 (CLOCK)
 *
 * 设置闹钟时间、铃声、开关。三档选择：时 → 分 → 铃声 → 循环。
 *
 * 按键行为：
 *   短按 按键1: 当前选中项值 +1
 *               - 选中"时"   (CLOCK_SEL_H):    hour+1   (0~23 循环)
 *               - 选中"分"   (CLOCK_SEL_M):    minute+1 (0~59 循环)
 *               - 选中"铃声" (CLOCK_SEL_BELL): bellIndex+1 (0~N 循环)
 *   短按 按键2: 当前选中项值 -1 (反向)
 *   短按 按键3: 前往 BRIGHT 页面 (亮度)
 *
 *   长按 按键1 (1.2s): 切换选中项 (时 → 分 → 铃声 → 时 循环)
 *   长按 按键2 (1.2s): 反向切换选中项
 *   长按 按键3 (1.5s): 开/关闹钟 并保存到 NVS
 *                     - 关闭→开启: 开始计时，到达后触发 Buzzer
 *                     - 开启→关闭: 取消计时
 *
 * 限制条件：
 *   - 闹钟未开启时 (clockOpen=false)，短按按键1/2 不响应
 *   - 闹钟响铃期间 (belling=true)，任意按键首先取消响铃
 */
class ClockPage : public PageBase {
public:
    void enter(Page from) override;       // 从 Store 加载当前闹钟配置 → 显示
    void update() override;               // 静态页面，不需要每帧刷新 (检查响铃状态即可)
    void onBtn1Short() override;          // 选中项 +1 (闹钟未开启时不响应)
    void onBtn2Short() override;          // 选中项 -1 (闹钟未开启时不响应)
    void onBtn3Short() override;          // 前往 BRIGHT 页面
    void onBtn1Long() override;           // 切换选中项 (时→分→铃声→时)
    void onBtn2Long() override;           // 反向切换选中项
    void onBtn3Long() override;           // 开/关闹钟 → 保存 NVS

private:
    int _selection = CLOCK_SEL_H;         // 当前选中项 (CLOCK_SEL_H / _M / _BELL)
    int _tmpHour;                         // 临时编辑的时 (0~23)
    int _tmpMinute;                       // 临时编辑的分 (0~59)
    int _tmpBell;                         // 临时编辑的铃声编号

    void _drawClock();                    // 绘制闹钟设置界面 (选中项高亮/闪烁)
    void _saveAndApply();                 // 保存闹钟配置到 NVS + 设置倒计时
};

#endif
