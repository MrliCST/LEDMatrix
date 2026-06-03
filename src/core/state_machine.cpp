#include "core/state_machine.h"
#include "pages/setting_page.h"
#include "pages/time_page.h"
#include "pages/rhythm_page.h"
#include "pages/anim_page.h"
#include "pages/clock_page.h"
#include "pages/bright_page.h"

StateMachine& StateMachine::instance() {
    static StateMachine sm;
    return sm;
}

void StateMachine::init() {
    _pages[0] = new SettingPage();
    _pages[1] = new TimePage();
    _pages[2] = new RhythmPage();
    _pages[3] = new AnimPage();
    _pages[4] = new ClockPage();
    _pages[5] = new BrightPage();
}

void StateMachine::update() {
    _pages[(int)_current]->update();
}

void StateMachine::gotoPage(Page p) {
    _pages[(int)_current]->exit();
    Page prev = _current;
    _current = p;
    _pages[(int)_current]->enter(prev);
}

void StateMachine::onButton(int btnIndex, ButtonEvent ev) {
    auto* pg = _pages[(int)_current];
    if (ev == ButtonEvent::SHORT_PRESS) {
        switch (btnIndex) {
            case 1: pg->onBtn1Short(); break;
            case 2: pg->onBtn2Short(); break;
            case 3: pg->onBtn3Short(); break;
        }
    } else {
        switch (btnIndex) {
            case 1: pg->onBtn1Long(); break;
            case 2: pg->onBtn2Long(); break;
            case 3: pg->onBtn3Long(); break;
        }
    }
}
