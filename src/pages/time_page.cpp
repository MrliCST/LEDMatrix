#include "pages/time_page.h"
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"

void TimePage::enter(Page from) {
    _prevDisplay = 0;  // 强制立即画一次
    LedMatrix::instance().clear();
}

void TimePage::update() {
    time_t now;
    time(&now);
    if (now != _prevDisplay) {
        _prevDisplay = now;
        _drawTime();
    }
}

void TimePage::_drawTime() {
    auto& m = LedMatrix::instance();
    m.setBrightness(Store::instance().brightness());

    int mode = Store::instance().mode().timeMode;
    if (mode == TIME_FORMAT_H_M_S) {
        // 画 时:分:秒
    } else if (mode == TIME_FORMAT_H_M) {
        // 画 时:分
    } else {
        // 画 日期
    }
    m.show();
}

void TimePage::onBtn1Short() {
    int cur = Store::instance().mode().timeMode;
    cur = (cur == TIME_FORMAT_H_M_S) ? TIME_FORMAT_H_M
        : (cur == TIME_FORMAT_H_M)   ? TIME_FORMAT_DATE
        :                               TIME_FORMAT_H_M_S;

    ModeConfig cfg = Store::instance().mode();
    cfg.timeMode = cur;
    Store::instance().saveMode(cfg);

    LedMatrix::instance().clear();
    _prevDisplay = 0;
}

void TimePage::onBtn2Short() { /* 反向同逻辑 */ }

void TimePage::onBtn3Short() {
    StateMachine::instance().gotoPage(Page::RHYTHM);
}
