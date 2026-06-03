#include "pages/clock_page.h"
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"
#include "hal/buzzer.h"
#include "net/ntp.h"
#include "img/img.h"
#include "songs/songs.h"

// 3x5 数字位图: 3字节/数字, bit0=顶行
static const uint8_t DIGIT[10][3] PROGMEM = {
    {0x0E,0x11,0x0E}, // 0
    {0x12,0x1F,0x10}, // 1
    {0x19,0x15,0x13}, // 2
    {0x11,0x15,0x1F}, // 3
    {0x07,0x04,0x1F}, // 4
    {0x17,0x15,0x1D}, // 5
    {0x0E,0x15,0x1D}, // 6
    {0x19,0x05,0x1F}, // 7
    {0x0A,0x15,0x0A}, // 8
    {0x12,0x15,0x0E}, // 9
};

static const uint8_t COLON_BMP[2] PROGMEM = {0x0A, 0x0A};

void ClockPage::enter(Page from) {
    auto& c = Store::instance().clock();
    _tmpHour   = c.hour;
    _tmpMinute = c.minute;
    _tmpBell   = c.bellIndex;
    _selection = CLOCK_SEL_H;
    _drawClock();
}

void ClockPage::update() {}

void ClockPage::onBtn1Short() {
    if (!Store::instance().clock().enabled) return;

    switch (_selection) {
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

void ClockPage::onBtn2Short() {
    if (!Store::instance().clock().enabled) return;

    switch (_selection) {
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

void ClockPage::onBtn3Short() {
    StateMachine::instance().gotoPage(Page::BRIGHT);
}

void ClockPage::onBtn1Long() {
    if (!Store::instance().clock().enabled) return;

    if (_selection == CLOCK_SEL_BELL)
        Buzzer::instance().stop();

    _selection = (_selection == CLOCK_SEL_H)   ? CLOCK_SEL_M
               : (_selection == CLOCK_SEL_M)   ? CLOCK_SEL_BELL
               :                                  CLOCK_SEL_H;

    if (_selection == CLOCK_SEL_BELL)
        Buzzer::instance().playSong(_tmpBell, false);

    _drawClock();
}

void ClockPage::onBtn2Long() {
    if (!Store::instance().clock().enabled) return;

    if (_selection == CLOCK_SEL_BELL)
        Buzzer::instance().stop();

    _selection = (_selection == CLOCK_SEL_H)   ? CLOCK_SEL_BELL
               : (_selection == CLOCK_SEL_M)   ? CLOCK_SEL_H
               :                                  CLOCK_SEL_M;

    if (_selection == CLOCK_SEL_BELL)
        Buzzer::instance().playSong(_tmpBell, false);

    _drawClock();
}

void ClockPage::onBtn3Long() {
    Buzzer::instance().stop();
    _saveAndApply();
}

void ClockPage::_drawClock() {
    auto& m = LedMatrix::instance();
    m.setBrightness(Store::instance().brightness());
    m.clear();

    uint16_t mc = Store::instance().color().mainColor;
    uint16_t wc = Store::instance().color().weekColor;

    if (!Store::instance().clock().enabled) {
        m.drawBitmap(13, 1, IMG_BELL.data, IMG_BELL.width, IMG_BELL.height, mc);
        m.setCursor(22, 1);
        m.setTextColor(m.color(255, 0, 0));
        m.print("X");
        m.show();
        return;
    }

    int x = 2, y = 1;
    _drawDigit3x5(_tmpHour / 10, x, y, mc); x += 4;
    _drawDigit3x5(_tmpHour % 10, x, y, mc); x += 4;
    _drawColon(x, y, mc); x += 3;
    _drawDigit3x5(_tmpMinute / 10, x, y, mc); x += 4;
    _drawDigit3x5(_tmpMinute % 10, x, y, mc);

    m.drawBitmap(23, 1, IMG_BELL.data, IMG_BELL.width, IMG_BELL.height, mc);

    switch (_selection) {
        case CLOCK_SEL_H:    m.fillRect(2,  7, 9, 1, wc); break;
        case CLOCK_SEL_M:    m.fillRect(13, 7, 9, 1, wc); break;
        case CLOCK_SEL_BELL: m.fillRect(23, 7, 8, 1, wc); break;
    }

    m.show();
}

void ClockPage::_saveAndApply() {
    ClockConfig cfg = Store::instance().clock();
    cfg.hour      = _tmpHour;
    cfg.minute    = _tmpMinute;
    cfg.bellIndex = _tmpBell;
    cfg.enabled   = !cfg.enabled;
    Store::instance().saveClock(cfg);

    Ntp::instance().stopClockTicker();
    if (cfg.enabled) {
        int32_t sec = Ntp::secondsUntilAlarm(cfg.hour, cfg.minute);
        Ntp::instance().startClockTicker(sec);
    }

    _drawClock();
}

void ClockPage::_drawDigit3x5(uint8_t digit, int x, int y, uint16_t color) {
    if (digit > 9) return;
    auto& m = LedMatrix::instance();
    for (int col = 0; col < 3; col++) {
        uint8_t mask = pgm_read_byte(&DIGIT[digit][col]);
        for (int row = 0; row < 5; row++) {
            if (mask & (1 << row)) m.drawPixel(x + col, y + row, color);
        }
    }
}

void ClockPage::_drawColon(int x, int y, uint16_t color) {
    auto& m = LedMatrix::instance();
    for (int col = 0; col < 2; col++) {
        uint8_t mask = pgm_read_byte(&COLON_BMP[col]);
        for (int row = 0; row < 5; row++) {
            if (mask & (1 << row)) m.drawPixel(x + col, y + row, color);
        }
    }
}
