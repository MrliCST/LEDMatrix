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

// ═══════════════════════════════════════════════════════════════
//  调试开关：true = Wokwi 仿真模式  |  false = 真实硬件模式
// ═══════════════════════════════════════════════════════════════
const bool DEBUG = true;

// 联网对时函数
static void startCalibrateTime() {
    bool ok = WifiManager::instance().connect(30);
    if (!ok) {
        StateMachine::instance().gotoPage(Page::RHYTHM);
        return;
    }

    ok = Ntp::instance().sync(30);
    if (!ok) {
        StateMachine::instance().gotoPage(Page::RHYTHM);
        return;
    }

    // 打印当前时间
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    Serial.printf("NTP sync OK, current time: %04d-%02d-%02d %02d:%02d:%02d\n",
                  t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                  t->tm_hour, t->tm_min, t->tm_sec);

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

    // 初始化硬件
    Store::instance().load();
    LedMatrix::instance().begin();
    Microphone::instance().begin();
    Buzzer::instance().begin();
    Buttons::instance().begin();
    StateMachine::instance().init();

    // 正常模式 + 未配置WiFi，进入配网模式
    if (!DEBUG && Store::instance().wifi().apConfig) {
        StateMachine::instance().gotoPage(Page::SETTING);
        WifiManager::instance().startAP();     // 开启热点，以访问esp提供的web页面
        WebServerWrapper::instance().start();  // 开启WebServer, 扫描WiFi,手动指定连接，并保存
        return;
    }

    // 调试模式, 自动配置wifi
    if(DEBUG){
        WifiManager::instance().connectAndSaveWokwiWIFI();  // 自动连接并保存Wokwi虚拟WiFi
    }

    // 具有配网信息，正常启动
    startCalibrateTime();
    Ntp::instance().startTicker();  // 启动定时对时器
    StateMachine::instance().gotoPage(Page::TIME);
}

void loop() {
    // 轮询监听按钮事件
    Buttons::instance().tick();

    // 校正时间flag是否到达
    if (StateMachine::instance().isCheckingTime()) {
        startCalibrateTimeAsync();
        StateMachine::instance().setCheckingTime(false);
    }

    // 对于setting页面，作用是轮询监听http请求
    // 对于其他页面，作用是重绘页面
    StateMachine::instance().update();

    delay(10);
}
