#include "pages/time_page.h"
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"
#include "net/ntp.h"
#include "img/img.h"
#include <time.h>

// 3x5 数字位图: 3字节/数字, bit0=顶行
static const uint8_t DIGIT[10][3] PROGMEM = {
    {0x0E, 0x11, 0x0E}, // 0
    {0x12, 0x1F, 0x10}, // 1
    {0x19, 0x15, 0x13}, // 2
    {0x11, 0x15, 0x1F}, // 3
    {0x07, 0x04, 0x1F}, // 4
    {0x17, 0x15, 0x1D}, // 5
    {0x0E, 0x15, 0x1D}, // 6
    {0x19, 0x05, 0x1F}, // 7
    {0x0A, 0x15, 0x0A}, // 8
    {0x12, 0x15, 0x0E}, // 9
};

static const uint8_t COLON_BMP[2] PROGMEM = {0x0A, 0x0A};
static const uint8_t DASH_BMP[3] PROGMEM = {0x04, 0x04, 0x04};

void TimePage::enter(Page from) {
    Serial.println("TimePage::enter");
    _prevDisplay = -1; // 确保首次 update() 必定触发 _drawTime
    LedMatrix::instance().clear();
    int m = Store::instance().mode().timeMode;
    // 动态能耗与事件分流设计，根据时间格式选择是否启动动画时钟
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
        // 每秒更新一次时间显示
        _prevDisplay = now;
        // 绘制时间
        _drawTime();
    }
}

void TimePage::_drawTime()
{
    auto &m = LedMatrix::instance();
    m.setBrightness(Store::instance().brightness()); // 设置亮度
    m.clear();
    // NTP对时加载等待图
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
    m.show(); //
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
    _drawDigit3x5(t->tm_hour / 10, x, y, mc);
    x += 4;
    _drawDigit3x5(t->tm_hour % 10, x, y, mc);
    x += 4;
    _drawColon(x, y, mc);
    x += 3;
    _drawDigit3x5(t->tm_min / 10, x, y, mc);
    x += 4;
    _drawDigit3x5(t->tm_min % 10, x, y, mc);
    x += 4;
    _drawColon(x, y, mc);
    x += 3;
    _drawDigit3x5(t->tm_sec / 10, x, y, mc);
    x += 4;
    _drawDigit3x5(t->tm_sec % 10, x, y, mc);

    _drawWeekday(3, 2, 7, t->tm_wday, mc, wc);
}

// ---- 子页面2: 11x8动画 + HH:MM ----
void TimePage::_drawTimeHM()
{
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    auto &m = LedMatrix::instance();
    uint16_t mc = Store::instance().color().mainColor;
    uint16_t wc = Store::instance().color().weekColor;

    m.drawRGBBitmap(0, 0, TIME_ANIM_FRAMES[_animFrame], 11, 8);

    int x = 14, y = 1;
    _drawDigit3x5(t->tm_hour / 10, x, y, mc);
    x += 4;
    _drawDigit3x5(t->tm_hour % 10, x, y, mc);
    x += 4;
    _drawColon(x, y, mc);
    x += 3;
    _drawDigit3x5(t->tm_min / 10, x, y, mc);
    x += 4;
    _drawDigit3x5(t->tm_min % 10, x, y, mc);

    _drawWeekday(2, 3, 7, t->tm_wday, mc, wc);
}

// ---- 子页面3: 11x8动画 + MM-DD ----
void TimePage::_drawTimeDate()
{
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    auto &m = LedMatrix::instance();
    uint16_t mc = Store::instance().color().mainColor;
    uint16_t wc = Store::instance().color().weekColor;

    m.drawRGBBitmap(0, 0, TIME_ANIM_FRAMES[_animFrame], 11, 8);

    int month = t->tm_mon + 1;
    int x = 12, y = 1;
    _drawDigit3x5(month / 10, x, y, mc);
    x += 4;
    _drawDigit3x5(month % 10, x, y, mc);
    x += 4;
    _drawDash(x, y, mc);
    x += 4;
    _drawDigit3x5(t->tm_mday / 10, x, y, mc);
    x += 4;
    _drawDigit3x5(t->tm_mday % 10, x, y, mc);

    _drawWeekday(2, 3, 7, t->tm_wday, mc, wc);
}

// ---- 星期指示线 ----
void TimePage::_drawWeekday(int barW, int gapW, int y, int wday, uint16_t mc, uint16_t wc)
{
    // wday: 0=Sun..6=Sat → 映射为周一=首个
    int today = (wday + 6) % 7; // Mon=0..Sun=6
    int x = 0;
    for (int i = 0; i < 7; i++)
    {
        uint16_t c = (i == today) ? mc : wc;
        LedMatrix::instance().fillRect(x, y, barW, 1, c);
        x += barW + gapW;
    }
}

// ---- 3x5 数字绘制 ----
void TimePage::_drawDigit3x5(uint8_t digit, int x, int y, uint16_t color)
{
    if (digit > 9)
        return;
    auto &m = LedMatrix::instance();
    for (int col = 0; col < 3; col++)
    {
        uint8_t mask = pgm_read_byte(&DIGIT[digit][col]);
        for (int row = 0; row < 5; row++)
        {
            if (mask & (1 << row))
                m.drawPixel(x + col, y + row, color);
        }
    }
}

void TimePage::_drawColon(int x, int y, uint16_t color)
{
    auto &m = LedMatrix::instance();
    for (int col = 0; col < 2; col++)
    {
        uint8_t mask = pgm_read_byte(&COLON_BMP[col]);
        for (int row = 0; row < 5; row++)
        {
            if (mask & (1 << row))
                m.drawPixel(x + col, y + row, color);
        }
    }
}

void TimePage::_drawDash(int x, int y, uint16_t color)
{
    auto &m = LedMatrix::instance();
    for (int col = 0; col < 3; col++)
    {
        uint8_t mask = pgm_read_byte(&DASH_BMP[col]);
        for (int row = 0; row < 5; row++)
        {
            if (mask & (1 << row))
                m.drawPixel(x + col, y + row, color);
        }
    }
}

// ---- 动画 Ticker ----
void TimePage::_startAnimTicker()
{
    if (_tickerAnim)
        return;
    _animFrame = 0;
    _tickerAnim = new Ticker();
    // 带参数的 Lambda 中断回调
    _tickerAnim->attach_ms(200, +[](TimePage *self)
                                { self->_animFrame = (self->_animFrame + 1) % TIME_ANIM_FRAME_COUNT; },
                           this); // 帧率环形缓冲器，每200ms更新一帧
}

// 销毁与内存释放函数
void TimePage::_stopAnimTicker()
{
    if (!_tickerAnim) // 事前防御（if (!ptr)）,_tickerAnim 是一个指向底层 Ticker（定时器）对象的指针。
        return;
    _tickerAnim->detach(); // 解除硬件依赖（detach()）,定时器不再发射中断信号
    delete _tickerAnim;    // 物理销毁（delete）
    _tickerAnim = nullptr; // 事后擦除（= nullptr）
}

// ---- 按钮 ----
void TimePage::onBtn1Short()
{
    int cur = Store::instance().mode().timeMode;
    cur = (cur == TIME_FORMAT_H_M_S) ? TIME_FORMAT_H_M
          : (cur == TIME_FORMAT_H_M) ? TIME_FORMAT_DATE
                                     : TIME_FORMAT_H_M_S;

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
    cur = (cur == TIME_FORMAT_H_M_S)  ? TIME_FORMAT_DATE
          : (cur == TIME_FORMAT_DATE) ? TIME_FORMAT_H_M
                                      : TIME_FORMAT_H_M_S;

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

void TimePage::_onAnimTick(TimePage *self)
{
    self->_animFrame = (self->_animFrame + 1) % TIME_ANIM_FRAME_COUNT;
}
