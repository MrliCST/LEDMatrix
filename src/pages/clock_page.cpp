#include "pages/clock_page.h"
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"
#include "hal/buzzer.h"
#include "net/ntp.h"
#include "img/img.h"
#include "songs/songs.h"

void ClockPage::enter(Page from)
{
    Serial.println("Clock::enter");

    auto &c = Store::instance().clock();
    // 拷贝时钟数据
    _tmpHour = c.hour;
    _tmpMinute = c.minute;
    _tmpBell = c.bellIndex;
    // 光标锚定
    _selection = CLOCK_SEL_H;
    _drawClock();
}

void ClockPage::exit()
{
    if (_clockTicker)
    {
        _clockTicker->detach();
        delete _clockTicker;
        _clockTicker = nullptr;
    }
    _cancelBelling();
}

void ClockPage::update() {}

void ClockPage::_cancelBelling()
{
    if (_belling)
    {
        Buzzer::instance().stop();
        _belling = false;
    }
}
// 响铃
void ClockPage::_onAlarm(ClockPage *self)
{
    self->_belling = true;
    Buzzer::instance().playSong(Store::instance().clock().bellIndex, true);
}

// 第一个按钮短按（+）
void ClockPage::onBtn1Short()
{
    if (_belling) // 掐断闹钟
    {
        _cancelBelling();
        return;
    }
    if (!Store::instance().clock().enabled)
        return;

    switch (_selection)
    {
    case CLOCK_SEL_H:
        _tmpHour = (_tmpHour + 1) % 24;
        break;
    case CLOCK_SEL_M:
        _tmpMinute = (_tmpMinute + 1) % 60;
        break;
    case CLOCK_SEL_BELL:
        _tmpBell = (_tmpBell + 1) % SONG_COUNT;
        Buzzer::instance().stop();
        Buzzer::instance().playSong(_tmpBell, false);
        break;
    }
    _drawClock();
}

void ClockPage::onBtn2Short()
{
    if (_belling)
    {
        _cancelBelling();
        return;
    }
    if (!Store::instance().clock().enabled)
        return;

    switch (_selection)
    {
    case CLOCK_SEL_H:
        _tmpHour = (_tmpHour == 0) ? 23 : _tmpHour - 1;
        break;
    case CLOCK_SEL_M:
        _tmpMinute = (_tmpMinute == 0) ? 59 : _tmpMinute - 1;
        break;
    case CLOCK_SEL_BELL:
        _tmpBell = (_tmpBell == 0) ? SONG_COUNT - 1 : _tmpBell - 1;
        Buzzer::instance().stop();
        Buzzer::instance().playSong(_tmpBell, false);
        break;
    }
    _drawClock();
}
// 按键3短按，跳转到亮度设置页面
void ClockPage::onBtn3Short()
{
    if (_belling)
    {
        _cancelBelling();
        return;
    }
    StateMachine::instance().gotoPage(Page::BRIGHT);
}

void ClockPage::onBtn1Long()
{
    if (_belling)
    {
        _cancelBelling();
        return;
    }
    if (!Store::instance().clock().enabled)
        return;

    if (_selection == CLOCK_SEL_BELL)
        Buzzer::instance().stop();

    _selection = (_selection == CLOCK_SEL_H)   ? CLOCK_SEL_M
                 : (_selection == CLOCK_SEL_M) ? CLOCK_SEL_BELL
                                               : CLOCK_SEL_H;

    if (_selection == CLOCK_SEL_BELL)
        Buzzer::instance().playSong(_tmpBell, false);

    _drawClock();
}

void ClockPage::onBtn2Long()
{
    if (_belling)
    {
        _cancelBelling();
        return;
    }
    if (!Store::instance().clock().enabled)
        return;

    if (_selection == CLOCK_SEL_BELL)
        Buzzer::instance().stop();

    _selection = (_selection == CLOCK_SEL_H)   ? CLOCK_SEL_BELL
                 : (_selection == CLOCK_SEL_M) ? CLOCK_SEL_H
                                               : CLOCK_SEL_M;

    if (_selection == CLOCK_SEL_BELL)
        Buzzer::instance().playSong(_tmpBell, false);

    _drawClock();
}
// 长按按钮3，闹钟生效
void ClockPage::onBtn3Long()
{
    if (_belling)
    {
        _cancelBelling();
        return;
    }
    Buzzer::instance().stop();
    _saveAndApply();
}

void ClockPage::_drawClock()
{
    // 打印串口信息
    Serial.println("ClockPage::_drawClock");
    auto &m = LedMatrix::instance();
    m.setBrightness(Store::instance().brightness());
    m.clear(); // 清空屏幕->全黑

    uint16_t mc = Store::instance().color().mainColor; // 主色（数字、铃铛）
    uint16_t wc = Store::instance().color().weekColor; //  辅助色（光标条）

    // 时钟未启用，显示🔔X
    if (!Store::instance().clock().enabled)
    {
        // 打印串口信息显示🔔X
        Serial.println("ClockPage::_drawClock: clock not enabled");
        // m.drawBitmap(13, 1, IMG_BELL.data, IMG_BELL.width, IMG_BELL.height, mc);
        // m.drawBitmap(21, 1, IMG_XMARK.data, IMG_XMARK.width, IMG_XMARK.height, m.color(255, 0, 0));
        auto &m = LedMatrix::instance();
        m.clear(); // 物理清空画布

        uint16_t mc = Store::instance().color().mainColor;
        uint16_t redColor = m.color(255, 0, 0); // 纯红色
        // 1. 手工渲染【选项B：复古小铃铛】 (X起点: 9, Y起点: 0) -> 占据 X: 9~14, Y: 0~7
        for (int col = 0; col < 6; col++)
        {
            uint8_t mask = pgm_read_byte(&bell[col]);
            for (int row = 0; row < 8; row++)
            {
                if (mask & (1 << row))
                {
                    m.drawPixel(9 + col, 0 + row, mc);
                }
            }
        }
        // 2. 精准手工渲染红色 XMARK (X起点: 19, Y起点: 1) -> 占据 X: 19~23, Y: 1~5 (高度完美对齐)
        for (int col = 0; col < 5; col++)
        {
            uint8_t mask = pgm_read_byte(&xmark[col]);
            for (int row = 0; row < 5; row++)
            {
                if (mask & (1 << row))
                {
                    m.drawPixel(19 + col, 1 + row, redColor);
                }
            }
        }
        m.show();
        return;
    }
    // 时钟启用，显示时间
    // 打印串口信息显示时间
    Serial.println("ClockPage::_drawClock: clock enabled");

    int x = 2, y = 1;
    LedMatrix::instance().drawDigit3x5(_tmpHour / 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawDigit3x5(_tmpHour % 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawColon(x, y, mc);
    x += 3;
    LedMatrix::instance().drawDigit3x5(_tmpMinute / 10, x, y, mc);
    x += 4;
    LedMatrix::instance().drawDigit3x5(_tmpMinute % 10, x, y, mc);

    for (int col = 0; col < IMG_BELL.width; col++)
    {
        uint8_t mask = pgm_read_byte(&IMG_BELL.data[col]);
        for (int row = 0; row < IMG_BELL.height; row++)
        {
            if (mask & (1 << row))
                m.drawPixel(23 + col, 1 + row, mc);
        }
    }

    // 移动光标到当前选择项
    switch (_selection)
    {
    case CLOCK_SEL_H:
        // 打印串口信息显示选择小时
        Serial.println("ClockPage::_drawClock: clock selection hour");
        m.fillRect(2, 7, 9, 1, wc);
        break;
    case CLOCK_SEL_M:
        // 打印串口信息显示选择分钟
        Serial.println("ClockPage::_drawClock: clock selection minute");
        m.fillRect(13, 7, 9, 1, wc);
        break;
    case CLOCK_SEL_BELL:
        // 打印串口信息显示选择铃声
        Serial.println("ClockPage::_drawClock: clock selection bell");
        m.fillRect(23, 7, 8, 1, wc);
        break;
    }

    m.show();
}
// 闹钟保存生效
void ClockPage::_saveAndApply()
{
    ClockConfig cfg = Store::instance().clock();
    cfg.hour = _tmpHour;
    cfg.minute = _tmpMinute;
    cfg.bellIndex = _tmpBell;
    cfg.enabled = !cfg.enabled;
    Store::instance().saveClock(cfg);

    // 停止旧的倒计时
    if (_clockTicker)
    {
        _clockTicker->detach();
        delete _clockTicker;
        _clockTicker = nullptr;
    }

    if (cfg.enabled)
    {
        int32_t sec = Ntp::secondsUntilAlarm(cfg.hour, cfg.minute); //
        _clockTicker = new Ticker();
        // 匿名函数，+[]表示捕获this指针，过sec秒后中断执行_onAlarm函数
        _clockTicker->once(sec, +[](ClockPage *self)
                                { self->_onAlarm(self); },
                           this);
    }

    _drawClock();
}

