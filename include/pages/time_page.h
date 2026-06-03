#ifndef TIME_PAGE_H
#define TIME_PAGE_H
#include "pages/page_base.h"

/**
 * 时间显示页面 (TIME)
 *
 * NTP 对时成功后默认进入此页面，每秒刷新一次。
 *
 * 按键行为：
 *   短按 按键1: 显示格式 +1 (时:分:秒 → 时:分 → 日期 → 循环)
 *   短按 按键2: 显示格式 -1 (反向循环)
 *   短按 按键3: 前往 RHYTHM 页面 (节奏灯)
 *   长按 按键3: 重启进入配网模式
 *   其他长按: 无
 *
 * 3 种显示格式：
 *   TIME_FORMAT_H_M_S:  时:分:秒 (如 14:30:25)
 *   TIME_FORMAT_H_M:    时:分   (如 14:30)
 *   TIME_FORMAT_DATE:   日期   (如 06/03 周一)
 *
 * 每次切换格式后保存到 NVS。
 */
class TimePage : public PageBase {
public:
    void enter(Page from) override;
    void update() override;              // 每秒检查 time() → 变了就重画
    void onBtn1Short() override;         // 切换显示格式 (时:分:秒 → 时:分 → 日期)
    void onBtn2Short() override;         // 反向切换 (日期 → 时:分 → 时:分:秒)
    void onBtn3Short() override;         // 前往 RHYTHM 页面

private:
    time_t _prevDisplay = 0;
    void _drawTime();
};

#endif
