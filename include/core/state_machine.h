#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "config.h"
#include "pages/page_base.h"

class StateMachine {
public:
    static StateMachine& instance();

    void init();
    void update();                           // loop() 每帧调用
    void gotoPage(Page p);

    void onButton(int btnIndex, ButtonEvent ev);

    Page  currentPage() const { return _current; }
    bool  isCheckingTime() const { return _isCheckingTime; }
    void  setCheckingTime(bool v) { _isCheckingTime = v; }

private:
    StateMachine() = default;
    Page       _current = Page::TIME;
    PageBase*  _pages[6];
    time_t     _prevDisplay;
    bool       _isCheckingTime = false;
};

#endif
