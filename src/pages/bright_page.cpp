#include "pages/bright_page.h"
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"

void BrightPage::enter(Page from) {
    _drawBright();
}

void BrightPage::onBtn1Short() {
    int b = Store::instance().brightness() + BRIGHTNESS_SPACING;
    if (b > BRIGHTNESS_MAX) return;
    Store::instance().saveBrightness(b);
    LedMatrix::instance().setBrightness(b);
    _drawBright();
}

void BrightPage::onBtn2Short() {
    int b = Store::instance().brightness() - BRIGHTNESS_SPACING;
    if (b < BRIGHTNESS_MIN) return;
    Store::instance().saveBrightness(b);
    LedMatrix::instance().setBrightness(b);
    _drawBright();
}

void BrightPage::onBtn3Short() {
    StateMachine::instance().gotoPage(Page::TIME);
}

void BrightPage::_drawBright() {
    // 显示当前亮度值
}
