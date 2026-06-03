#include <Arduino.h>
#include "core/store.h"
#include "core/state_machine.h"
#include "hal/led_matrix.h"
#include "hal/microphone.h"
#include "hal/buzzer.h"
#include "hal/buttons.h"
#include "net/wifi_manager.h"
#include "net/ntp.h"
#include "net/web_server.h"

// 向前声明，这样函数可以写在后面
static void startConfigMode();  // 配置WIFI模式
static void startNormalMode();  // 正常启动模式

// 加载
void setup() {
    Serial.begin(115200);

    // 配置必要信息 和 使用的芯片引脚
    Store::instance().load();         // 从闪存Flash中读取配置
    LedMatrix::instance().begin();    // 初始化 LED 矩阵
    Microphone::instance().begin();   // 初始化麦克风
    Buzzer::instance().begin();       // 初始化蜂鸣器
    Buttons::instance().begin();      // 初始化按钮
    StateMachine::instance().init();  // 初始化页面状态机

    // 若wifi未配置，则进入配置，否则跳过
    if (Store::instance().wifi().apConfig) {
        startConfigMode();
        return;
    }

    // 开始正常启动
    startNormalMode();
}

// 主循环
void loop() {
    Buttons::instance().tick();

    if (StateMachine::instance().isCheckingTime()) {
        // 定时对时：连 WiFi → 对时 → 断 WiFi（同原 checkTimeTicker）
        // ...
        StateMachine::instance().setCheckingTime(false);
    }

    StateMachine::instance().update();
    delay(10);
}

// 配置WIFI模式
static void startConfigMode() {
    StateMachine::instance().gotoPage(Page::SETTING);
    WifiManager::instance().startAP();
    WebServerWrapper::instance().start();
}

// 正常启动模式
static void startNormalMode() {
    bool ok = WifiManager::instance().connect(30);
    if (!ok) return;   // WIFI连接失败， 进入节奏灯模式

    ok = Ntp::instance().sync(30);
    if (!ok) return;

    Ntp::instance().startTicker();
    StateMachine::instance().gotoPage(Page::TIME);

    WifiManager::instance().disconnect();
}

