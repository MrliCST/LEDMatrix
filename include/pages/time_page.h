#ifndef TIME_PAGE_H
#define TIME_PAGE_H
#include "pages/page_base.h"
#include <Ticker.h>

class TimePage : public PageBase {
public:
    void enter(Page from) override;
    void exit() override;
    void update() override;
    void onBtn1Short() override;
    void onBtn2Short() override;
    void onBtn3Short() override;

private:
    time_t _prevDisplay = 0;
    uint8_t _animFrame = 0;
    Ticker* _tickerAnim = nullptr;

    void _drawTime();
    void _drawTimeHMS();
    void _drawTimeHM();
    void _drawTimeDate();
    void _drawWeekday(int barW, int gapW, int y, int wday, uint16_t mc, uint16_t wc);
    void _drawDigit3x5(uint8_t digit, int x, int y, uint16_t color);
    void _drawColon(int x, int y, uint16_t color);
    void _drawDash(int x, int y, uint16_t color);
    void _startAnimTicker();
    void _stopAnimTicker();

    static void _onAnimTick(TimePage* self);
};

#endif
