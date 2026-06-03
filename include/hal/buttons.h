#ifndef BUTTONS_H
#define BUTTONS_H

#include <OneButton.h>
#include "core/config.h"
#include "core/state_machine.h"

class Buttons {
public:
    static Buttons& instance();  // 单例

    void begin();     // 绑定回调
    void tick();      // 轮询三个按键，loop() 里每帧调用

private:
    Buttons() = default;  // 构建3个 OneButton 对象
    OneButton _btn1;
    OneButton _btn2;
    OneButton _btn3;

    // 回调函数
    static void _onBtn1Click()  { StateMachine::instance().onButton(1, ButtonEvent::SHORT_PRESS); }
    static void _onBtn2Click()  { StateMachine::instance().onButton(2, ButtonEvent::SHORT_PRESS); }
    static void _onBtn3Click()  { StateMachine::instance().onButton(3, ButtonEvent::SHORT_PRESS); }
    static void _onBtn1Long()   { StateMachine::instance().onButton(1, ButtonEvent::LONG_PRESS); }
    static void _onBtn2Long()   { StateMachine::instance().onButton(2, ButtonEvent::LONG_PRESS); }
    static void _onBtn3Long()   { StateMachine::instance().onButton(3, ButtonEvent::LONG_PRESS); }
};

#endif
