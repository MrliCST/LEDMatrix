#ifndef CLOCK_PAGE_H
#define CLOCK_PAGE_H
#include "pages/page_base.h"
#include <Ticker.h>

class ClockPage : public PageBase {
public:
    void enter(Page from) override;
    void exit() override;               // 离开时停止闹钟 Ticker
    void update() override;             // 检查闹钟到点标志
    void onBtn1Short() override;
    void onBtn2Short() override;
    void onBtn3Short() override;
    void onBtn1Long() override;
    void onBtn2Long() override;
    void onBtn3Long() override;

private:
    int _selection = CLOCK_SEL_H;
    int _tmpHour, _tmpMinute, _tmpBell;

    Ticker* _clockTicker = nullptr;       // 闹钟倒计时
    bool    _belling = false;             // 正在响铃中

    void _drawClock();
    void _drawDigit3x5(uint8_t digit, int x, int y, uint16_t color);
    void _drawColon(int x, int y, uint16_t color);
    void _saveAndApply();
    void _cancelBelling();
    static void _onAlarm(ClockPage* self);
};

#endif
