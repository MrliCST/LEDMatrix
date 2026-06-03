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

    WifiManager::instance().disconnect();
}

// 在后台线程中执行对时
static void startCalibrateTimeAsync() {
    xTaskCreate(
        [](void*) { startCalibrateTime(); vTaskDelete(nullptr); },
        "ntpSync", 4096, nullptr, 1, nullptr
    );
}

// 连接仿真器的wIFI
static void startWokwiMode() {
    Serial.println("[Wokwi] 连接虚拟 WiFi...");
    WiFi.begin("Wokwi-GUEST", "");  // 连接wifi

    // 等待连接成功
    int ticks = 0;
    while (WiFi.status() != WL_CONNECTED && ticks < 60) {
        delay(500);
        ticks++;
        Serial.print(".");
    }
    Serial.println();

    // 判定是否连接成功
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Wokwi] WiFi 连接失败！");
        return;
    }

    // 连接成功，启动 WebServer
    Serial.print("[Wokwi] 已连接, IP: ");
    Serial.println(WiFi.localIP());

    // 启动 WebServer
    WebServerWrapper::instance().start();

    // 进入设置页面
    StateMachine::instance().gotoPage(Page::SETTING);
}

// 加载并启动
void setup() {
    Serial.begin(115200);

    Store::instance().load();
    LedMatrix::instance().begin();
    Microphone::instance().begin();
    Buzzer::instance().begin();
    Buttons::instance().begin();
    StateMachine::instance().init();

    if (DEBUG) {
        startWokwiMode();
        return;
    }

    // 未配置WiFi，进入配网模式
    if (Store::instance().wifi().apConfig) {
        StateMachine::instance().gotoPage(Page::SETTING);
        WifiManager::instance().startAP();
        WebServerWrapper::instance().start();
        return;
    }

    // 正常启动
    startCalibrateTime();
    Ntp::instance().startTicker();
    StateMachine::instance().gotoPage(Page::TIME);
}

void loop()
{
    // 基础物理层：按键轮询心跳，判断单击、双击、长按事件
    Buttons::instance().tick();

    // 是否为配网模式
    if (Store::instance().wifi().apConfig) {
        WebServerWrapper::instance().handleClient();
    }

    // 校正时间flag是否到达
    if (StateMachine::instance().isCheckingTime()) {
        startCalibrateTimeAsync();
        StateMachine::instance().setCheckingTime(false);
    }

    StateMachine::instance().update();  
    delay(10);
}
