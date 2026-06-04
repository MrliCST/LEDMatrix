#include "pages/rhythm_page.h"
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"
#include "hal/microphone.h"
#include "img/color_tables.h"    // ← 渐变色表，从原 light.cpp 的 model2ColorArray / model4ColorArrar 提取

void _cycleRhythm(int step) {
    int cur = Store::instance().mode().rhythmMode;
    cur += step;
    if (cur < RHYTHM_MODE_1) cur = RHYTHM_MODE_4;
    if (cur > RHYTHM_MODE_4) cur = RHYTHM_MODE_1;

    ModeConfig cfg = Store::instance().mode();
    cfg.rhythmMode = cur;
    Store::instance().saveMode(cfg);
}

void RhythmPage::update() {
    // Microphone::instance().sample();
    // const int* bands = Microphone::instance().bands();
    //
    // LedMatrix::instance().clear();
    //
    // // 把 bands 映射为柱高度 + 平滑 + 峰值下落（同原 drawRHYTHM 逻辑）
    // // ...
    //
    // switch (Store::instance().mode().rhythmMode) {
    //     case RHYTHM_MODE_1: _drawRainbowBars();   break;
    //     case RHYTHM_MODE_2: _drawGradientBars();  break;
    //     case RHYTHM_MODE_3: _drawVerticalGradient(); break;
    //     case RHYTHM_MODE_4: _drawPeaksOnly();     break;
    // }
    //
    // LedMatrix::instance().show();

    auto& m = LedMatrix::instance();
    m.clear();
    m.setTextColor(m.color(255, 0, 0));
    m.setCursor(0, 0);
    m.print("hello");
    m.show();
}

void RhythmPage::_drawRainbowBars()   { /* 同原 case RHYTHM_MODEL1 */ }
void RhythmPage::_drawGradientBars()  { /* 同原 case RHYTHM_MODEL2 */ }
void RhythmPage::_drawVerticalGradient() { /* 同原 case RHYTHM_MODEL3 */ }
void RhythmPage::_drawPeaksOnly()     { /* 同原 case RHYTHM_MODEL4 */ }

void RhythmPage::onBtn1Short() { _cycleRhythm(1); }
void RhythmPage::onBtn2Short() { _cycleRhythm(-1); }
void RhythmPage::onBtn3Short() { StateMachine::instance().gotoPage(Page::ANIM); }
