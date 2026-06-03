#include "hal/buttons.h"
#include "core/state_machine.h"

Buttons& Buttons::instance() {
    static Buttons btns;  // 单例模式(懒汉式)
    return btns;
}

void Buttons::begin() {
    // 引脚内部上拉电阻，低电平触发
    _btn1.setup(BTN1_PIN, INPUT_PULLUP, true);
    _btn2.setup(BTN2_PIN, INPUT_PULLUP, true);
    _btn3.setup(BTN3_PIN, INPUT_PULLUP, true);

    // 绑定 短按 和 长按 的回调函数
    _btn1.attachClick(_onBtn1Click);
    _btn2.attachClick(_onBtn2Click);
    _btn3.attachClick(_onBtn3Click);
    _btn1.attachLongPressStart(_onBtn1Long);
    _btn2.attachLongPressStart(_onBtn2Long);
    _btn3.attachLongPressStart(_onBtn3Long);

    // 设置消抖时间
    _btn1.setDebounceMs(10);  _btn1.setPressMs(1200);
    _btn2.setDebounceMs(10);  _btn2.setPressMs(1200);
    _btn3.setDebounceMs(10);  _btn3.setPressMs(1500);
}

void Buttons::tick() {
    _btn1.tick();
    _btn2.tick();
    _btn3.tick();
}
