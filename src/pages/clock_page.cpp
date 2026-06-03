#include "pages/clock_page.h"
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"
#include "hal/buzzer.h"

void ClockPage::enter(Page from) {
    auto& c = Store::instance().clock();
    _tmpHour   = c.hour;
    _tmpMinute = c.minute;
    _tmpBell   = c.bellIndex;
    _selection = CLOCK_SEL_H;
    _drawClock();
}

void ClockPage::update() {
    // 静态页面，不需要每帧刷新
}

void ClockPage::onBtn1Short() {
    if (!Store::instance().clock().enabled) return;
    // CLOCK_SEL_H → ++hour / CLOCK_SEL_M → ++minute / CLOCK_SEL_BELL → next bell
    _drawClock();
}

void ClockPage::onBtn2Short() { /* 反向 */ }
void ClockPage::onBtn3Short() { StateMachine::instance().gotoPage(Page::BRIGHT); }

void ClockPage::onBtn1Long()       { /* 切换选中项 H→M→Bell→H */ _drawClock(); }
void ClockPage::onBtn2Long()       { /* 反向 */ _drawClock(); }
void ClockPage::onBtn3Long()       { /* toggle enabled → save */ _saveAndApply(); }

void ClockPage::_drawClock() {
    // 绘制闹钟设置界面
}

void ClockPage::_saveAndApply() {
    ClockConfig cfg = Store::instance().clock();
    cfg.hour      = _tmpHour;
    cfg.minute    = _tmpMinute;
    cfg.bellIndex = _tmpBell;
    cfg.enabled  ^= true;   // toggle
    Store::instance().saveClock(cfg);
    // 重新设置闹钟倒计时 Ticker ...
}
