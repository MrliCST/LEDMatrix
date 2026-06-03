#include "pages/anim_page.h"
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"
#include <img/img.h>

void _cycleAnim(int step) {
    int cur = Store::instance().mode().animMode;
    cur += step;
    if (cur < ANIM_MODE_1) cur = ANIM_MODE_3;
    if (cur > ANIM_MODE_3) cur = ANIM_MODE_1;

    ModeConfig cfg = Store::instance().mode();
    cfg.animMode = cur;
    Store::instance().saveMode(cfg);
}

void AnimPage::enter(Page from) {
    LedMatrix::instance().clear();
    _resetRaindrops();
    _resetColorWipe();
    _resetTwinkle();
}

void AnimPage::update() {
    unsigned long now = millis();
    int mode = Store::instance().mode().animMode;

    int interval = (mode == ANIM_MODE_1) ? ANIM_INTERVAL1
                 : (mode == ANIM_MODE_2) ? ANIM_INTERVAL2 : ANIM_INTERVAL3;
    if (now - _lastFrame < interval) return;
    _lastFrame = now;

    if (mode == ANIM_MODE_1)      _drawRaindrops();
    else if (mode == ANIM_MODE_2) _drawColorWipe();
    else                          _drawTwinkle();
}

// ... 各 _draw / _reset 方法从原 drawAnim 拆分 ...

void AnimPage::onBtn1Short() { _cycleAnim(1);  _resetRaindrops(); _resetColorWipe(); _resetTwinkle(); }
void AnimPage::onBtn2Short() { _cycleAnim(-1); _resetRaindrops(); _resetColorWipe(); _resetTwinkle(); }
void AnimPage::onBtn3Short() { StateMachine::instance().gotoPage(Page::CLOCK); }

// ---- 动画1: 雨滴 ----
void AnimPage::_drawRaindrops()  { /* 同原 drawAnim case 1 */ }
void AnimPage::_resetRaindrops() { /* 初始化雨滴状态 */ }

// ---- 动画2: 渐变 ----
void AnimPage::_drawColorWipe()  { /* 同原 drawAnim case 2 */ }
void AnimPage::_resetColorWipe() { _wipeStep = 0; _wipeSubStep = 0; }

// ---- 动画3: 闪烁 ----
void AnimPage::_drawTwinkle()  { /* 同原 drawAnim case 3 */ }
void AnimPage::_resetTwinkle() { _twinkleLitCount = 0; _twinkleIncreasing = true; }
