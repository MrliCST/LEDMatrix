#include "pages/time_page.h"
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"
#include "net/ntp.h"
#include "img/img.h"
#include <time.h>

void TimePage::enter(Page from)
{
    Serial.println("TimePage::enter");
    _prevDisplay = -1;
    LedMatrix::instance().clear();
    int m = Store::instance().mode().timeMode;
    if (m == TIME_FORMAT_H_M || m == TIME_FORMAT_DATE)
        _startAnimTicker();
    else
        _stopAnimTicker();
}

void TimePage::exit() { _stopAnimTicker(); }

void TimePage::update()
{
    time_t now;
    time(&now);
    if (now != _prevDisplay)
    {
        _prevDisplay = now;
        _drawTime();
    }
}

void TimePage::_drawTime()
{
    auto &m = LedMatrix::instance();
    m.setBrightness(Store::instance().brightness());
    m.clear();
    if (!Ntp::instance().isOk())
    {
        m.drawBitmap(0, 0, IMG_CHECKING_TIME.data,
                     IMG_CHECKING_TIME.width, IMG_CHECKING_TIME.height,
                     Store::instance().color().mainColor);
        m.show();
        return;
    }

    switch (Store::instance().mode().timeMode)
    {
    case TIME_FORMAT_H_M_S:
        _drawTimeHMS();
        break;
    case TIME_FORMAT_H_M:
        _drawTimeHM();
        break;
    default:
        _drawTimeDate();
        break;
    }
    m.show();
}

// ---- 子页面1: HH:MM:SS (x=2起, 30px) ----
void TimePage::_drawTimeHMS()
{
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    auto &m = LedMatrix::instance();
    uint16_t mc = Store::instance().color().mainColor;
    uint16_t wc = Store::instance().color().weekColor;

    int x = 2, y = 1;
    LedMatrix::instance().drawDigit3x5(t->tm_hour / 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawDigit3x5(t->tm_hour % 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawColon(x, y, mc);
    x += 3;
    LedMatrix::instance().drawDigit3x5(t->tm_min / 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawDigit3x5(t->tm_min % 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawColon(x, y, mc);
    x += 3;
    LedMatrix::instance().drawDigit3x5(t->tm_sec / 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawDigit3x5(t->tm_sec % 10, x, y, mc);

    _drawWeekday(3, 2, 7, t->tm_wday, mc, wc);
}

// ---- 子页面2: 11x8动画 + HH:MM ----
void TimePage::_drawTimeHM()
{
    Serial.println("11x8动画 + HH:MM::enter");
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    auto &m = LedMatrix::instance();
    uint16_t mc = Store::instance().color().mainColor;
    uint16_t wc = Store::instance().color().weekColor;

    m.drawRGBBitmap(0, 0, TIME_ANIM_FRAMES[_animFrame], 11, 8);

    int x = 14, y = 1;
    LedMatrix::instance().drawDigit3x5(t->tm_hour / 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawDigit3x5(t->tm_hour % 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawColon(x, y, mc);
    x += 3;
    LedMatrix::instance().drawDigit3x5(t->tm_min / 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawDigit3x5(t->tm_min % 10, x, y, mc);

    _drawWeekday(2, 3, 7, t->tm_wday, mc, wc);
}

// ---- 子页面3: 11x8动画 + MM-DD ----
void TimePage::_drawTimeDate()
{
    Serial.println("11x8动画 + MM-DD::enter");
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    auto &m = LedMatrix::instance();
    uint16_t mc = Store::instance().color().mainColor;
    uint16_t wc = Store::instance().color().weekColor;

    m.drawRGBBitmap(0, 0, TIME_ANIM_FRAMES[_animFrame], 11, 8);

    int month = t->tm_mon + 1;
    int x = 12, y = 1;
    LedMatrix::instance().drawDigit3x5(month / 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawDigit3x5(month % 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawDash(x, y, mc);
    x += 4;
    LedMatrix::instance().drawDigit3x5(t->tm_mday / 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawDigit3x5(t->tm_mday % 10, x, y, mc);

    _drawWeekday(2, 3, 7, t->tm_wday, mc, wc);
}

void TimePage::_drawWeekday(int barW, int gapW, int y, int wday, uint16_t mc, uint16_t wc)
{
    int today = (wday + 6) % 7;
    int x = 0;
    for (int i = 0; i < 7; i++)
    {
        uint16_t c = (i == today) ? mc : wc;
        LedMatrix::instance().fillRect(x, y, barW, 1, c);
        x += barW + gapW;
    }
}

void TimePage::_startAnimTicker()
{
    if (_tickerAnim)
        return;
    _animFrame = 0;
    _tickerAnim = new Ticker();
    _tickerAnim->attach_ms(200, +[](TimePage *self)
                                { self->_animFrame = (self->_animFrame + 1) % TIME_ANIM_FRAME_COUNT; },
                           this);
}

void TimePage::_stopAnimTicker()
{
    if (!_tickerAnim)
        return;
    _tickerAnim->detach();
    delete _tickerAnim;
    _tickerAnim = nullptr;
}

void TimePage::onBtn1Short()
{
    int cur = Store::instance().mode().timeMode;
    switch (cur)
    {
    case TIME_FORMAT_H_M_S:
        cur = TIME_FORMAT_H_M;
        break;
    case TIME_FORMAT_H_M:
        cur = TIME_FORMAT_DATE;
        break;
    default:
        cur = TIME_FORMAT_H_M_S;
        break;
    }

    ModeConfig cfg = Store::instance().mode();
    cfg.timeMode = cur;
    Store::instance().saveMode(cfg);

    LedMatrix::instance().clear();
    _prevDisplay = -1;

    if (cur == TIME_FORMAT_H_M || cur == TIME_FORMAT_DATE)
        _startAnimTicker();
    else
        _stopAnimTicker();
}

void TimePage::onBtn2Short()
{
    int cur = Store::instance().mode().timeMode;
    switch (cur)
    {
    case TIME_FORMAT_H_M_S:
        cur = TIME_FORMAT_DATE;
        break;
    case TIME_FORMAT_DATE:
        cur = TIME_FORMAT_H_M;
        break;
    default:
        cur = TIME_FORMAT_H_M_S;
        break;
    }

    ModeConfig cfg = Store::instance().mode();
    cfg.timeMode = cur;
    Store::instance().saveMode(cfg);

    LedMatrix::instance().clear();
    _prevDisplay = -1;

    if (cur == TIME_FORMAT_H_M || cur == TIME_FORMAT_DATE)
        _startAnimTicker();
    else
        _stopAnimTicker();
}

void TimePage::onBtn3Short()
{
    StateMachine::instance().gotoPage(Page::RHYTHM);
}
