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

// 联网对时函数
static void startCalibrateTime() {
    bool ok = WifiManager::instance().connect(30);
    if (!ok) {
        StateMachine::instance().gotoPage(Page::RHYTHM);  // WiFi 失败 → 节奏灯模式
        return;
    }

    ok = Ntp::instance().sync(30);
    if (!ok) {
        StateMachine::instance().gotoPage(Page::RHYTHM);  // 对时失败 → 节奏灯模式
        return;
    }

    WifiManager::instance().disconnect();
}

// 在后台线程中执行对时
static void startCalibrateTimeAsync() {
    xTaskCreate(
        [](void*) { startCalibrateTime(); vTaskDelete(nullptr); },
        "ntpSync", 4096, nullptr, 1, nullptr
    );
}

// 加载并启动
void setup() {
    Serial.begin(115200);

    // 配置必要信息 和 使用的芯片引脚
    Store::instance().load();         // 从闪存Flash中读取配置
    LedMatrix::instance().begin();    // 初始化 LED 矩阵
    Microphone::instance().begin();   // 初始化麦克风
    Buzzer::instance().begin();       // 初始化蜂鸣器
    Buttons::instance().begin();      // 初始化按钮
    StateMachine::instance().init();  // 初始化页面状态机

    // 未配置WIFI密码信息，进入配置模式, 否则跳过
    if (Store::instance().wifi().apConfig) {
        StateMachine::instance().gotoPage(Page::SETTING);  // 进入网络配置页面
        WifiManager::instance().startAP();     // 启动WIFI热点
        WebServerWrapper::instance().start();  // 启动Web服务器
        return;
    }

    // 校准时间
    startCalibrateTime();

    // 设置定时对时器，并设置页面为 TIME
    Ntp::instance().startTicker();
    StateMachine::instance().gotoPage(Page::TIME);
}

// 主循环
void loop() {
    Buttons::instance().tick();  // 监听按钮事件

    // 校时信号亮起
    if (StateMachine::instance().isCheckingTime()) {
        startCalibrateTimeAsync();   // 后台线程对时
        StateMachine::instance().setCheckingTime(false); 
    }

    StateMachine::instance().update(); 
    delay(10);
}


